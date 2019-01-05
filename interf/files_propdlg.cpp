#include "stdafx.h"
#include "files_propdlg.h"
#include "stat.h"

///////////////////////////////////////////////////////////////////////////

static common_file *g_files;
static DWORD *g_files_indx;
static DWORD g_num_of_files;

///////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND hwndStatic = GetDlgItem(hwndDlg, IDC_STATIC_SPLITTER);
			LONG Style = GetWindowLong(hwndStatic, GWL_STYLE);
			SetWindowLong(hwndStatic, GWL_STYLE, Style | SS_ETCHEDHORZ);
			RECT rc;
			GetClientRect(hwndDlg, &rc);
			SetWindowPos(hwndStatic, 0, 5, 50, rc.right - rc.left - 10, 2,  SWP_NOZORDER | SWP_FRAMECHANGED);

			if (g_num_of_files > 0)
			{
				char s[512];
				wsprintf(s, "%s: %s, ...", Message(MSG_PROPERTIES), g_files[g_files_indx[0]].file_name);
				SetWindowText(hwndDlg, s);
				DWORD num_of_files = 0;
				DWORD num_of_dirs = 0;
				DWORD sum_files_size = 0;
				DWORD sum_dirs_size = 0;

				for (DWORD i = 0; i < g_num_of_files; i++)
				{
					common_file *file = g_files + g_files_indx[i];
					if (file->type == FT_DIR)
					{
						num_of_dirs++;
						sum_dirs_size += file->i_size;
					}
					else
					{
						num_of_files++;
						sum_files_size += file->i_size;
					}
				}
				if (num_of_files > 0 && num_of_dirs > 0)
				{
					// files and dirs...
					wsprintf(s, Message(MSG_DIRSANDFILES), num_of_dirs, num_of_files);
					SetDlgItemText(hwndDlg, IDC_STATIC_FILES, s);
					SetDlgItemText(hwndDlg, IDC_STATIC_MSG_TITLE1, Message(MSG_SUMFILESIZE));
					SetDlgItemText(hwndDlg, IDC_STATIC_MSG_TITLE1_TEXT, size_to_string(sum_files_size));
				}
				else if (num_of_files == 0)
				{
					// only dirs
					wsprintf(s, Message(MSG_DIRS), num_of_dirs);
					SetDlgItemText(hwndDlg, IDC_STATIC_FILES, s);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_MSG_TITLE1), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_MSG_TITLE1_TEXT), SW_HIDE);
				}
				else
				{
					// only files
					wsprintf(s, Message(MSG_FILES), num_of_files);
					SetDlgItemText(hwndDlg, IDC_STATIC_FILES, s);
					SetDlgItemText(hwndDlg, IDC_STATIC_MSG_TITLE1, Message(MSG_SUMFILESIZE));
					SetDlgItemText(hwndDlg, IDC_STATIC_MSG_TITLE1_TEXT, size_to_string(sum_files_size));
				}
			}
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

void create_files_propdlg(explorer *e, common_file *files, DWORD *files_indx, DWORD num_of_files)
{
	g_files = files;
	g_files_indx = files_indx;
	g_num_of_files = num_of_files;

	DialogBox((HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_FILES_PROPS),
		e->hwndMainWindow,
		DialogProc);
}

///////////////////////////////////////////////////////////////////////////
