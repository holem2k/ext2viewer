#include <stdafx.h>
#include "options.h"

static options *g_options;

///////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DialogProc(HWND hwndDlg,  UINT uMsg,
							WPARAM wParam,  LPARAM lParam)
{
	BOOL bResult = FALSE;
	int i;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		for (i = 0; i < COL_MAX; i++)
		{
			int iControl = 0;
			switch(g_options->fb_columns[i])
			{
			case COL_NAME:
				iControl = IDC_CHECK_NAME;
				break;
			case COL_SIZE:
				iControl = IDC_CHECK_SIZE;
				break;
			case COL_TYPE:
				iControl = IDC_CHECK_TYPE;
				break;
			case COL_RIGHTS:
				iControl = IDC_CHECK_RIGHTS;
				break;
			case COL_CTIME:
				iControl = IDC_CHECK_CTIME;
				break;
			case COL_MTIME:
				iControl = IDC_CHECK_MTIME;
				break;
			case COL_ATIME:
				iControl = IDC_CHECK_ATIME;
				break;
			case COL_USER:
				iControl = IDC_CHECK_USER;
				break;
			case COL_GROUP:
				iControl = IDC_CHECK_GROUP;
				break;
			}
			if (iControl)
			{
				SendMessage(GetDlgItem(hwndDlg, iControl), BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			}
		}
		bResult = TRUE;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				int ci = 0;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_NAME),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_NAME;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_TYPE),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_TYPE;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_SIZE),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_SIZE;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_RIGHTS),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_RIGHTS;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_CTIME),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_CTIME;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_MTIME),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_MTIME;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_ATIME),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_ATIME;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_USER),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_USER;
				if (SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_GROUP),
					BM_GETCHECK, 0, 0) == BST_CHECKED)	
					g_options->fb_columns[ci++] = COL_GROUP;
				for (int i = ci; i < COL_MAX; i++) g_options->fb_columns[i] = -1;
				bResult = TRUE;
				EndDialog(hwndDlg, IDOK);
			}
			break;
		case IDCANCEL:
			bResult = TRUE;
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////

DWORD CreateOptionsDialog(HWND hwndParent, options *opt)
{
	g_options = opt;
	return DialogBox((HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE), 
		MAKEINTRESOURCE(IDD_DIALOG2),
		hwndParent,
		DialogProc);
}

///////////////////////////////////////////////////////////////////////////
