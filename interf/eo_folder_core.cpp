#include <stdafx.h>
#include "eo_folder_core.h"
#include "exp_core.h"

/////////////////////////////////////////////////////////////////////////

BOOL build_foldername(HWND hwndTreeView, HTREEITEM hTreeItem, char *buf, DWORD buflen, BYTE &fi)
{
	BOOL result = TRUE;
	BOOL builded;
	TVITEM tvi;
	char lpText[255];

	tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_TEXT;
	tvi.pszText = lpText;
	tvi.cchTextMax = 255;
	tvi.hItem = hTreeItem;
	TreeView_GetItem(hwndTreeView, &tvi);

	DWORD curlen = 1; // last zero
	char *file_name = new char[buflen];
	file_name[0] = 0;
	do
	{
		WORD folder = GET_EO_DATA(tvi.lParam);
		if (builded = (GET_FOLDER_INDEX(folder) > 0))
		{
			// process root folder
			if (curlen == 1) lstrcpy(file_name, "/");
			fi = GET_FOLDER_INDEX(folder);
		}
		else
		{
			// process non-root folder
			DWORD part_len = lstrlen(tvi.pszText);
			if (part_len < buflen - curlen - 1 /*reserved for '/'*/)
			{
				MoveMemory(file_name + part_len + 1, file_name, curlen);
				file_name[0] = '/';
				MoveMemory(file_name + 1, tvi.pszText, part_len);
				curlen += part_len + 1;
			}
			else
			{
				result = FALSE;
				break;
			}
		}
		tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_TEXT;
		tvi.pszText = lpText;
		tvi.cchTextMax = 255;
		tvi.hItem = TreeView_GetParent(hwndTreeView, tvi.hItem);
		if (!tvi.hItem) // meet tree root 
		{
			result = FALSE;
			break;
		}

		TreeView_GetItem(hwndTreeView, &tvi);
	} while(!builded);
	
	if (result) lstrcpy(buf, file_name);
	delete [] file_name;
	return result;
}

//////////////////////////////////////////////////////////////////////////


BOOL supermount(explorer *e, TVITEM *tvitem)
{
	BOOL result = TRUE; // 'software' folders are always mounted
	WORD folder = GET_EO_DATA(tvitem->lParam);

	if (!(folder & EO_FOLDER_MOUNTED))
	{
		// it's 'hardware'  folder, try to mount
		BYTE fi = GET_FOLDER_INDEX(folder);
		assert(fi);

		BOOL retry;
		do
		{
			retry = FALSE;
			explorer_busy(e, TRUE);
			e->strg[fi].fs_handle = mount(e->strg[fi].storage_name);
			explorer_busy(e, FALSE);
			
			if (e->strg[fi].fs_handle != FS_INVALID_HANDLE)
			{
				DWORD lParam = tvitem->lParam;
				DWORD lNewParam = MAKE_EO_ATTR(GET_EO_DATA(lParam) | EO_FOLDER_MOUNTED,
					GET_EO_TYPE(lParam));
				tvitem->lParam = lNewParam;
				
				TVITEM tvi;
				tvi.mask = TVIF_PARAM | TVIF_HANDLE;
				tvi.lParam = lNewParam;
				tvi.hItem = tvitem->hItem;
				TreeView_SetItem(e->hwndDiskbar, &tvi);
			}
			else
			{
				if (error_message2(e, e->strg[fi].common_name) == IDRETRY) 
                    retry = TRUE;
				else
                    result = FALSE;
			}
		}
        while (retry);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
