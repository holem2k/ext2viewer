#include "stdafx.h"
#include "exp_core.h"
#include "explorer.h"
#include "disk.h"
#include "eo_comp.h"
#include "options.h"
#include "eo_folder.h"
#include "warn_dlg.h"
#include "aboutdlg.h"
#include "registry.h"

enum DiskbarObjectName {TXT_MSG1, TXT_MSG2, TXT_MSG3};
static char lpDiskbarObjectName[][50] = {"Компьютер", "fd0", ""};


/////////////////////////////////////////////////////////////////////////

BOOL build_storage(explorer *e)
{
	TVINSERTSTRUCT tvis;
	TVITEM tvi;
	HTREEITEM hItem;
	// Computer EO
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM;
	tvi.pszText = lpDiskbarObjectName[TXT_MSG1];
	tvi.cchTextMax = lstrlen(lpDiskbarObjectName[TXT_MSG1]);
	tvi.iImage = ICO_DB_COMPUTER;
	tvi.lParam = MAKE_EO_ATTR(0, EO_COMPUTER_ID);

	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_ROOT;
	tvis.item  = tvi;

	HTREEITEM root = TreeView_InsertItem(e->hwndDiskbar, &tvis);

	if (!disk_init())
	{
		MessageBox(0, "Can't find libraries dio9x.dll/dionnt.dll", "Error", MB_OK);
		return FALSE;
	}

	e->num_strg = 1; // first (0) storage is virtual;
	// disk fd0
	if (disk_check(DISK_FD0) == DISK_EXIST)
	{
		// insert item in tree
		tvi.mask = TVIF_TEXT  | TVIF_PARAM | TVIF_CHILDREN | TVIF_SELECTEDIMAGE | TVIF_IMAGE;
		tvi.pszText = lpDiskbarObjectName[TXT_MSG2];
		tvi.cchTextMax = lstrlen(lpDiskbarObjectName[TXT_MSG2]);
		tvi.iImage = ICO_DB_FD3_5; 
		WORD data = 0;
		SET_FOLDER_INDEX(data, (BYTE)e->num_strg);
		tvi.lParam = MAKE_EO_ATTR(data, EO_FOLDER_ID);
		tvi.cChildren = 1;
		tvi.iSelectedImage = ICO_DB_FD3_5;
	
		tvis.hParent = root;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item = tvi;
		
		hItem = TreeView_InsertItem(e->hwndDiskbar, &tvis);
		
		// fill storage array
		e->strg[e->num_strg].fs_handle = 0;
		e->strg[e->num_strg].icon = ICO_FB_FD3_5;
		e->strg[e->num_strg].hTreeItem = hItem;
		e->strg[e->num_strg].atime = 0;
		lstrcpy(e->strg[e->num_strg].storage_name, "fd0");
		lstrcpy(e->strg[e->num_strg].common_name, "fd0");
		e->num_strg++;
	}
	// enum hdds
	BOOL access_denied = FALSE;
	for (int i = 0; i < 4; i++)
	{
		BYTE bDrive = DISK_HDA + i;
		DWORD result = disk_check(bDrive);
		if (result == DISK_EXIST)
		{
			// enum partitions on hdd
			PartitionInfo pi[256];
			int num_pi;
			if (num_pi = disk_get_partitions(bDrive, pi, 256))
			{
				for (int j = 0; j < num_pi; j++)
				{
					if (pi[j].type == PARTITION_LINUX)
					{
						tvi.mask = TVIF_TEXT  | TVIF_PARAM | TVIF_CHILDREN | TVIF_SELECTEDIMAGE | TVIF_IMAGE;
						char part_name[256];
						wsprintf(part_name, "%s%s", lpDiskbarObjectName[TXT_MSG3], pi[j].part_name);
						tvi.pszText = part_name;
						tvi.cchTextMax = lstrlen(part_name);
						tvi.iImage = ICO_DB_HDD; 
						WORD data = 0;
						SET_FOLDER_INDEX(data, (BYTE)e->num_strg);
						tvi.lParam = MAKE_EO_ATTR(data, EO_FOLDER_ID);
						tvi.cChildren = 1;
						tvi.iSelectedImage = ICO_DB_HDD;
						
						tvis.hParent = root;
						tvis.hInsertAfter = TVI_LAST;
						tvis.item = tvi;
						
						hItem = TreeView_InsertItem(e->hwndDiskbar, &tvis);
						
						// fill storage array
						e->strg[e->num_strg].fs_handle = 0/**/;
						e->strg[e->num_strg].icon = ICO_FB_HDD;
						e->strg[e->num_strg].hTreeItem  = hItem;
						e->strg[e->num_strg].atime = 0;
						lstrcpy(e->strg[e->num_strg].storage_name, pi[j].part_name);
						lstrcpy(e->strg[e->num_strg].common_name, pi[j].part_name);
						e->num_strg++;
					}
				}
			}
			else
			{
				char s[256];
				wsprintf(s, Message(MSG_BADPARTITIONTABLE), 'a' + bDrive - DISK_HDA);
				MessageBox(0, s, "ext2viewer", MB_OK);
			}
		}
		else if (result == DISK_ACCESSDENIED)
		{
			access_denied = TRUE;
		}
	}

	if (access_denied)
        show_warning(e);

	disk_done();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////

void explorer_init_obj(explorer *e)
{
	for (int i = 0; i < MAX_E_OBJECT; i++)
    {
		for (int j = 0; j < MAX_E_OP; j++)
            e->eo[i].op[j] = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////

DWORD run_e_op(explorer *e, EO_ID eo_id, EO_OP op_id, DWORD data)
{
	EXPLOREROP op = (EXPLOREROP)e->eo[eo_id].op[op_id];
	return op ? op(e, data) : 0;
}


/////////////////////////////////////////////////////////////////////////

void set_explorer_caption(explorer *e, LPCTSTR lpCaption)
{
	char s[512];

	if (lpCaption)
        wsprintf(s, "%s - %s", lpApplicationName, lpCaption);
	else
        wsprintf(s, "%s", lpApplicationName);

	SetWindowText(e->hwndMainWindow, s);
}

/////////////////////////////////////////////////////////////////////////

void set_explorer_status(explorer *e, LPCTSTR lpStatus1, LPCTSTR lpStatus2)
{
    SendMessage(e->hwndStatus, SB_SETTEXT, 0, (LPARAM)lpStatus1);
    SendMessage(e->hwndStatus, SB_SETTEXT, 1,  (LPARAM)lpStatus2);
}

/////////////////////////////////////////////////////////////////////////

void set_fb_style(explorer *e, FB_STYLE style)
{
	LONG  lStyle = GetWindowLong(e->hwndFilebar, GWL_STYLE);
	lStyle &= ~(LVS_REPORT | LVS_ICON | LVS_SMALLICON | LVS_LIST);
	switch (style)
	{
	case FB_STYLE_ICON:
		lStyle |= LVS_ICON;
		e->fb_style = FB_STYLE_ICON;
		break;
	case FB_STYLE_SMALLICON:
		lStyle |= LVS_SMALLICON;
		e->fb_style = FB_STYLE_SMALLICON;
		break;
	case FB_STYLE_LIST:
		lStyle |= LVS_LIST;
		e->fb_style = FB_STYLE_LIST;
		break;
	case FB_STYLE_REPORT:
		lStyle |= LVS_REPORT;
		e->fb_style = FB_STYLE_REPORT;
		break;
	}
	SetWindowLong(e->hwndFilebar, GWL_STYLE, lStyle);
	SetWindowPos(e->hwndFilebar, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	if (style != FB_STYLE_REPORT) 
		ListView_SetColumnWidth(e->hwndFilebar, 0, 130);
	UpdateWindow(e->hwndFilebar);
}

/////////////////////////////////////////////////////////////////////////

void init(explorer *e)
{
    //// default settings
    e->bstatus = TRUE;                  // status bar 
	e->fb_style = FB_STYLE_ICON ;      // listview style
	e->fb_sort_order = FB_SORT_BY_NAME; // sort order

    // columns
	for (int i = 0; i < COL_MAX; i++)
        e->fb_columns[i] = -1;
	e->fb_columns[0] = COL_NAME;
	e->fb_columns[1] = COL_SIZE;
	e->fb_columns[2] = COL_MTIME;

    //// load settings
    DWORD option;
    if (reg_getnum(PVAL_STATUS, &option))
        e->bstatus = (BOOL)option;

    if (reg_getnum(PVAL_STYLE, &option)) 
        e->fb_style = (FB_STYLE)option;

    if (reg_getnum(PVAL_SORTBY, &option))
        e->fb_sort_order = (FB_SORT_ORDER)option;


    // show/hide status bar window
    ShowWindow(e->hwndStatus, e->bstatus ? SW_SHOW : SW_HIDE);
    // set listview window style
    set_fb_style(e, e->fb_style);

    // select tree root 
	HTREEITEM hRoot = TreeView_GetRoot(e->hwndDiskbar);
	e->hOpenObject = hRoot;
	run_e_op(e, EO_COMPUTER_ID, OP_FB_PREPARECOLUMNS, 0);
    e->intchange = TRUE;
	TreeView_SelectItem(e->hwndDiskbar, hRoot);
	/* this invoke indirect call of 
	     run_e_op(e, EO_COMPUTER_ID, OP_DB_OPEN, 0)*/
	TreeView_Expand(e->hwndDiskbar, hRoot, TVE_EXPAND);
}

/////////////////////////////////////////////////////////////////////////

void explorer_busy(explorer *e, BOOL busy)
{
	HCURSOR hCursor = busy ? LoadCursor(NULL, IDC_WAIT) : LoadCursor(NULL, IDC_ARROW);
	SetCursor(hCursor);
}

/////////////////////////////////////////////////////////////////////////

void set_explorer_options(explorer *e)
{
	options opt;
	// set options
	for (int i = 0; i < COL_MAX; i++) opt.fb_columns[i] = e->fb_columns[i];
	if (CreateOptionsDialog(e->hwndMainWindow, &opt) == IDOK)
	{
		for (int i = 0; i < COL_MAX; i++) 
            e->fb_columns[i] = opt.fb_columns[i];
		if (e->current_eo_fb == EO_FOLDER_ID)
		{
			// adjust columns in filebar
			ListView_SetItemCount(e->hwndFilebar, 0);
			run_e_op(e, EO_FOLDER_ID, OP_FB_PREPARECOLUMNS, 0);
			DWORD ItemCount = run_e_op(e, EO_FOLDER_ID, OP_FB_GETITEMCOUNT, 0);
			ListView_SetItemCount(e->hwndFilebar, ItemCount);
			UpdateWindow(e->hwndFilebar);
		}
	}
}

/////////////////////////////////////////////////////////////////////////

void set_fbsort_order(explorer *e, FB_SORT_ORDER so)
{
	e->fb_sort_order = so;
	if (e->current_eo_fb == EO_FOLDER_ID)
	{
		eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
		sort_fb(folder_data, e->fb_sort_order);
		ListView_RedrawItems(e->hwndFilebar, 0, folder_data->num_of_files ? folder_data->num_of_files - 1 : 0);
		InvalidateRect(e->hwndFilebar, NULL,  FALSE);
		UpdateWindow(e->hwndFilebar);
	}
}

/////////////////////////////////////////////////////////////////////////

DWORD show_properties(explorer *e, BOOL checkonly)
{
	DWORD result = 0;

	// process diskbar
	HWND hwndFocused = GetFocus();
	if (hwndFocused == e->hwndDiskbar)
	{
		HTREEITEM hSelected = TreeView_GetSelection(e->hwndDiskbar);
		if (hSelected)
		{
			// get selected object id
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = hSelected;
			TreeView_GetItem(e->hwndDiskbar, &tvi);
			EO_ID eo_id = GET_EO_TYPE(tvi.lParam);
			// show properties dialog
			if (eo_id == EO_FOLDER_ID)
			{
				if (!checkonly)
				{
					run_e_op(e, eo_id, OP_DB_PROPS, (DWORD)hSelected);
					/* show selection on treeview item again */ 
					SetFocus(e->hwndDiskbar);
				}
				result = 1;
			}
		}
	}
	else if (hwndFocused == e->hwndFilebar)
	{
		// process filebar
		int iSelected = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | LVNI_FOCUSED);
		if (iSelected != -1)
		{
			/* one or more items in filebar  is selected */
			if (!checkonly)
			{
				run_e_op(e, e->current_eo_fb, OP_FB_PROPS, 0);
				/* show selection on treeview item again */ 
				SetFocus(e->hwndFilebar);
			}
			result = 1;
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////

void cleanup_explorer(explorer *e)
{
	// free resources
	run_e_op(e, e->current_eo_fb, OP_FB_CLEANUP, 0);
	// unmount 
	for (int i = 1/*pass "software" folder*/; i < e->num_strg; i++)
	{
		if (e->strg[i].fs_handle != FS_INVALID_HANDLE)
			umount(e->strg[i].fs_handle);
	}

    // save settins
    reg_setnum(PVAL_STATUS, (DWORD)e->bstatus);
    reg_setnum(PVAL_STYLE, (DWORD)e->fb_style);
    reg_setnum(PVAL_SORTBY, (DWORD)e->fb_sort_order);
    reg_delval(PVAL_PATH);
}

/////////////////////////////////////////////////////////////////////////

int error_message(explorer *e, const char *folder_name)
{
	HWND hwndFocused = GetFocus();
	char lpText[16384];
	wsprintf(lpText, Message(MSG_DEVICENOTREADY), folder_name);
	int result = MessageBox(e->hwndMainWindow, lpText, Message(MSG_ERROR), MB_OK | MB_ICONERROR);

	if (hwndFocused == e->hwndDiskbar)
	{
		if (TreeView_GetSelection(e->hwndDiskbar))
            SetFocus(hwndFocused);
	}
    else
        SetFocus(hwndFocused);
	return result;
}

/////////////////////////////////////////////////////////////////////////

int error_message2(explorer *e, const char *folder_name)
{
	HWND hwndFocused = GetFocus();
	char lpText[16384];
	wsprintf(lpText, Message(MSG_DEVICENOTREADY), folder_name);
	int result = MessageBox(e->hwndMainWindow, lpText, Message(MSG_ERROR), MB_RETRYCANCEL | MB_ICONERROR);
	if (hwndFocused == e->hwndDiskbar)
	{
		if (TreeView_GetSelection(e->hwndDiskbar)) 
            SetFocus(hwndFocused);
	} 
    else
        SetFocus(hwndFocused);
	return result;
}

/////////////////////////////////////////////////////////////////////////

int error_message3(explorer *e, const char *folder_name)
{
	HWND hwndFocused = GetFocus();
	char lpText[16384];
	wsprintf(lpText, Message(MSG_DISKCHANGED), folder_name);
	int result = MessageBox(e->hwndMainWindow, lpText, "", MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
	if (hwndFocused == e->hwndDiskbar)
	{
		if (TreeView_GetSelection(e->hwndDiskbar))
            SetFocus(hwndFocused);
	}
    else 
        SetFocus(hwndFocused);
	return result;
}

/////////////////////////////////////////////////////////////////////////

int error_message4(explorer *e, const char *folder_name)
{
	HWND hwndFocused = GetFocus();
	char lpText[16384];
	wsprintf(lpText, Message(MSG_ACCESSDENIED), folder_name);
	int result = MessageBox(e->hwndMainWindow, lpText, "", MB_RETRYCANCEL | MB_ICONERROR | MB_DEFBUTTON1);
	if (hwndFocused == e->hwndDiskbar)
	{
		if (TreeView_GetSelection(e->hwndDiskbar)) 
            SetFocus(hwndFocused);
	}
    else 
        SetFocus(hwndFocused);
	return result;
}

/////////////////////////////////////////////////////////////////////////

int error_message5(explorer *e, const char *folder_name)
{
	HWND hwndFocused = GetFocus();
	char lpText[16384];
	wsprintf(lpText, Message(MSG_DISKFULL), folder_name);
	int result = MessageBox(e->hwndMainWindow, lpText, "", MB_RETRYCANCEL | MB_ICONERROR | MB_DEFBUTTON1);
	if (hwndFocused == e->hwndDiskbar)
	{
		if (TreeView_GetSelection(e->hwndDiskbar))
            SetFocus(hwndFocused);
	} 
    else
        SetFocus(hwndFocused);
	return result;
}

/////////////////////////////////////////////////////////////////////////

int error_message6(explorer *e)
{
	HWND hwndFocused = GetFocus();

	int result = MessageBox(e->hwndMainWindow, Message(MSG_NAMETOOLONG), "", MB_OK);
	if (hwndFocused == e->hwndDiskbar)
	{
		if (TreeView_GetSelection(e->hwndDiskbar))
            SetFocus(hwndFocused);
	} 
    else
        SetFocus(hwndFocused);
	return result;
}

/////////////////////////////////////////////////////////////////////////
/* rem: Ни один из элементов сворачиваемого дерева не должен быть выделен */

void collapse_storage(explorer *e, BYTE fi)
{
	TreeView_Expand(e->hwndDiskbar, e->strg[fi].hTreeItem, TVE_COLLAPSE | TVE_COLLAPSERESET);

	// clear 'mounted' and 'expanded' flags
	TVITEM tvi;
	tvi.mask = TVIF_PARAM;
	tvi.hItem = e->strg[fi].hTreeItem; 
	TreeView_GetItem(e->hwndDiskbar, &tvi);
	tvi.mask = TVIF_PARAM;
	tvi.lParam = MAKE_EO_ATTR(GET_EO_DATA(tvi.lParam) & 0xFF, GET_EO_TYPE(tvi.lParam));
	TreeView_SetItem(e->hwndDiskbar, &tvi);
}

/////////////////////////////////////////////////////////////////////////
//  Обновление диска при клике по выделенному значку диска

int refresh_by_click(explorer *e)
{
	TVHITTESTINFO tvhti;
	GetCursorPos(&tvhti.pt);
	ScreenToClient(e->hwndDiskbar, &tvhti.pt);
	TreeView_HitTest(e->hwndDiskbar, &tvhti);
	int result = 0;
	if (tvhti.flags & TVHT_ONITEMICON || tvhti.flags & TVHT_ONITEMLABEL)
	{
		for (int i = 1; i < e->num_strg; i++)
		{
			if (e->strg[i].hTreeItem == tvhti.hItem && e->hOpenObject == tvhti.hItem )
			{
				//////////
				TR("Refresh\n");
				/////////
				run_e_op(e, EO_FOLDER_ID, OP_REFRESH, (DWORD)tvhti.hItem);
				result = 1;
				break;
			}
		}
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////
// Обновление диска при нажатии на нем 'Enter'

void refresh_by_return(explorer *e)
{
	for (int i = 1; i < e->num_strg; i++)
	{
		if (e->strg[i].hTreeItem == e->hOpenObject)
		{
			run_e_op(e, EO_FOLDER_ID, OP_REFRESH, (DWORD)e->hOpenObject);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////

DWORD check_disk(explorer *e, BYTE fi, BOOL ask_remount)
{
	//return DISK_READY;
	DWORD result = DISK_NOT_READY;

	DWORD rmv = is_removable(e->strg[fi].fs_handle);
	if (rmv == DISK_REMOVABLE)
	{
/*		time_t t;
		time(&t);
		char s[256];
		wsprintf(s, "%u\n", t - e->strg[fi].atime);
		OutputDebugString(s);
		if (t - e->strg[fi].atime <= 3)
		{
			e->strg[fi].atime = t;
			return DISK_READY;
		}
		e->strg[fi].atime = t;
*/
		BOOL retry;
		do
		{
			retry = FALSE;
			FS_HANDLE fs_handle = mount(e->strg[fi].storage_name);
			if (fs_handle == FS_INVALID_HANDLE)
			{
				if (error_message2(e, e->strg[fi].common_name) == IDRETRY) 
					retry = TRUE;
			}
			else
			{
				if (!is_same_removable(fs_handle, e->strg[fi].fs_handle))
				{
					if (!ask_remount || error_message3(e, e->strg[fi].common_name) == IDYES)
						result = DISK_NEED_REFRESH;
					else
						result = DISK_NOT_READY;
				}
				else
                    result = DISK_READY;

				umount(fs_handle);
			}
		}
        while (retry);
	}
	else if (rmv == DISK_FIXED)	
        result = DISK_READY;

	return result;
}


/////////////////////////////////////////////////////////////////////////

void refresh_removable(explorer *e)
{
	/* disk refresh. stage 2 - read data for new disk tree */
	refresh_struct *rs = &e->rs;
	HTREEITEM hItem = e->strg[rs->strg_indx].hTreeItem;
	HWND hwndDiskbar = e->hwndDiskbar;

	TVITEM tvi;
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hItem;
	TreeView_GetItem(hwndDiskbar, &tvi);

	rs->refresh_data = (void *)run_e_op(e, EO_FOLDER_ID,
		rs->expand ? OP_DB_OPENEXPAND : OP_DB_OPEN, (DWORD)&tvi);

	if (rs->refresh_data)
	{
		/* go stage 3 - select disk icon */
		TreeView_SelectItem(hwndDiskbar, hItem); 
		if (rs->expand) 
            TreeView_Expand(hwndDiskbar, hItem, TVE_EXPAND); // eo_folder_expand not really called, cause EO_FOLDER_EXPANDED is set
	}
	else 
	{
		TreeView_SelectItem(hwndDiskbar, rs->hFailureItem);
		SetFocus(e->hwndDiskbar);
	}

}	

/////////////////////////////////////////////////////////////////////////

DWORD explorer_copy(explorer *e, BOOL onlycheck)
{
	DWORD result = 0;
	HWND hwndFocused = GetFocus();

	if (onlycheck)
	{
		if (hwndFocused == e->hwndFilebar) 
			result = run_e_op(e, e->current_eo_fb, OP_FB_CANCOPY, 0);
        else 
		{
			if (hwndFocused == e->hwndDiskbar)
				result = run_e_op(e, e->current_eo_fb, OP_DB_CANCOPY, 0);
		}
	}
	else
	{
		if (hwndFocused == e->hwndFilebar)
			result = run_e_op(e, e->current_eo_fb, OP_FB_COPY, 0);
		else
		{
			if (hwndFocused == e->hwndDiskbar)
				result = run_e_op(e, e->current_eo_fb, OP_DB_COPY, 0);
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////

void explorer_about(explorer *e)
{
	create_aboutdlg(e->hwndMainWindow);
}

/////////////////////////////////////////////////////////////////////////

void context_menu(explorer *e)
{
	POINT p;
	GetCursorPos(&p);
	HWND hwndFocused = GetFocus();
	if (hwndFocused == e->hwndFilebar)
	{
		int iMenu = 0;
		LVHITTESTINFO info;
		POINT cp = p;
		ScreenToClient(e->hwndFilebar, &cp);
		info.pt = cp;
		ListView_HitTest(e->hwndFilebar, &info);
		if (info.iItem != -1 && ((info.flags & LVHT_ONITEMICON) | (info.flags & LVHT_ONITEMLABEL)))
		{
			iMenu = 1;
			//  TO DO - run_e_op(... OP_DB_CNTX_MENU);
		}

		HINSTANCE hInstance = (HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE);
		HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_POPUPS));
		HMENU hPopup = GetSubMenu(hMenu, iMenu);
		RECT rc;
		TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
			p.x, p.y, 0, e->hwndMainWindow, &rc);
		DestroyMenu(hMenu);
	}
}

/////////////////////////////////////////////////////////////////////////

void toggle_status(explorer *expl)
{
    ShowWindow(expl->hwndStatus, expl->bstatus = !expl->bstatus ? SW_SHOW : SW_HIDE);

    RECT rect;
    GetClientRect(expl->hwndMainWindow, &rect);
    SendMessage(expl->hwndMainWindow, WM_SIZE, SIZE_RESTORED,
        ((rect.bottom - rect.top)<<16) + rect.right - rect.left);
}

/////////////////////////////////////////////////////////////////////////

DWORD explorer_view(explorer *e, BOOL onlycheck)
{
	DWORD result = 0;
	HWND hwndFocused = GetFocus();

	if (onlycheck)
	{
		if (hwndFocused == e->hwndFilebar) 
			result = run_e_op(e, e->current_eo_fb, OP_FB_CANVIEW, 0);
	}
	else
	{
		if (hwndFocused == e->hwndFilebar)
			result = run_e_op(e, e->current_eo_fb, OP_FB_VIEW, 0);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////