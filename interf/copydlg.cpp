#include <stdafx.h>
#include "copydlg.h"
#include "math.h"


const DWORD IDT_TIMER = 0xAFAD2411;

////////////////////////////////////////////////////////////////////////////////

static volatile DWORD *g_lpThreadEndFlag;
static LPTHREAD_START_ROUTINE g_lpWorkerThread;
static ThreadData *g_lpThreadData;
static HANDLE g_hWorkerThread;
static HWND g_hwndDlg;
static DWORD *g_dwResult;
static HDC g_dc;

////////////////////////////////////////////////////////////////////////////////

char *shortlinname(char *name, int width)
{
    static char sn[MAXFILENAME];

    int i; SIZE s;
    GetTextExtentExPoint(g_dc, name, lstrlen(name), width - 10, &i, NULL, &s);

    lstrcpyn(sn, name, i + 1);
    if (i < lstrlen(name))
        lstrcat(sn, "...");
    return sn;
}

////////////////////////////////////////////////////////////////////////////////

char *shortwinname(char *name, int width)
{
    static char sn[MAXFILENAME];

    int i; SIZE s;
    GetTextExtentExPoint(g_dc, name, lstrlen(name), width - 10, &i, NULL, &s);
    
    lstrcpyn(sn, name, i + 1);
    if (i < lstrlen(name))
        lstrcat(sn, "...");
    return sn;
}

////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	assert(lpParameter);
	ThreadData *lpThreadInfo = (ThreadData *)lpParameter;
	return lpThreadInfo->lpfThreadFunc(lpThreadInfo->lpData);
}

////////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK CopyDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
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
		{
			switch (wParam)
			{
			case IDT_TIMER:
				if (WaitForSingleObject(g_hWorkerThread, 1) == WAIT_OBJECT_0)
				{
					// Worker thread has done copying or was cancelled
					KillTimer(hwndDlg, IDT_TIMER);
					GetExitCodeThread(g_hWorkerThread, g_dwResult);
					CloseHandle(g_hWorkerThread);
					EndDialog(hwndDlg, IDOK);
					bResult = FALSE; /* WM_TIMER processed */
				}
				break;
			}
		}
		break;
	}
	return bResult;
}		

////////////////////////////////////////////////////////////////////////////////

void SetCopyDlgData(char *lpLinuxName, char *lpWindowsName, DWORD dwNumOfFiles,
					DWORD dwPercent, DWORD dwState)
{
    
	switch (dwState)
	{
	case DLG_STATE_NORMAL:
        {
            RECT rc;
            HWND hwndLabel = GetDlgItem(g_hwndDlg, IDC_STATIC_LINUXNAME);
            GetWindowRect(hwndLabel, &rc);
            g_dc = GetDC(g_hwndDlg);
            HFONT hSysFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            HFONT hOldFont = (HFONT)SelectObject(g_dc, hSysFont);
            if (lpLinuxName) 
                SetDlgItemText(g_hwndDlg, IDC_STATIC_LINUXNAME, shortlinname(lpLinuxName, rc.right - rc.left));
            if (lpWindowsName) 
                SetDlgItemText(g_hwndDlg, IDC_STATIC_WINDOWSNAME, shortwinname(lpWindowsName, rc.right - rc.left));
            SetDlgItemInt(g_hwndDlg, IDC_STATIC_FILENUM, dwNumOfFiles, FALSE);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_PROGRESS1), PBM_SETPOS, dwPercent, 0);

            SelectObject(g_dc, hOldFont);
            ReleaseDC(hwndLabel, g_dc);
        }
		break;

	case DLG_STATE_HIDDEN:
		ShowWindow(g_hwndDlg, SW_HIDE);
		break;

	case DLG_STATE_VISIBLE:
		ShowWindow(g_hwndDlg, SW_SHOW);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void CreateCopyDlg(explorer *e, ThreadData *lpThreadData,
				   volatile DWORD *lpThreadEndFlag, DWORD *dwResult)
{
	g_lpThreadData = lpThreadData;
	g_lpThreadEndFlag = lpThreadEndFlag;
	g_dwResult = dwResult;

	DialogBox((HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_COPYFILE),
		e->hwndMainWindow,
		CopyDialogProc);
}


////////////////////////////////////////////////////////////////////////////////
