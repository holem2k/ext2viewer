#ifndef __RES_CACHE_H__
#define __RES_CACHE_H__
#include "dev.h"

const DWORD RES_CACHE_ENTRY_LEN = 2048;

typedef struct tag_res_cache_entry
{
	char name[RES_CACHE_ENTRY_LEN];
	DWORD inode;
	BOOL empty;
} res_cache_entry;



const DWORD ENTRY_NUM = 4;

typedef struct tag_res_cache
{
	res_cache_entry entry[ENTRY_NUM];
	int free;
	device *dev;
} res_cache;


void init_res_cache(res_cache *rc, device *dev);
void put_into_res_cache(res_cache *rc, char *name, DWORD inode);
BOOL get_from_res_cache(res_cache *rc, char *name, DWORD &inode/*out*/);

#endif //__RES_CACHE_H__