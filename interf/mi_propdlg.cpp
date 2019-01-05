#include "stdafx.h"
#include "mi_propdlg.h"
#include "text.h"
#include "time.h"
#include "stat.h"


///////////////////////////////////////////////////////////////////////////

static HWND g_hTabCtrl;
static mount_info *g_mi;
static common_file *g_file;
static const char *g_disk_name;

static BOOL CALLBACK DialogProcPage(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		ShowWindow(hwndDlg, SW_HIDE);
		bResult = TRUE;
		break;
	}
	return bResult;
}		

///////////////////////////////////////////////////////////////////////////

static HWND create_page(HWND hwndTabCtrl, int iPageID)
{
	return CreateDialog((HINSTANCE)GetWindowLong(hwndTabCtrl, GWL_HINSTANCE),
		MAKEINTRESOURCE(iPageID),
		hwndTabCtrl,
		DialogProcPage);
}

///////////////////////////////////////////////////////////////////////////

void set_mi_page(HWND hwndPage1, mount_info *mi)
{
	char s[256];
	wsprintf(s, "%lu", g_mi->inodes_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_INODES_COUNT, s);
	wsprintf(s, "%lu", g_mi->inodes_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_INODES_COUNT, s); 
	wsprintf(s, "%lu", g_mi->free_inodes_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_FREE_INODES_COUNT, s);   
	wsprintf(s, "%lu", g_mi->inodes_per_group);
	SetDlgItemText(hwndPage1, IDC_STATIC_INODES_PER_GROUP, s);    
	wsprintf(s, "%lu", g_mi->blocks_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_BLOCK_COUNT, s);
	wsprintf(s, "%lu", g_mi->free_blocks_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_FREE_BLOCK_COUNT, s);    
	wsprintf(s, "%lu", g_mi->r_blocks_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_R_BLOCK_COUNT, s);       
	//wsprintf(s, "%lu", g_mi->block_size);
	SetDlgItemText(hwndPage1, IDC_STATIC_BLOCK_SIZE, size_to_spaced_string(g_mi->block_size));          

	SetDlgItemText(hwndPage1, IDC_STATIC_MTIME, time_to_string(g_mi->mtime));   

	wsprintf(s, "%lu", g_mi->mnt_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_MNT_COUNT, s);
	wsprintf(s, "%lu", g_mi->max_mnt_count);
	SetDlgItemText(hwndPage1, IDC_STATIC_MAX_MNT_COUNT, s);
	SetDlgItemText(hwndPage1, IDC_STATIC_STATE, get_fs_state(mi));

	DWORD volume = g_mi->block_size*g_mi->blocks_count/1024;
	if (volume < 1024) wsprintf(s, "%lu K", volume);
	else 
	{
		wsprintf(s, "%lu,%lu M", volume/1024, (int)((volume/1024.0 - (volume>>10))*10));
	}
	SetDlgItemText(hwndPage1, IDC_STATIC_VOLUME, s);

}

///////////////////////////////////////////////////////////////////////////

void set_file_page(HWND hwndPage2, common_file *file)
{
	char s[256];
	SetDlgItemText(hwndPage2, IDC_STATIC_FILE_NAME, file->file_name);
	SetDlgItemText(hwndPage2, IDC_STATIC_TYPE, get_type_from_file(file));
	SetDlgItemText(hwndPage2, IDC_STATIC_ATTR, get_attr_from_file(file));

	wsprintf(s, "%lu", file->i_links_count);
	SetDlgItemText(hwndPage2, IDC_STATIC_I_LINKS_COUNT, s);

	//wsprintf(s, "%lu", file->i_size);
	SetDlgItemText(hwndPage2, IDC_STATIC_I_SIZE, size_to_spaced_string(file->i_size));
	SetDlgItemText(hwndPage2, IDC_STATIC_CTIME, time_to_string(file->i_ctime));
	SetDlgItemText(hwndPage2, IDC_STATIC_MTIME, time_to_string(file->i_mtime));
	SetDlgItemText(hwndPage2, IDC_STATIC_ATIME, time_to_string(file->i_atime));

	SetDlgItemText(hwndPage2, IDC_STATIC_SUID, get_suid_from_file(g_file));
	SetDlgItemText(hwndPage2, IDC_STATIC_SGID, get_sgid_from_file(g_file));
	
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// I hate Delphi!:(
			g_hTabCtrl = GetDlgItem(hwndDlg, IDC_TAB1);
			HWND hwndPage1, hwndPage2;
			hwndPage1 = create_page(g_hTabCtrl, IDD_DIALOG_MI_PROPS_PAGE1);
			hwndPage2 = create_page(g_hTabCtrl, IDD_DIALOG_MI_PROPS_PAGE2);
			TCITEM tci;
			tci.mask = TCIF_TEXT | TCIF_PARAM;
			// first tab
			tci.pszText = Message(MSG_SUPERBLOCK);
			tci.lParam = (DWORD)hwndPage1;
			TabCtrl_InsertItem(g_hTabCtrl, 0, &tci);
			// and second one
			tci.pszText = Message(MSG_ROOTDIR);
			tci.lParam = (DWORD)hwndPage2;
			TabCtrl_InsertItem(g_hTabCtrl, 1, &tci);
			RECT rc, drc;
			GetWindowRect(g_hTabCtrl, &rc);
			drc = rc;
			TabCtrl_AdjustRect(g_hTabCtrl, FALSE, &drc);
			MoveWindow(hwndPage1, drc.left - rc.left, drc.top - rc.top, drc.right - drc.left, drc.bottom - drc.top, TRUE);
			MoveWindow(hwndPage2, drc.left - rc.left, drc.top - rc.top, drc.right - drc.left, drc.bottom - drc.top, TRUE);

			set_mi_page(hwndPage1, g_mi);
			set_file_page(hwndPage2, g_file);

			// show first page
			ShowWindow(hwndPage1, SW_SHOW);

			char s[256];
			wsprintf(s, "%s: %s", Message(MSG_PROPERTIES), g_disk_name);
			SetWindowText(hwndDlg, s);

		}
		bResult = TRUE;
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
		case WM_NOTIFY:
			{
				LPNMHDR lpnmhdr = (LPNMHDR)lParam;
				int iCurrentSel;
				TCITEM tci;
			
				switch(lpnmhdr->code)
				{
				case TCN_SELCHANGE:
					iCurrentSel = TabCtrl_GetCurSel(g_hTabCtrl);
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(g_hTabCtrl, iCurrentSel,  &tci);
					ShowWindow((HWND)tci.lParam, SW_SHOW);
					UpdateWindow((HWND)tci.lParam);
					break;
					
				case TCN_SELCHANGING:
					iCurrentSel = TabCtrl_GetCurSel(g_hTabCtrl);
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(g_hTabCtrl, iCurrentSel, &tci);
					ShowWindow((HWND)tci.lParam, SW_HIDE);
					bResult = FALSE; // allow to change tab
					break;
				}
			}
			break;
			
	}
	return bResult;
}

////////////////////////////////////////////////////////

void create_mi_propdlg(explorer *e, const char *disk_name, mount_info *mi, common_file *file)
{
	g_mi = mi;
	g_file = file;
	g_disk_name = disk_name;

	DialogBox((HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_MI_PROPS),
		e->hwndMainWindow,
		DialogProc);
}

////////////////////////////////////////////////////////

