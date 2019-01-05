#include "stdafx.h"
#include "explorer.h"
#include "exp_core.h"
#include "eo_comp.h"
#include "props.h"

enum {EO_COMP_COLUMN1};
static char msg[][50] = {"Èìÿ"};

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_preparecolumns(explorer *e, DWORD data)
{
	int column = 0;
	while(ListView_DeleteColumn(e->hwndFilebar, 0));

	LVCOLUMN cm;
	cm.mask = LVCF_TEXT | LVCF_WIDTH;
	cm.pszText = msg[EO_COMP_COLUMN1];
	cm.cx = 200;
	ListView_InsertColumn(e->hwndFilebar, 0, &cm);

	return 1/*success*/;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_db_open(explorer *e, DWORD data)
{
	// set filebar
	set_explorer_caption(e, NULL);

    TCHAR status[MAX_STATUS];
    wsprintf(status, "%s %lu", Message(MSG_OBJECTS), e->num_strg - 1);
    set_explorer_status(e, status, 0);

	eo_comp_data *fb_data = new eo_comp_data;
	assert(fb_data);
	fb_data->num_of_strg = e->num_strg - 1/* minus virtual storage*/;

	return (DWORD)fb_data;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_db_expand(explorer *e, DWORD data)
{
	return TRUE; // computer object is expandable:)
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_callback(explorer *e, LPARAM lParam)
{
	NMLVDISPINFO *dispinfo = (NMLVDISPINFO *)lParam;
	LVITEM *pItem = &dispinfo->item;
	if (pItem->iItem < e->num_strg - 1)
	{
		int strg_index = pItem->iItem + 1;
		if (pItem->mask && LVIF_TEXT)
		{
			pItem->pszText = e->strg[strg_index].common_name;
			pItem->cchTextMax = lstrlen(pItem->pszText);
		}
		if (pItem->mask && LVIF_IMAGE)
		{
			pItem->iImage = e->strg[strg_index].icon;
		}
		if (pItem->mask && LVIF_INDENT)
		{
			pItem->iIndent = 0;
		}

	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_cleanup(explorer *e, DWORD data)
{
	assert(e->current_eo_fb == EO_COMPUTER_ID);
	delete (eo_comp_data *)e->fb_data;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_getitemcount(explorer *e, DWORD data)
{
	assert(e->current_eo_fb == EO_COMPUTER_ID);
	return ((eo_comp_data *)e->fb_data)->num_of_strg;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_props(explorer *e, DWORD data)
{
	int num_of_items = ListView_GetSelectedCount(e->hwndFilebar);
	
	if (num_of_items == 1)
	{
		BYTE si = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | LVNI_SELECTED) + 1;
		
		FS_HANDLE fs_handle = FS_INVALID_HANDLE;
		if (e->strg[si].fs_handle == FS_INVALID_HANDLE)
		{
			// not mounted
			fs_handle = mount(e->strg[si].storage_name);
		}
		else
		{
			DWORD chkdsk = check_disk(e, si, TRUE);
			if (chkdsk == DISK_READY) 
                fs_handle = e->strg[si].fs_handle;
			else if (chkdsk == DISK_NEED_REFRESH) 
                e->need_internal_refresh = si;
		}
		
		if (fs_handle != FS_INVALID_HANDLE)
		{
			mount_info mi;
			common_file file;
			if (get_mount_info(fs_handle, &mi) && get_file_prop(fs_handle, "/", &file))
				show_mount_props(e, e->strg[si].common_name, &mi, &file);
			else
                error_message(e, e->strg[si].common_name);
		}
		
		if (e->strg[si].fs_handle == FS_INVALID_HANDLE)
		{
			if (fs_handle == FS_INVALID_HANDLE)
                error_message(e, e->strg[si].common_name);
			else
                umount(fs_handle);
		}
	}
	else if (num_of_items > 1)
		MessageBox(e->hwndMainWindow, Message(MSG_CHOOSEONEDISK), Message(MSG_ERROR), MB_OK);
	
	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_refresh(explorer *e, DWORD data)
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_cancopy(explorer *e, DWORD data)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_db_openexpand(explorer *e, DWORD data)
{
	// set filebar
	set_explorer_caption(e, NULL);

	eo_comp_data *fb_data = new eo_comp_data;
	assert(fb_data);
	fb_data->num_of_strg = e->num_strg - 1/* minus virtual storage*/;

	return (DWORD)fb_data;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_comp_fb_changed(explorer *e, DWORD data)
{

    return 0 /* not used */;
}

//////////////////////////////////////////////////////////////////////////
