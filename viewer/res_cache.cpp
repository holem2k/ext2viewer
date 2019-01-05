#include "stdafx.h"
#include "res_cache.h"

//////////////////////////////////////////////////////////////////////

void init_res_cache(res_cache *rc, device *dev)
{
	rc->dev = dev;
	rc->free = 0;
	for (int i = 0; i < ENTRY_NUM; i++)	rc->entry[i].empty = TRUE;
}

//////////////////////////////////////////////////////////////////////

void put_into_res_cache(res_cache *rc, char *name, DWORD inode)
{
	if (lstrlen(name) < RES_CACHE_ENTRY_LEN)
	{
		int free = rc->free;
		rc->entry[free].empty = FALSE;
		rc->entry[free].inode = inode;
		lstrcpy(rc->entry[free].name, name);
		rc->free = (free + 1)%ENTRY_NUM;
	}
}

//////////////////////////////////////////////////////////////////////

BOOL get_from_res_cache(res_cache *rc, char *name, DWORD &inode/*out*/)
{
	//  TEMPORARY CODE FOR REMOVABLE DEVICES
	if (rc->dev->is_removable) return FALSE;

	BOOL result = FALSE;
	for (int i = 0; i < ENTRY_NUM; i++)
	{
		if (!rc->entry[i].empty)
		{
			if (!lstrcmp(name, rc->entry[i].name))
			{
				inode = rc->entry[i].inode;
				result =  TRUE;
				break;
			}
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////


