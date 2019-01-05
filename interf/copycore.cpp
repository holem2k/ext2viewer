#include "stdafx.h"
#include <commdlg.h>
#include <objbase.h>
#include <shlobj.h>
#include "copycore.h"
#include "copydlg.h"
#include "exp_core.h"
#include "stat.h"
#include "registry.h"

//////////////////////////////////////////////////////////////////

BOOL getfilename(HWND hwndOwner, char *buf, DWORD buflen)
{
    LPITEMIDLIST pidl = NULL;
    SHGetSpecialFolderLocation(hwndOwner, CSIDL_DESKTOP, &pidl);
    char lpInitialDir[MAXFILENAME];
    if (pidl)
        SHGetPathFromIDList(pidl, lpInitialDir);
    
    IMalloc *pMalloc;
    if (SHGetMalloc(&pMalloc) == NOERROR)
    {
        pMalloc->Free(pidl);
        pMalloc->Release();
    }
    
    OPENFILENAME ofn;
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = buflen;
    ofn.lpstrFilter = "Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = lpInitialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;// OFN_EXPLORER;
    
    return GetSaveFileName(&ofn);
}

//////////////////////////////////////////////////////////////////

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,
                                LPARAM lParam, LPARAM lpData)
{
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
        {
            char buffer[MAXFILENAME];
            if (reg_getstr(PVAL_PATH, buffer, MAXFILENAME))
                SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)buffer);
        }
        break;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////

BOOL getdirname(HWND hwndOwner, char *buf, DWORD buflen)
{
    CoInitialize(NULL);
    LPITEMIDLIST pidl = NULL;
    SHGetSpecialFolderLocation(hwndOwner, CSIDL_DESKTOP, &pidl);
    
    BROWSEINFO brinfo;
    brinfo.hwndOwner = hwndOwner;
    brinfo.pidlRoot = pidl;
    brinfo.lpszTitle = Message(MSG_BROWSETITLE);
    brinfo.lParam = 0;
    brinfo.lpfn = BrowseCallbackProc;
    brinfo.ulFlags = BIF_RETURNONLYFSDIRS;
    brinfo.pszDisplayName = NULL;
    
    
    LPITEMIDLIST pidl2 = SHBrowseForFolder(&brinfo);
    
    BOOL bResult = FALSE;
    if (pidl2)
    {
        SHGetPathFromIDList(pidl2, buf);
        reg_setstr(PVAL_PATH, buf);
        bResult = TRUE;
    }
    
    IMalloc *pMalloc = NULL;
    if (SHGetMalloc(&pMalloc) == NOERROR)
    {
        pMalloc->Free(pidl);
        pMalloc->Free(pidl2);
        pMalloc->Release();
    }
    
    CoUninitialize();
    return bResult;
}

//////////////////////////////////////////////////////////////////

void allocbuffer(BYTE *&buffer, DWORD &buflen, BOOL removable)
{
    if (removable)
        buflen = 1024*64;
    else
        buflen = 1024*1024*2;
    
    buffer = new BYTE[buflen];
    assert(buffer);
}

//////////////////////////////////////////////////////////////////

void freebuffer(BYTE *buffer)
{
    delete [] buffer;
}

//////////////////////////////////////////////////////////////////

dirinfo *create_rootdirinfo(DWORD *indx)
{
    dirinfo *info = new dirinfo;
    info->cur = 0;
    info->updir = NULL;
    info->indx = indx;
    info->data = new dirname [1]; // delete []
    info->numdirs = 1;
    lstrcpy(info->data[0].data, "");
    
    return info;
}

//////////////////////////////////////////////////////////////////

dirinfo *create_dirinfo(dirinfo *updir, common_file *files, DWORD *indx, DWORD numfiles)
{
    DWORD numdirs = 0, i;
    for (i = 0; i < numfiles; i++) 
    {
        common_file *file = indx ? files + indx[i] : files + i;
        if (file->type == FT_DIR)
            numdirs++;
    }
    
    dirinfo *info = NULL;
    if (numdirs > 0) 
    {
        info = new dirinfo;
        info->updir = updir;
        info->cur = info->numdirs = 0;
        info->numdirs = numdirs;
        info->data = new dirname[info->numdirs];
        info->indx = NULL;
        DWORD j = 0;
        for (i = 0; i < numfiles; i++)
        {
            common_file *file = indx ? files + indx[i] : files + i;
            if (file->type == FT_DIR)
                lstrcpy(info->data[j++].data, file->file_name);
        }
    }
    
    return info;
}

//////////////////////////////////////////////////////////////////

void kill_dirinfo(dirinfo *info)
{
    delete [] info->data;
    delete info;
}

//////////////////////////////////////////////////////////////////
//  размер буфера - FOLDER_NAME_LEN

void build_reldir(char *buf, dirinfo *info)
{
    char tbuf[MAXFILENAME];
    lstrcpy(buf, "");
    
    do
    {
        lstrcpy(tbuf, info->data[info->cur].data);
        if (info->updir && lstrlen(buf))
            lstrcat(tbuf, "/");
        lstrcat(tbuf, buf);
        lstrcpy(buf, tbuf);
        info = info->updir;
    } while (info);
}

//////////////////////////////////////////////////////////////////
//  размер буфера - FOLDER_NAME_LEN

void build_windir(const char *dir, const char *reldir, char *buf)
{
    lstrcpy(buf, dir);
    
    int len = lstrlen(dir); 
    if (dir[len - 1] != '\\')
        lstrcat(buf, "\\");
    lstrcat(buf, reldir);
    
    for (int i = len - 1; i < lstrlen(buf); i++) 
    {
        if (buf[i] == '/')
            buf[i] = '\\';
    }
}


//////////////////////////////////////////////////////////////////
//  размер буфера - FOLDER_NAME_LEN

void build_lindir(const char *dir, const char *reldir, char *buf)
{
    lstrcpy(buf, dir);
    if (lstrcmp(dir, "/") && lstrlen(reldir))
        lstrcat(buf, "/");
    
    lstrcat(buf, reldir);
}

//////////////////////////////////////////////////////////////////
// '/usr/bin' + 'perl' = '/usr/bin/perl'
// '/' + 'perl' = 'perl'

void build_linfile(const char *dir, const char *file, char *buf)
{
    lstrcpy(buf, dir);
    
    if (lstrcmp(dir, "/")) 
        lstrcat(buf, "/");
    
    lstrcat(buf, file);
}

//////////////////////////////////////////////////////////////////
// 'c:\windows' + 'win.com' = 'c:\windows\'
// 'c:\' + 'config.sys' = 'c:\windows\config.sys'

void build_winfile(const char *dir, const char *file, char *buf)
{
    lstrcpy(buf, dir);
    
    int len = lstrlen(dir);
    if (dir[len - 1] != '\\')
        lstrcat(buf, "\\");
    
    lstrcat(buf, file);
    // change banned symbols
    for (int i = 2; i < lstrlen(buf); i++)
    {
        switch(buf[i])
        {
        case '?':
        case '*':
        case '<':
        case '>':
        case ':':
            buf[i] = '-';
            break;
        case '\"':
            buf[i] = '\'';
            break;
        }
    }
    
}

//////////////////////////////////////////////////////////////////

BOOL create_dir(char *directory)
{
    USE_LONGNAMES;

	// NT doesn't allow to create directories like 'c:\' or 'd:\', 
	// but MD does.
	if (lstrlen(directory) == 3 && !lstrcmp(directory + 1, ":\\"))
		return TRUE;

    BOOL result;
    CALL_LONGNAME_PROC(CreateDirectory, (directory, NULL), directory, MAXFILENAME, result);
    
    if (!result)
        result = (GetLastError() == ERROR_ALREADY_EXISTS);
    
    return result;
}

//////////////////////////////////////////////////////////////////

BOOL file_exist(char *file_name)
{
    USE_LONGNAMES;

    DWORD result;
    CALL_LONGNAME_PROC(GetFileAttributes, (file_name), file_name, MAXFILENAME, result);

    BOOL bexist = TRUE;
    if (result == 0xFFFFFFFF && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        bexist = FALSE;
    }

    return bexist;
}

//////////////////////////////////////////////////////////////////


DWORD rawcopy(explorer *e, BYTE si, 
			  char *src,  /* файла (ext2)    */
			  common_file *file, /* файла    */
			  char *dest, /* файл (FAT/NTFS) */
			  BYTE *buf,  /* буфер           */
			  DWORD buflen, /* длина буфера  */
			  volatile DWORD *endflag,
			  LPFNSETDLGDATA setdlg_func, DWORD fileprocesed)
{
    USE_LONGNAMES;

    assert(buflen%512 == 0);
    
    setdlg_func(src, dest, fileprocesed, 0, DLG_STATE_NORMAL);
    
    BOOL retry;
    // create dest. file
    HANDLE fh;
    do
    {
        retry = FALSE;
        CALL_LONGNAME_PROC(CreateFile,
            (
            dest, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
            ),
            dest, MAXFILENAME, fh);
        
        if (fh == INVALID_HANDLE_VALUE)
        {
            setdlg_func(src, dest, fileprocesed, 0, DLG_STATE_HIDDEN);
            char disk[8];
            lstrcpyn(disk, dest, 4);
            
            DWORD error = GetLastError();
            if (error == ERROR_ACCESS_DENIED)
            {
                if (error_message4(e, disk) == IDRETRY)
                    retry = TRUE;
            }
            else 
            {
                if (error_message2(e, disk) == IDRETRY)
                    retry = TRUE;
            }
            setdlg_func(src, dest, fileprocesed, 0, DLG_STATE_VISIBLE);
        }
    } while (retry);
    
    if (fh == INVALID_HANDLE_VALUE) 
        return RAWCOPY_FAIL;
    
    DWORD result = RAWCOPY_OK;

	// start copying
    DWORD size = file->i_size;
	DWORD offset = 0;
	do
    {
		// read next file chunk
        DWORD chunksize = buflen > size - offset ? size - offset : buflen;
		BOOL bread;
		do
        {
			retry = FALSE;
			if (bread = read_file(e->strg[si].fs_handle, src, buf, offset, chunksize))
                offset += chunksize;
			else 
            {
				if (*endflag == 1)
					break;

				setdlg_func(NULL, NULL, 0, 0, DLG_STATE_HIDDEN);
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

				setdlg_func(NULL, NULL, 0, 0, DLG_STATE_VISIBLE);
			}
		} while (retry);
		if (!bread || *endflag == 1) 
            break;

        setdlg_func(NULL, NULL, fileprocesed, (DWORD)(100.0*(offset - chunksize/2)/size), DLG_STATE_NORMAL);

		// write chunk to disk
		BOOL bwrite;
		do
        {
			retry = FALSE;
			DWORD bytes_written;
			if((bwrite = WriteFile(fh, buf, chunksize, &bytes_written, NULL)) == FALSE) 
			{
				if (*endflag == 1) 
					break;

				setdlg_func(src, dest, fileprocesed, 0, DLG_STATE_HIDDEN);
				
				char disk[8];
				lstrcpyn(disk, dest, 4);
				DWORD error = GetLastError();
				if (error == ERROR_DISK_FULL) 
                {
					if (error_message5(e, disk) == IDRETRY)
                        retry = TRUE;
					else 
                        result = RAWCOPY_FAIL;
				}
				else
                {
					if (error_message2(e, disk) == IDRETRY)
                        retry = TRUE;
					else 
                        result = RAWCOPY_FAIL;
				}

				setdlg_func(src, dest, fileprocesed, 0, DLG_STATE_VISIBLE);
			}
		} while (retry);

		if (!bwrite || *endflag == 1)
            break;

		setdlg_func(NULL, NULL, fileprocesed, (DWORD)(100.0*offset/size), DLG_STATE_NORMAL);

	} while (offset < size);

	setdlg_func(src, dest, fileprocesed + 1, 100, DLG_STATE_NORMAL);

    FILETIME ctime;
    time_to_filetime(file->i_ctime, &ctime);

    FILETIME atime;
    time_to_filetime(file->i_atime, &atime);

    FILETIME mtime;
    time_to_filetime(file->i_mtime, &mtime);

    SetFileTime(fh, &ctime, &atime, &mtime);
	CloseHandle(fh);

	if (*endflag)
		result = RAWCOPY_FAIL;

	if (result != RAWCOPY_OK)
        DeleteFile(dest);

	return result;
}

//////////////////////////////////////////////////////////////////
