#include "stdafx.h"
#include "file_propdlg.h"
#include "stat.h"
#include "text.h"

static common_file *g_file;
static HICON g_hIcon;
static char *g_exinfo;
static char *g_exinfotext;

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
			// set icon 
			DWORD dwIcon;
			switch (g_file->type)
			{
			case FT_DIR: dwIcon = IDI_FOLDER;
				break;
			case FT_REG: dwIcon = IDI_REGFILE;
				break;
			case FT_LNK: dwIcon = IDI_LNKFILE;
				break;
			case FT_FIFO: dwIcon = IDI_FIFOFILE;
				break;
			case FT_SOCK: dwIcon = IDI_SOCKFILE;
				break;
			case FT_CHR: dwIcon = IDI_CHARFILE;
				break;
			case FT_BLK: dwIcon = IDI_BLOCKFILE;
				break;
			}
			g_hIcon = LoadIcon((HINSTANCE)GetWindowLong(hwndDlg, GWL_HINSTANCE),
				MAKEINTRESOURCE(dwIcon));
			SendMessage(GetDlgItem(hwndDlg, IDC_STATIC_ICON),
				STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIcon);
			// set properties
			char s[512];
			SetDlgItemText(hwndDlg, IDC_STATIC_FILE_NAME, g_file->file_name);
			SetDlgItemText(hwndDlg, IDC_STATIC_TYPE, get_type_from_file(g_file));
			SetDlgItemText(hwndDlg, IDC_STATIC_ATTR, get_attr_from_file(g_file));
			
			wsprintf(s, "%lu", g_file->i_links_count);
			SetDlgItemText(hwndDlg, IDC_STATIC_I_LINKS_COUNT, s);
			
			//wsprintf(s, "%lu", g_file->i_size);
			SetDlgItemText(hwndDlg, IDC_STATIC_I_SIZE, size_to_spaced_string(g_file->i_size));
			SetDlgItemText(hwndDlg, IDC_STATIC_CTIME, time_to_string(g_file->i_ctime));
			SetDlgItemText(hwndDlg, IDC_STATIC_MTIME, time_to_string(g_file->i_mtime));
			SetDlgItemText(hwndDlg, IDC_STATIC_ATIME, time_to_string(g_file->i_atime));
			
			SetDlgItemText(hwndDlg, IDC_STATIC_SUID, get_suid_from_file(g_file));
			SetDlgItemText(hwndDlg, IDC_STATIC_SGID, get_sgid_from_file(g_file));

			wsprintf(s, "%s: %s", Message(MSG_PROPERTIES), g_file->file_name);
			SetWindowText(hwndDlg, s);
			BOOL bExtended = FALSE;
			if (g_exinfo && g_exinfotext)
			{
				SetDlgItemText(hwndDlg, IDC_STATIC_EXTENDED, g_exinfo);
				SetDlgItemText(hwndDlg, IDC_STATIC_EXTENDED_TEXT, g_exinfotext);
				bExtended = TRUE;
			}
			if (!bExtended)
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_EXTENDED), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_EXTENDED_TEXT), SW_HIDE);
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

void create_file_propdlg(explorer *e, common_file *file, char *exinfo, char *exinfotext)
{
	g_file = file;
	g_exinfo = exinfo;
	g_exinfotext = exinfotext;

	DialogBox((HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_FILE_PROPS),
		e->hwndMainWindow,
		DialogProc);
}

///////////////////////////////////////////////////////////////////////////
