#include  <stdafx.h>
#include "copy.h"
#include "exp_core.h"
#include "ext2_fs.h"
#include "copydlg.h"
#include "copycore.h"
#include "overdlg.h"
#include "stat.h"

//////////////////////////////////////////////////////////////////

DWORD copy_several_files(explorer *e, BYTE si, char *srcdir, char *destdir,
                         BYTE *buffer, DWORD buflen,
                         common_file *files, DWORD *indx, DWORD numfiles,
                         volatile DWORD *endflag,
                         LPFNSETDLGDATA lpfnSetCopyDlgData)
{
    DWORD result = RAWCOPY_OK;
    DWORD mode = COPY_ASK;
    DWORD files_processed = 0;

    dirinfo *info = create_rootdirinfo(indx);

    BOOL error = FALSE;
    while (info)
    {
        while (info->cur < info->numdirs && !error && *endflag == 0)
        {
            char reldir[MAXFILENAME];
            build_reldir(reldir, info);

            char cur_srcdir[MAXFILENAME];
            build_lindir(srcdir, reldir, cur_srcdir);

            char cur_destdir[MAXFILENAME];
            build_windir(destdir, reldir, cur_destdir);

            DWORD num_of_efiles;
            common_file *efiles;

            BOOL benum, retry;
            do
            {
                retry = FALSE;
                if (info->indx)
                {
                    efiles = files;
                    num_of_efiles = numfiles;
                    benum = TRUE;
                }
                else
                    benum = start_enum(e->strg[si].fs_handle, cur_srcdir, efiles, num_of_efiles);

                if (!benum)
                {
                    lpfnSetCopyDlgData(NULL, NULL, 0, 0, DLG_STATE_HIDDEN);
                    if (error_message2(e, e->strg[si].common_name) == IDRETRY)
                    {
                        DWORD chkdsk = check_disk(e, si, TRUE);
                        switch (chkdsk)
                        {
                        case DISK_READY:
                            retry = TRUE;
                            break;
                            
                        case DISK_NOT_READY:
                            result = RAWCOPY_FAIL;
                            break;
                            
                        case DISK_NEED_REFRESH:
                            result = RAWCOPY_REFRESH;
                            break;
                        }
                    } 
                    else 
                        result = RAWCOPY_FAIL;

                    lpfnSetCopyDlgData(NULL, NULL, 0, 0, DLG_STATE_VISIBLE);
                }
            }
            while (retry);
            
            if (benum)
            {
                BOOL bdir = create_dir(cur_destdir);
                if (bdir)
                {
                    for (DWORD i = 0; i < num_of_efiles; i++)
                    {
                        common_file *file = info->indx ? efiles + indx[i] : efiles + i;
                        if (file->type == FT_REG)
                        {
                            char win_name[MAXFILENAME];
                            build_winfile(cur_destdir, file->file_name, win_name);

                            BOOL ballow;
                            switch (mode)
                            {
                            case COPY_OVERWRITE_ALL:
                                ballow = TRUE;
                                break;
                            case COPY_SKIP_ALL:
                                ballow = !file_exist(win_name);
                                break;
                            case COPY_ASK:
                                if (file_exist(win_name))
                                {
                                    lpfnSetCopyDlgData(NULL, NULL, 0, 0, DLG_STATE_HIDDEN);
                                    DWORD answer = CreateOverwriteDlg(e, win_name);
                                    if (answer != ANSWER_CANCEL)
                                        lpfnSetCopyDlgData(NULL, NULL, 0, 0, DLG_STATE_VISIBLE);
                                    switch (answer)
                                    {
                                    case ANSWER_CANCEL:
                                        error = 1; // cancel coping, free resources...
                                        break;
                                    case ANSWER_OVERWRITE:
                                        ballow = TRUE;
                                        break;
                                    case ANSWER_OVERWRITE_ALL:
                                        mode = COPY_OVERWRITE_ALL;
                                        break;
                                    case ANSWER_SKIP:
                                        ballow = FALSE;
                                        break;
                                    case ANSWER_SKIP_ALL:
                                        mode = COPY_SKIP_ALL;
                                        break;
                                    }
                                }
                                else
                                    ballow = TRUE;
                                break;
                            }
                            if (ballow)
                            {
                                char lin_name[MAXFILENAME];
                                build_linfile(cur_srcdir, file->file_name, lin_name);

                                result = rawcopy(e, si,
                                    lin_name, file,
                                    win_name,
                                    buffer, buflen,
                                    endflag,
                                    lpfnSetCopyDlgData, 
                                    files_processed + i);

                                if (result != RAWCOPY_OK)
                                    error = TRUE;
                            }

                            if (error || *endflag == 1)
                                break; // for

                        } // if (... == FT_REG)
                    } // for (...)

                    dirinfo *subinfo = create_dirinfo(info, efiles, info->indx, num_of_efiles);

                    if (!info->indx)
                        cancel_enum(efiles);
                    
                    if (subinfo)
                        info = subinfo;
                    else
                        info->cur++;
                }
                else
                {
                    error = TRUE;
					lpfnSetCopyDlgData(NULL, NULL, 0, 0, DLG_STATE_HIDDEN);
                    error_message6(e);
					lpfnSetCopyDlgData(NULL, NULL, 0, 0, DLG_STATE_VISIBLE);
                    assert(0);
                }
                files_processed += num_of_efiles;
            } // if (benum)
            else
                error = TRUE;
        }

        dirinfo *subinfo = info;
        info = info->updir;
        kill_dirinfo(subinfo);
        if (info) info->cur++;
    } // while (info)

    return result;
}

//////////////////////////////////////////////////////////////////

DWORD copy_single_file(explorer *e, BYTE si, 
                       char *src,  
                       common_file *file, 
                       char *dest,
                       BYTE *buf, 
                       DWORD buflen, 
                       volatile DWORD *endflag,
                       LPFNSETDLGDATA lpfnSetCopyDlgData, DWORD fileprocessed)
{
    return rawcopy(e, si, src, file, dest, buf, buflen, endflag,
        lpfnSetCopyDlgData, fileprocessed);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

static DWORD copyfile_thread(void *data)
{
	copyfilepar *p = (copyfilepar *)data;
	return copy_single_file(
        p->e, p->si,
        p->src, p->file, p->dest,
        p->buf, p->buflen,
        p->endflag, p->lpfnSetCopyDlgData, 0);
}

//////////////////////////////////////////////////////////////////

static DWORD copyfiles_thread(void *data)
{
	copyfilespar *p = (copyfilespar *)data;
    return copy_several_files(
        p->e, p->si,
        p->srcdir, p->destdir,
        p->buf, p->buflen, 
        p->files, p->indx, p->numfiles,
        p->endflag,
        p->lpfnSetCopyDlgData);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

DWORD copyfile(explorer *e, BYTE si, char *basedir, common_file *file)
{
	DWORD result = COPY_FAIL;
    HWND hwndFocused = GetFocus();

	char filename[MAXFILENAME];
	lstrcpy(filename, file->file_name);
	if (getfilename(e->hwndMainWindow, filename, MAXFILENAME))
	{
		BYTE *buffer;
		DWORD buflen;
		allocbuffer(buffer, buflen, is_removable(e->strg[si].fs_handle) == DISK_REMOVABLE);

		// create full windows path
		char src[MAXFILENAME];
		lstrcpy(src, basedir);
		if (src[1]) lstrcat(src, "/");
		lstrcat(src, file->file_name);

		volatile DWORD endflag;

		// fill structure for worker thread
		copyfilepar par;
		par.e = e;
		par.si = si;
		par.src = src;
		par.file = file;
		par.dest = filename;
		par.buf = buffer;
		par.buflen = buflen;
		par.endflag = &endflag;
		par.lpfnSetCopyDlgData = SetCopyDlgData;
		ThreadData Data;

		Data.lpData = &par;
		Data.lpfThreadFunc = copyfile_thread;

		// show dialog and run worker thread
		DWORD cres;
		CreateCopyDlg(e, &Data, &endflag, &cres);
		if (cres == RAWCOPY_OK)
            result = COPY_OK;
		else if (cres == RAWCOPY_REFRESH)
            result = COPY_REFRESH;

		freebuffer(buffer);
	}

    SetFocus(hwndFocused);
	return result;
}

//////////////////////////////////////////////////////////////////

DWORD copyfiles(explorer *e, BYTE si, char *basedir, common_file *files,
                DWORD *indx, DWORD numfiles)
{
    DWORD result = COPY_FAIL;
    HWND hwndFocused = GetFocus();

	char destdir[MAXFILENAME];
    if (getdirname(e->hwndMainWindow, destdir, MAXFILENAME)) 
    {
        //
        BYTE *buffer;
        DWORD buflen;
        allocbuffer(buffer, buflen, is_removable(e->strg[si].fs_handle) == DISK_REMOVABLE);
        
        volatile DWORD endflag;
        //// fill structure for worker thread
        copyfilespar par;
        par.e = e;
        par.si = si;             // mounted disk handle
        par.srcdir = basedir;    
        par.destdir = destdir;
        par.buf = buffer;        // disk buffer
        par.buflen = buflen;     // ... its size
        par.endflag = &endflag;
        par.lpfnSetCopyDlgData = SetCopyDlgData;
        par.files = files;
        par.indx = indx;
        par.numfiles = numfiles;
        
        ThreadData data;
        data.lpData = &par;
        data.lpfThreadFunc = copyfiles_thread;
        
        DWORD cres;
        CreateCopyDlg(e, &data, &endflag, &cres);
        
        if (cres == RAWCOPY_OK) 
            result = COPY_OK;
        else if (cres == RAWCOPY_REFRESH) 
            result = COPY_REFRESH;
        
        freebuffer(buffer);
    }

    SetFocus(hwndFocused);
    return result;
}

//////////////////////////////////////////////////////////////////

