#ifndef __READDLG_H__
#define __READDLG_H__

typedef struct tag_thread_data
{
	DWORD (*thread_func)(void *data); // read function 
	void *data;   // pointer to data for read function
} thread_data;

const DWORD READ_DLG_STATE_NORMAL  = 0x01;
const DWORD READ_DLG_STATE_HIDDEN  = 0x02;
const DWORD READ_DLG_STATE_VISIBLE = 0x04;

typedef void (*SETDLG_FUNC)(DWORD state, const char *filename, 
					  DWORD percent);

void set_readdlg_data(DWORD state, const char *filename, 
					  DWORD percent);
void create_readdlg(HWND hParent, thread_data *data,
					volatile DWORD *end_flag,
					DWORD *thread_result/*OUT*/);

#endif //__READDLG_H__