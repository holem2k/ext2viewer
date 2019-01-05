#include <stdafx.h>
#include "icons.h"


HWND CreateFileBar(HWND hwndParent, DWORD dwLeftOffset, DWORD dwTopOffset)
{
	RECT rect;
	GetClientRect(hwndParent, &rect);
	HWND hwndListView = CreateWindowEx(WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN |  
		WS_CLIPSIBLINGS | WS_HSCROLL | WS_TABSTOP |
		LVS_REPORT| LVS_OWNERDATA, 
        dwLeftOffset, dwTopOffset + 3,
		rect.right - rect.left - dwLeftOffset,
		rect.bottom - rect.top - dwTopOffset - 3, 
        hwndParent,
		(HMENU)ID_FILEBAR_LV,
		(HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE),
		NULL); 

	HIMAGELIST himlIcons = CreateIconList(FilebarIcons, FilebarIconsNum, TRUE);
	HIMAGELIST himlIconsSmall = CreateIconList(FilebarIconsSmall, FilebarIconsNum);

	ListView_SetImageList(hwndListView, himlIcons, LVSIL_NORMAL);
	ListView_SetImageList(hwndListView, himlIconsSmall, LVSIL_SMALL);
	ListView_SetItemCount(hwndListView, 0);

	return hwndListView;
}