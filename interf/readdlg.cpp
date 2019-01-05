#include "stdafx.h"
#include "readdlg.h"

const DWORD IDT_TIMER = 0x27FA9EAC;

////////////////////////////////////////////////////////////////////////////////

static HWND g_hwndDlg;
static volatile DWORD *g_lpThreadEndFlag;
static HANDLE g_hWorkerThread;
static void  *g_lpThreadData;
static DWORD *g_dwThreadResult;

////////////////////////////////////////////////////////////////////////////////

static DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	assert(lpParameter);

	thread_data *data = (thread_data *)lpParameter;
	return data->thread_func(data->data);
}

////////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK ReadDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		g_hwndDlg = hwndDlg;
		*g_lpThreadEndFlag = 0;
		DWORD dwThreadId;
		g_hWorkerThread = CreateThread(NULL, 0, ThreadProc,
			g_lpThreadData, 0, &dwThreadId);
		assert(g_hWorkerThread);

		SetTimer(hwndDlg, IDT_TIMER, 50, NULL);
		bResult = TRUE;
		break;

	case WM_COMMAND:
		{
			switch (wParam)
			{
			case IDCANCEL:
				*g_lpThreadEndFlag = 1;
				break;
			}
		}
		break;

	case WM_TIMER:
		switch (wParam)
		{
		case IDT_TIMER:
			if (WaitForSingleObject(g_hWorkerThread, 1) == WAIT_OBJECT_0)
			{
				// Worker thread has done reading or was cancelled
				KillTimer(hwndDlg, IDT_TIMER);
				GetExitCodeThread(g_hWorkerThread, g_dwThreadResult);
				CloseHandle(g_hWorkerThread);
				EndDialog(hwndDlg, 1);
				bResult = FALSE; /* WM_TIMER processed */
			}
			break;
		}
		break;
	}

	return bResult;
} 

////////////////////////////////////////////////////////////////////////////////

void set_readdlg_data(DWORD state, const char *filename, DWORD percent)
{
	switch (state)
	{
	case READ_DLG_STATE_NORMAL:
		// set file name
		SetDlgItemText(g_hwndDlg, IDC_STATIC_FILE, filename);
		// progress bar
		SendMessage(GetDlgItem(g_hwndDlg, IDC_PROGRESS1), PBM_SETPOS, percent, 0);
		break;

	case READ_DLG_STATE_HIDDEN:
		ShowWindow(g_hwndDlg, SW_HIDE);
		break;

	case READ_DLG_STATE_VISIBLE:
		ShowWindow(g_hwndDlg, SW_SHOW);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void create_readdlg(HWND hParent, thread_data *data,
					volatile DWORD *end_flag,
					DWORD *thread_result/*OUT*/)
{
	// set global vars
	g_lpThreadEndFlag = end_flag;
	g_lpThreadData = data;
	g_dwThreadResult = thread_result;

	// create modal dialog, run work thread 
	DialogBox((HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_READFILE),
		hParent,
		ReadDialogProc);
}

////////////////////////////////////////////////////////////////////////////////
