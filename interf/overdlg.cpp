#include <stdafx.h>
#include "overdlg.h"

////////////////////////////////////////////////////////////////////////////////

static char *g_filename;

static BOOL CALLBACK OverwriteDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_STATIC_OVERWRITE_TEXT, g_filename);
		bResult = TRUE;
		break;
		
	case WM_COMMAND:
		{
            DWORD dwAnswer;
			switch (wParam)
			{
			case ID_CANCEL:
                dwAnswer = ANSWER_CANCEL;
                break;

			case ID_OVERWRITE:
                dwAnswer = ANSWER_OVERWRITE;
                break;

			case ID_OVERWRITE_ALL:
                dwAnswer = ANSWER_OVERWRITE_ALL;
                break;

			case ID_SKIP:
                dwAnswer = ANSWER_SKIP;
                break;

			case ID_SKIP_ALL:
                dwAnswer = ANSWER_SKIP_ALL;
                break;
			}

            EndDialog(hwndDlg, dwAnswer);
		}
		break;
	}
	return bResult;
}		

////////////////////////////////////////////////////////////////////////////////

int CreateOverwriteDlg(explorer *e, char *filename)
{
    g_filename = filename;
	return DialogBox((HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_OVERWRITE),
		e->hwndMainWindow,
		OverwriteDialogProc);
}

////////////////////////////////////////////////////////////////////////////////
