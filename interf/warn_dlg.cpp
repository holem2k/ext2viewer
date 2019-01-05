#include "stdafx.h"
#include "warn_dlg.h"

///////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hwndDlg, Message(MSG_WARNING));
			SetDlgItemText(hwndDlg, IDC_STATIC_WARNIG_TEXT, Message(MSG_DIRECTACCESSDENIED));
			bResult = TRUE;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			bResult = TRUE;
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			bResult = TRUE;
			break;
		}
		break;
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////

void show_warning(explorer *e)
{
	DialogBox((HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_WARNING),
		e->hwndMainWindow,
		DialogProc);
}

///////////////////////////////////////////////////////////////////////////
