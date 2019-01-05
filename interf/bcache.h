#ifndef __BCACHE_H__
#define __BCACHE_H__
#include "dev.h"
//#define _CACHESTAT

const DWORD BCACHE_LINELEN = 16;
const DWORD BCACHE_ACC = 32;

typedef struct tag_bcache_entry 
{
	DWORD block_num;
	BYTE *block;
} bcache_entry;

typedef struct tag_bcache
{
	bcache_entry cache[BCACHE_ACC][BCACHE_LINELEN];
	int next_free[BCACHE_ACC];
	device *dev;
} bcache;


void bcache_init(bcache *cache, device *dev);
void bcache_done(bcache *cache);
BYTE *bcache_lookup(bcache *cache, DWORD block_num);
void bcache_put(bcache *cache, DWORD block_num, BYTE *block);

#endif //__BCACHE_H__