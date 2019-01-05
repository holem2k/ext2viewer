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



const DWORD ENTRY_NUM = 400;

typedef struct tag_res_cache
{
	res_cache_entry entry[ENTRY_NUM];
	int free;
	device *dev;
} res_cache;


void rcache_init(res_cache *rc, device *dev);
void rcache_put(res_cache *rc, char *name, DWORD inode);
BOOL rcache_lookup(res_cache *rc, char *name, DWORD &inode/*out*/);

#endif //__RES_CACHE_H__