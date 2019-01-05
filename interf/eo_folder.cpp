#include <stdafx.h>
#include "eo_folder_core.h"
#include "eo_folder.h"
#include "exp_core.h"
#include "ext2_fs.h"
#include "stat.h"
#include "qsort.h"
#include "props.h"
#include "copy.h"
#include "view.h"

static const char *lpMsg[] = {"Ошибка !", "Нет доступа к %s.\n\nУстройство не готово."};
static const char *lpMsg2[] = {"Имя", "Тип", "Размер", "Права доступа", "Время создания", "Время модификации", "Время доступа", "Пользователь", "Группа"};
const int ColumnWidth[] = {100, 100, 100, 100, 100, 130, 100, 100, 100};
const int ColumnFmt[] = {LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_LEFT, LVCFMT_LEFT};


enum EO_FOLDER_MSG{MSG1, MSG2};

/////////////////////////////////////////////////////////////////////////

void sort_fb(eo_folder_data *folder_data, FB_SORT_ORDER fb_sort_order)
{
	SORT_ORDER so;
	switch (fb_sort_order)
	{
	case FB_SORT_BY_NAME: so = SORT_BY_NAME;
		break;
	case FB_SORT_BY_TYPE: so = SORT_BY_TYPE; 
		break;
	case FB_SORT_BY_SIZE: so = SORT_BY_SIZE;
		break;
	case FB_SORT_BY_DATE: so = SORT_BY_DATE;
		break;
	case FB_SORT_BY_NONE: so = SORT_BY_NONE;
		break;
	default:
		assert(0);
	}
	
	sort(folder_data->files, folder_data->sorted_files_order, folder_data->num_of_files, so);
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_preparecolumns(explorer *e, DWORD data)
{
	//int column = 0;
	while (ListView_DeleteColumn(e->hwndFilebar, 0));

	LVCOLUMN cm;
	cm.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

	int column_indx = 0;
	for (int i = 0; i <= COL_MAX - 1; i++)
	{
		int column = e->fb_columns[i];
		if (column == -1) continue;
		cm.cx = ColumnWidth[column];
		cm.pszText = (char *)lpMsg2[column];
		cm.fmt = ColumnFmt[column];
		ListView_InsertColumn(e->hwndFilebar, column_indx++, &cm);
	}
	return 1; /*success*/
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_db_open(explorer *e, DWORD data)
{
	TVITEM  *ptvi = (TVITEM *)data;
	eo_folder_data *folder_data = NULL;

	if (supermount(e, ptvi))
	{
		char folder_name[MAXFILENAME];  BYTE fi;
		build_foldername(e->hwndDiskbar, ptvi->hItem, folder_name, MAXFILENAME, fi);

		BOOL ask_remount = lstrcmp(folder_name, "/");
		BOOL retry;
		do
		{
			retry = FALSE;
			
			DWORD chkres = check_disk(e, fi, ask_remount);
			if (chkres == DISK_READY)
			{
				common_file *files;
				DWORD num_of_files;
				
				explorer_busy(e, TRUE);
				BOOL benum = start_enum(e->strg[fi].fs_handle, folder_name, files, num_of_files);
				explorer_busy(e, FALSE);
				
				if (benum)
				{
					folder_data = new eo_folder_data; 
					folder_data->files = files;
					folder_data->num_of_files = num_of_files;
					folder_data->sorted_files_order = new DWORD[num_of_files];

                    DWORD folder_size = 0;
					for (DWORD i = 0; i < num_of_files; i++)
                    {
                        folder_data->sorted_files_order[i] = i;
                        if (files[i].type != FT_DIR)
                            folder_size += files[i].i_size;
                    }
				
					set_explorer_caption(e, folder_name);
					sort_fb(folder_data, e->fb_sort_order);
				}
				else if (error_message2(e, folder_name) == IDRETRY) 
                    retry = TRUE;

			} else if (chkres == DISK_NEED_REFRESH)
                e->need_internal_refresh = fi;

		}
        while (retry);
	}
	
	return (DWORD)folder_data;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_db_expand(explorer *e, DWORD data)
{
	NMTREEVIEW *pnmtv = (NMTREEVIEW *)data;
	BOOL result = FALSE;
	WORD folder = GET_EO_DATA(pnmtv->itemNew.lParam);

	if (!(folder & EO_FOLDER_EXPANDED))
	{
		if (supermount(e, &pnmtv->itemNew))
		{
			// get full folder name
			char folder_name[MAXFILENAME];
			BYTE fi;
			
			build_foldername(e->hwndDiskbar, pnmtv->itemNew.hItem, folder_name, MAXFILENAME, fi);
			BOOL ask_remount = TRUE;
			BOOL retry;

			do
			{
				retry = FALSE;
				
				DWORD chkres = check_disk(e, fi, ask_remount);
				if (chkres == DISK_READY)
				{
					directory *dirs;
					DWORD num_of_dirs;
					BOOL benum;
					
					explorer_busy(e, TRUE);
					benum = start_enum_dirs(e->strg[fi].fs_handle, folder_name, dirs, num_of_dirs);
					explorer_busy(e, FALSE);
					
					if (benum)
					{
						TVINSERTSTRUCT tvis;
						tvis.hParent = pnmtv->itemNew.hItem;
						tvis.hInsertAfter = TVI_SORT;
						for (DWORD i = 0; i < num_of_dirs; i++)
						{
							tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM 
								| TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
							tvis.item.pszText = dirs[i].dir_name;
							tvis.item.cchTextMax = lstrlen(dirs[i].dir_name);
							tvis.item.cChildren = dirs[i].has_subdir;
							tvis.item.lParam = MAKE_EO_ATTR(EO_FOLDER_MOUNTED, EO_FOLDER_ID);
							tvis.item.iImage = ICO_DB_FOLDER;
							tvis.item.iSelectedImage = ICO_DB_OFOLDER;
							
							HTREEITEM a = TreeView_InsertItem(e->hwndDiskbar, &tvis);
						}
						cancel_enum_dirs(dirs);
						// set new folder state
						TVITEM ntvi;
						ntvi.mask = TVIF_PARAM;
						ntvi.hItem = pnmtv->itemNew.hItem;
						ntvi.lParam = MAKE_EO_ATTR(folder | EO_FOLDER_EXPANDED | EO_FOLDER_MOUNTED, EO_FOLDER_ID);
						TreeView_SetItem(e->hwndDiskbar, &ntvi);
						
						result = TRUE;
					}
					else  if (error_message2(e, folder_name) == IDRETRY)
                        retry = TRUE;
				}
				else if (chkres == DISK_NEED_REFRESH)
                    e->need_internal_refresh = fi;

			}
            while (retry);
		}
	} else
        result = TRUE;
	
	return result;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_db_openexpand(explorer *e, DWORD data)
{
	TVITEM  *ptvi = (TVITEM *)data;
	eo_folder_data *folder_data = NULL;
	WORD folder = GET_EO_DATA(ptvi->lParam);   // now folder & EO_FOLDER_EXPANDED =  0 always

	if (supermount(e, ptvi))
	{
		char folder_name[MAXFILENAME];  BYTE fi;
		build_foldername(e->hwndDiskbar, ptvi->hItem, folder_name, MAXFILENAME, fi);

		BOOL ask_remount = lstrcmp(folder_name, "/");
		BOOL retry;
		do
		{
			retry = FALSE;
			
			DWORD chkres = check_disk(e, fi, ask_remount);
			if (chkres == DISK_READY)
			{
				common_file *files;
				DWORD num_of_files;
				directory *dirs;
				DWORD num_of_dirs;
				BOOL benum1, benum2;
				
				explorer_busy(e, TRUE);
				benum1 = start_enum(e->strg[fi].fs_handle, folder_name, files, num_of_files);
				explorer_busy(e, FALSE);

				if (benum1 && !(folder & EO_FOLDER_EXPANDED))
				{
					explorer_busy(e, TRUE);
					benum2 = start_enum_dirs(e->strg[fi].fs_handle, folder_name, dirs, num_of_dirs);
					explorer_busy(e, FALSE);
				}
				else
                    benum2 = TRUE;

				if (benum1 && benum2)
				{
					DWORD i;
					// open folder
					folder_data = new eo_folder_data; 
					folder_data->files = files;
					folder_data->num_of_files = num_of_files;
					folder_data->sorted_files_order = new DWORD[num_of_files];
					for (i = 0; i < num_of_files; i++)
                        folder_data->sorted_files_order[i] = i;
					
					set_explorer_caption(e, folder_name);
					sort_fb(folder_data, e->fb_sort_order);
				
					if (!(folder & EO_FOLDER_EXPANDED))
					{
						// expand folder
						TVINSERTSTRUCT tvis;
						tvis.hParent = ptvi->hItem;
						tvis.hInsertAfter = TVI_SORT;
						for (i = 0; i < num_of_dirs; i++)
						{
							tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM 
								| TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
							tvis.item.pszText = dirs[i].dir_name;
							tvis.item.cchTextMax = lstrlen(dirs[i].dir_name);
							tvis.item.cChildren = dirs[i].has_subdir;
							tvis.item.lParam = MAKE_EO_ATTR(EO_FOLDER_MOUNTED, EO_FOLDER_ID);
							tvis.item.iImage = ICO_DB_FOLDER;
							tvis.item.iSelectedImage = ICO_DB_OFOLDER;
							
							TreeView_InsertItem(e->hwndDiskbar, &tvis);
						}
						cancel_enum_dirs(dirs);
					}

					// set new folder state
					TVITEM ntvi;
					ntvi.mask = TVIF_PARAM | TVIF_HANDLE;
					ntvi.hItem = ptvi->hItem;
					ntvi.lParam = MAKE_EO_ATTR(folder | EO_FOLDER_EXPANDED | EO_FOLDER_MOUNTED, EO_FOLDER_ID);
					TreeView_SetItem(e->hwndDiskbar, &ntvi);
				}
				else 
				{
					if (benum1)
                        cancel_enum(files); // if open was ok, but expand failed

					if (error_message2(e, folder_name) == IDRETRY)
                        retry = TRUE;
				}

			} 
            else if (chkres == DISK_NEED_REFRESH)
                e->need_internal_refresh = fi;

		}
        while (retry);
	}
	
	return (DWORD)folder_data;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_callback(explorer *e, DWORD data)
{
	BOOL result = FALSE;
	assert(e->current_eo_fb == EO_FOLDER_ID);

	NMLVDISPINFO *dispinfo = (NMLVDISPINFO *)data;
	LVITEM *pItem = &dispinfo->item;
	eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;

	
	if (pItem->iItem < (int)folder_data->num_of_files)
	{
		DWORD subj = e->fb_columns[pItem->iSubItem];
		DWORD file_num = folder_data->sorted_files_order[pItem->iItem];
		common_file *file = folder_data->files + file_num;

		if (pItem->mask & LVIF_TEXT)
		{
			switch(subj)
			{
			case COL_NAME:
				pItem->pszText = file->file_name;
				pItem->cchTextMax = lstrlen(file->file_name);
				break;

			case COL_TYPE:
				pItem->pszText = get_type_from_file(file);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				
				break;
			case COL_SIZE:
				pItem->pszText = get_size_from_file(file);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;

			case COL_RIGHTS:
				pItem->pszText = get_attr_from_file(file);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;

			case COL_CTIME:
				pItem->pszText = time_to_string(file->i_ctime);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;

			case COL_MTIME:
				pItem->pszText = time_to_string(file->i_mtime);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;

			case COL_ATIME:
				pItem->pszText = time_to_string(file->i_atime);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;

			case COL_USER:
				pItem->pszText = user_to_string(file->i_uid);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;

			case COL_GROUP:
				pItem->pszText = group_to_string(file->i_gid);
				pItem->cchTextMax = lstrlen(pItem->pszText);
				break;
			}
		}
		
		if (pItem->mask & LVIF_IMAGE)
		{
			pItem->iImage = get_icon_from_file(file);
		}
		if (pItem->mask & LVIF_INDENT)
		{
			pItem->iIndent = 0;
		}
		if (pItem->mask & LVIF_PARAM)
		{
			pItem->lParam = 22;
		}
		result = TRUE;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_cleanup(explorer *e, DWORD data)
{
	eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
	cancel_enum(folder_data->files);
	delete [] folder_data->sorted_files_order;
	delete folder_data;
	
	return 1/*ok*/;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_getitemcount(explorer *e, DWORD data)
{
	assert(e->current_eo_fb == EO_FOLDER_ID);
	return ((eo_folder_data *)e->fb_data)->num_of_files;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_db_props(explorer *e, DWORD data)
{
	BYTE si;
	char folder_name[MAXFILENAME];

	HTREEITEM hTreeItem = (HTREEITEM)data;
	build_foldername(e->hwndDiskbar, hTreeItem, folder_name, MAXFILENAME, si);

	DWORD chkdsk = check_disk(e, si, TRUE);
	if (chkdsk == DISK_READY)
	{
		if (!lstrcmp(folder_name, "/"))
		{
			// process root folder
			mount_info mi;
			BOOL result = FALSE;
			if (get_mount_info(e->strg[si].fs_handle, &mi))
			{
				common_file file;
				if (get_file_prop(e->strg[si].fs_handle, folder_name, &file))
				{
					show_mount_props(e, e->strg[si].common_name, &mi, &file);
					result = TRUE;
				}
			}
			if (!result) 
                error_message(e, e->strg[si].common_name);
		}
		else
		{
			// process folder
			build_foldername(e->hwndDiskbar, hTreeItem, folder_name, MAXFILENAME, si);
			common_file file;
			if (get_file_prop(e->strg[si].fs_handle, folder_name, &file))
			{
				char *exinfo = Message(MSG_CONTAIN);
				char exinfotext[256];
				assert(e->current_eo_fb == EO_FOLDER_ID);
				eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
				wsprintf(exinfotext, "%s %lu", Message(MSG_FILESANDFOLDERS), folder_data->num_of_files);
				show_file_props(e, &file, exinfo, exinfotext);
			}
			else 
                error_message(e, folder_name);
		}
	}
	else if (chkdsk == DISK_NEED_REFRESH)
        e->need_internal_refresh = si;

	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_props(explorer *e, DWORD data)
{
	int num_of_items = ListView_GetSelectedCount(e->hwndFilebar);
	if (num_of_items == 0) return 0;
	
	BYTE si;
	char folder_name[MAXFILENAME];
	build_foldername(e->hwndDiskbar, e->hOpenObject, folder_name, MAXFILENAME, si);
	DWORD chkdsk = check_disk(e, si, TRUE);
	if (chkdsk == DISK_READY)
	{

		if (num_of_items == 1)
		{
			int sel = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | LVNI_SELECTED);
			eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
			if ((DWORD)sel < folder_data->num_of_files)
			{
				common_file *file = folder_data->files + folder_data->sorted_files_order[sel];
				if (file->type == FT_CHR || file->type == FT_BLK)
				{
					char prop[PROP_LEN];
					char text[20];
					wsprintf(prop, Message(MSG_DEVICE));
					wsprintf(text, get_size_from_file(file));
					show_file_props(e, file, prop, text);
				}
				else if (file->type == FT_LNK)
				{
					char prop[PROP_LEN];
					wsprintf(prop, Message(MSG_LINK));
					char link[LINK_LEN];
					// make file name
					char file_name[MAXFILENAME + 256];
					lstrcpy(file_name, folder_name);
					if (file_name[1]) lstrcat(file_name, "/");
					lstrcat(file_name, file->file_name);
					
					if (resolve_sym_link(e->strg[si].fs_handle, file_name, link, LINK_LEN));
					else
                        lstrcpy(link, "");
					show_file_props(e, file, prop, link);
				}
				else
                    show_file_props(e, file, NULL, NULL);
			}
		}
		else if (num_of_items > 1)
		{
			eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
			DWORD *files_indx = new DWORD[num_of_items];
			
			int num_of_files = 0;
			int file_indx = -1;
			while ((file_indx = ListView_GetNextItem(e->hwndFilebar, file_indx,
				LVNI_ALL | LVNI_SELECTED)) != -1) 
			{
				files_indx[num_of_files] = folder_data->sorted_files_order[file_indx];
				num_of_files++;
			}
			show_files_props(e, folder_data->files, files_indx, num_of_files);
			delete [] files_indx;
		}
	}
	else if (chkdsk == DISK_NEED_REFRESH)
        e->need_internal_refresh = si;
	
	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_refresh(explorer *e, DWORD data)
{
	BYTE si;
	HTREEITEM hTreeItem = (HTREEITEM)data;
	char folder_name[MAXFILENAME];
	// get disk storage index
	build_foldername(e->hwndDiskbar, hTreeItem, folder_name, MAXFILENAME, si);

	// check is selection on refreshed tree ?
	BOOL selkill = TRUE;
	HTREEITEM hSelection = TreeView_GetSelection(e->hwndDiskbar);
	if (hSelection)
	{
		BYTE nsi;
		if (build_foldername(e->hwndDiskbar, hSelection, folder_name, MAXFILENAME, nsi))
			if (nsi != si)
                selkill = FALSE;
	}

	if (is_removable(e->strg[si].fs_handle) == DISK_REMOVABLE)
	{
		BOOL retry;
		do 
		{
			retry = FALSE;
			FS_HANDLE fs_handle = mount(e->strg[si].storage_name);
			if (fs_handle == FS_INVALID_HANDLE)
			{
				// removable disk is corrupted or there is no disk
				if (error_message2(e,  e->strg[si].storage_name) == IDRETRY)
                    retry = TRUE;
				else 
				{
					if (selkill) 
                        TreeView_SelectItem(e->hwndDiskbar, TreeView_GetRoot(e->hwndDiskbar));

					collapse_storage(e, si);
					umount(e->strg[si].fs_handle);
					e->strg[si].fs_handle = FS_INVALID_HANDLE;
				}
			}
			else
			{
				if (is_same_removable(fs_handle, e->strg[si].fs_handle)) 
				{
					// same disk
					umount(fs_handle);
				}
				else
				{
					/* disk refresh. Stage 1 - collapse tree */
					e->rs.hFailureItem = selkill ? TreeView_GetRoot(e->hwndDiskbar)
						: TreeView_GetSelection(e->hwndDiskbar);
					TreeView_SelectItem(e->hwndDiskbar, NULL);

					TVITEM tvi;
					tvi.mask = TVIF_PARAM | TVIF_STATE;
					tvi.hItem = e->strg[si].hTreeItem;
					tvi.stateMask = TVIS_EXPANDED;
					TreeView_GetItem(e->hwndDiskbar, &tvi);
					BOOL expand = tvi.state & TVIS_EXPANDED;

					collapse_storage(e, si);
					umount(e->strg[si].fs_handle);
					e->strg[si].fs_handle = FS_INVALID_HANDLE;
					umount(fs_handle);

					e->rs.expand = expand;
					e->rs.strg_indx = si;
					e->rs.refresh_data = NULL;
//					e->rs.setroot = selkill;

					PostMessage(e->hwndMainWindow, WM_REFRESH , 0, 0);
				}
			}
		}
        while(retry);
	}
	else
	{
		if (ListView_GetItemCount(e->hwndFilebar) > 0)
		{
			/* clear focus */
			int iFocused = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | LVNI_FOCUSED);
			if (iFocused != - 1)
				ListView_SetItemState(e->hwndFilebar, iFocused, 0, LVIS_FOCUSED | LVIS_SELECTED);

			/*  clear selection */
			int iSelected = -1;
			while ((iSelected = ListView_GetNextItem(e->hwndFilebar, iSelected, LVNI_ALL | LVNI_SELECTED)) != -1)
				ListView_SetItemState(e->hwndFilebar, iSelected, 0, LVIS_SELECTED);

			/* focus first item */
			ListView_SetItemState(e->hwndFilebar, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			ListView_EnsureVisible(e->hwndFilebar, 0, FALSE);
			UpdateWindow(e->hwndFilebar);
		}
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_internal_refresh(explorer *e, DWORD data)
{
	assert(0);
	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_copy(explorer *e, DWORD data)
{
	int selcount = ListView_GetSelectedCount(e->hwndFilebar);
	eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;

	BYTE si;
	char folder_name[MAXFILENAME];
	build_foldername(e->hwndDiskbar, e->hOpenObject, folder_name, MAXFILENAME, si);

	DWORD chkdsk = check_disk(e, si, TRUE);
	if (chkdsk == DISK_READY)
	{
		// copy
        BOOL bsingle = FALSE;
		if (selcount == 1)
		{
			int sel = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | LVNI_SELECTED);
			assert(sel != -1);
			
			DWORD fileindx = folder_data->sorted_files_order[sel];
			if (folder_data->files[fileindx].type == FT_REG)
			{
				DWORD cres;
				cres = copyfile(e, si, folder_name, folder_data->files + fileindx);
				if (cres == COPY_REFRESH) 
                    e->need_internal_refresh = si;

                bsingle = TRUE;
			}
		}

		if (selcount >= 1 && !bsingle)
		{
            DWORD *indx = new DWORD[selcount];

            DWORD numfiles = 0;
            int sel = -1;
            while ((sel = ListView_GetNextItem(e->hwndFilebar, sel, LVNI_ALL | LVNI_SELECTED)) != -1)
            {
                indx[numfiles++] = folder_data->sorted_files_order[sel];
            }

            DWORD cres = copyfiles(e, si, folder_name, folder_data->files,
                indx, numfiles);

            delete [] indx;
		}
	}
	else if (chkdsk == DISK_NEED_REFRESH) 
        e->need_internal_refresh = si;

	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_cancopy(explorer *e, DWORD data)
{
	DWORD result = 0;

	eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
	int selcount = ListView_GetSelectedCount(e->hwndFilebar);

	if (selcount == 1)
	{
		int item = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | 
			LVNI_FOCUSED | LVNI_SELECTED);
		assert(item != -1);
        WORD type = folder_data->files[folder_data->sorted_files_order[item]].type;

		if (type == FT_REG || type == FT_DIR)
            result = 1;
	}
	else if (selcount > 1)
        result = 1;

	return result;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_changed(explorer *e, DWORD data)
{
    TCHAR status[MAX_STATUS], exstatus[MAX_STATUS];
    eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;

    DWORD selcount = ListView_GetSelectedCount(e->hwndFilebar);
    
    if (selcount == 0)
    {
        // show directory info
        DWORD folder_size = 0;
        for (DWORD i = 0; i < folder_data->num_of_files; i++)
        {
            if (folder_data->files[i].type != FT_DIR)
                folder_size += folder_data->files[i].i_size;
        }

        wsprintf(status, "%s %lu", Message(MSG_OBJECTS), folder_data->num_of_files);
        wsprintf(exstatus, "%s", foldersize_to_string(folder_size));
    }
    else
    {
        // show selected files info
        DWORD files_size = 0;
        int sel = -1;

        while ((sel = ListView_GetNextItem(e->hwndFilebar, sel, LVNI_ALL | LVNI_SELECTED)) != -1)
        {
            DWORD file_indx = folder_data->sorted_files_order[sel];
            if (folder_data->files[file_indx].type != FT_DIR)
                files_size += folder_data->files[file_indx].i_size;
        }

        wsprintf(status, "%s %lu", Message(MSG_SELECTED), selcount);
        wsprintf(exstatus, "%s", foldersize_to_string(files_size));
    }

    set_explorer_status(e, status, exstatus);

    return 0; /* not used */
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_enter(explorer *e, DWORD data)
{
    int item = -1; // no item to enter
    if (data == ENTER_BY_RETURN)
    {
        if (ListView_GetSelectedCount(e->hwndFilebar) == 1)
        {
            item = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | 
                LVNI_FOCUSED | LVNI_SELECTED);
        }
    }
    else // ENTER_BY_CLICK
    {
        POINT p;
        GetCursorPos(&p);
        ScreenToClient(e->hwndFilebar, &p);

        LVHITTESTINFO hti;
        hti.pt = p;
        item = ListView_HitTest(e->hwndFilebar, &hti);
        if (hti.flags != LVHT_ONITEMICON && hti.flags != LVHT_ONITEMLABEL)
            item = -1;
    }

    // try enter the folder or view file
    if (item != -1)
    {
        eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
        common_file *file = folder_data->files + folder_data->sorted_files_order[item];

        if (file->type == FT_DIR)
        {
            HWND hwndDiskbar = e->hwndDiskbar;
            
            TVITEM tvi;
            tvi.mask = TVIF_PARAM;
            tvi.hItem = e->hOpenObject;
            TreeView_GetItem(hwndDiskbar, &tvi);
            
            NMTREEVIEW nmtv;
            nmtv.itemNew = tvi;
            BOOL bexpand = (BOOL)eo_folder_db_expand(e, (DWORD)&nmtv);
            if (bexpand)
            {
                HTREEITEM hChild = TreeView_GetChild(hwndDiskbar, e->hOpenObject);
                if (hChild)
                {
                    BOOL find = FALSE;
                    HTREEITEM hNext = hChild;
                    do
                    {
                        char folder_name[EXT2_FILENAME_LEN];
                        TVITEM tvi;
                        tvi.hItem = hNext;
                        tvi.mask = TVIF_TEXT;
                        tvi.pszText = folder_name;
                        tvi.cchTextMax = EXT2_FILENAME_LEN;
                        TreeView_GetItem(hwndDiskbar, &tvi);

                        if (!lstrcmp(folder_name, file->file_name))
                        {
                            find = TRUE;
                            break;
                        }

                        e->intchange = TRUE;
                        hNext = TreeView_GetNextSibling(hwndDiskbar, hNext);
                    } 
					while (hNext);

                    if (find)
                        TreeView_SelectItem(hwndDiskbar, hNext);
                }
            }
        }
		else if (file->type == FT_REG)
		{
			PostMessage(e->hwndMainWindow, WM_COMMAND, IDM_VIEW, 0);
		}
    }

    return 0; /* not used */
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_db_copy(explorer *e, DWORD data)
{
    char folder_name[MAXFILENAME]; 
    BYTE si;
    build_foldername(e->hwndDiskbar, e->hOpenObject, folder_name, MAXFILENAME, si);
	DWORD chkdsk = check_disk(e, si, TRUE);
	if (chkdsk == DISK_READY)
    {
        BOOL retry;
        do
        {
            retry = FALSE;
            common_file file;
            BOOL bprop = get_file_prop(e->strg[si].fs_handle, folder_name, &file);
            if (bprop)
            {
                char base_dir[MAXFILENAME];
                DWORD indx = 0;
                int  i = lstrlen(folder_name);
                while (folder_name[i] != '/' && i > 0) i--;
                if (i)
                    lstrcpyn(base_dir, folder_name, i + 1);
                else
                    lstrcpy(base_dir, "/");

                DWORD cres = copyfiles(e, si, base_dir, &file, &indx, 1);
				if (cres == COPY_REFRESH) 
                    e->need_internal_refresh = si;

            }
            else if (error_message2(e, folder_name) == IDRETRY) 
                retry = TRUE;
        }
        while (retry);

	}
	else if (chkdsk == DISK_NEED_REFRESH)
        e->need_internal_refresh = si;

	return 1;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_db_cancopy(explorer *e, DWORD data)
{
    BYTE si;
    char folder_name[MAXFILENAME]; 
    build_foldername(e->hwndDiskbar, e->hOpenObject, folder_name, MAXFILENAME, si);

    return lstrcmp(folder_name, "/");
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_canview(explorer *e, DWORD data)
{
	DWORD result = 0;
	if (GetFocus() == e->hwndFilebar)
	{
		DWORD selcount = ListView_GetSelectedCount(e->hwndFilebar);
		if (selcount == 1)
		{
            DWORD item = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL | 
                LVNI_FOCUSED | LVNI_SELECTED);
			
			eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;
			common_file *file = folder_data->files + folder_data->sorted_files_order[item];
			
			if (file->type == FT_REG)
				result = 1;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

DWORD eo_folder_fb_view(explorer *e, DWORD data)
{
	int selcount = ListView_GetSelectedCount(e->hwndFilebar);
	eo_folder_data *folder_data = (eo_folder_data *)e->fb_data;

	BYTE si;
	char folder_name[MAXFILENAME];
	build_foldername(e->hwndDiskbar, e->hOpenObject, folder_name, MAXFILENAME, si);

	DWORD chkdsk = check_disk(e, si, TRUE);
	if (chkdsk == DISK_READY)
	{
		// view
		if (selcount == 1)
		{
			int sel = ListView_GetNextItem(e->hwndFilebar, -1, LVNI_ALL |
				LVNI_FOCUSED | LVNI_SELECTED);
			assert(sel != -1);
			
			DWORD fileindx = folder_data->sorted_files_order[sel];
			if (folder_data->files[fileindx].type == FT_REG)
			{
				DWORD vres;
				vres = viewfile(e, si, folder_name, folder_data->files + fileindx);
				if (vres == VIEW_REFRESH) 
                    e->need_internal_refresh = si;
			}
		}
	}
	else if (chkdsk == DISK_NEED_REFRESH) 
        e->need_internal_refresh = si;

	return 1;
}

//////////////////////////////////////////////////////////////////////////

