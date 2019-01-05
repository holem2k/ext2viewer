#include "stdafx.h"
#include "view.h"
#include "readdlg.h"
#include "showdlg.h"
#include "exp_core.h"

//  internal structures and consts

typedef struct tag_readfilepar
{
	explorer *e;
	BYTE si;
	const char *file_name;		// full file name - /usr/bin/perl
	DWORD size;
	volatile DWORD *end_flag;	
	BYTE *buf;			     	
	SETDLG_FUNC setdlg_func;
} readfilepar;

const DWORD READ_OK      = 0x00;
const DWORD READ_FAIL    = 0x01;
const DWORD READ_REFRESH = 0x02; 

////////////////////////////////////////////////////////////////////////////////

static DWORD readfile(explorer *e, 
					  BYTE si,
					  const char *file_name,
					  DWORD size,
					  volatile DWORD *end_flag,
					  BYTE *buf,
					  SETDLG_FUNC setdlg_func)
{
	DWORD result = READ_OK;

	setdlg_func(READ_DLG_STATE_NORMAL, file_name, 0);

	DWORD offset = 0;
	DWORD chunksize = is_removable(e->strg[si].fs_handle) 
		? SLOW_DEVICE_CHUNK : FAST_DEVICE_CHUNK;

	if (size < chunksize)
		setdlg_func(READ_DLG_STATE_HIDDEN, NULL, 0);

	// read file into memory
	do
    {
		// read next file chunk
		DWORD bytes_read = chunksize > size - offset ? size - offset : chunksize;
		BOOL bread;
		BOOL retry;
		do
		{
			retry = FALSE;
			if (bread = read_file(e->strg[si].fs_handle, (char *)file_name, buf, offset, bytes_read))
				offset += bytes_read;
			else 
			{
				if (*end_flag == 1) 
					break;
				
				setdlg_func(READ_DLG_STATE_HIDDEN, NULL, 0);
				if (error_message2(e, e->strg[si].common_name) == IDRETRY) {
					DWORD chkdsk = check_disk(e, si, TRUE);
					switch (chkdsk)	
					{
					case DISK_READY:
						retry = TRUE;
						break;
						
					case DISK_NOT_READY:
						result = READ_FAIL;
						break;
						
					case DISK_NEED_REFRESH:
						result = READ_REFRESH;
						break;
					}
				} 
				else 
					result = READ_FAIL;
				
				if (size >= chunksize)
					setdlg_func(READ_DLG_STATE_VISIBLE, NULL, 0);
			}
		} while (retry);
		
		if (!bread || *end_flag == 1) 
			break;
		
		setdlg_func(READ_DLG_STATE_NORMAL, file_name, (DWORD)(100.0*offset/size));
	} while (offset < size);

	if (*end_flag)
		result = READ_FAIL;

	return result;
}

////////////////////////////////////////////////////////////////////////////////

static DWORD readfile_thread(void *data)
{
	readfilepar *p = (readfilepar *)data;
	return readfile(p->e,
		p->si,
		p->file_name,
		p->size,
		p->end_flag,
		p->buf, 
		p->setdlg_func);
}

////////////////////////////////////////////////////////////////////////////////

DWORD viewfile(explorer *e, BYTE si, 
			   const char *folder_name, common_file *file)
{
	assert(file->type == FT_REG);

	DWORD buflen;
	buflen = file->i_size ? file->i_size : 16;
	BYTE *buffer = new BYTE[buflen];
	if (!buffer)
	{
		MessageBox(e->hwndMainWindow, Message(MSG_NOMEMORYFORVIEW), "",  MB_OK);
		return VIEW_FAIL;
	}

	// make full file name
	char file_name[MAXFILENAME];
	lstrcpy(file_name, folder_name);
	if (lstrlen(folder_name) > 1)
		lstrcat(file_name, "/");
	lstrcat(file_name, file->file_name);

	volatile DWORD end_flag;

	// set params for readfile()
	readfilepar p;
	p.e = e;
	p.si = si;
	p.end_flag = &end_flag;
	p.file_name = file_name;
	p.buf = buffer; 
	p.setdlg_func = set_readdlg_data;
	p.size = file->i_size;

	// show dialog, run thread
	thread_data data;
	data.data = &p;
	data.thread_func = readfile_thread;
	DWORD readres;

	HWND hwndFocused = GetFocus();
	create_readdlg(e->hwndMainWindow, &data, &end_flag, &readres);
	SetFocus(hwndFocused);

	if (readres == READ_OK)
		create_view(e, buffer, file->i_size, file->file_name);

	delete [] buffer;

	DWORD result = VIEW_OK;
	switch (readres)
	{
	case READ_FAIL:
		result = VIEW_FAIL;
		break;

	case READ_REFRESH:
		result = VIEW_REFRESH;
		break;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
