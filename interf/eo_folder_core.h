#ifndef __EO_FOLDER_CORE_H__
#define __EO_FOLDER_CORE_H__
#include "explorer.h"

BOOL supermount(explorer *e, TVITEM *tvitem);
BOOL build_foldername(HWND hwndTreeView, HTREEITEM hTreeItem, char *buf, DWORD buflen, BYTE &fi);

#endif //__EO_FOLDER_CORE_H__