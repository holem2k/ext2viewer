#include "stdafx.h"
#include "bcache.h"

///////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#ifdef _CACHESTAT
static clookup_num = 0;
static chit_num = 0;
#endif
#endif

void bcache_init(bcache *cache, device *dev)
{
	for (int i = 0; i < BCACHE_ACC; i++)
	{
		for (int j = 0; j < BCACHE_LINELEN; j++)
		{
			cache->cache[i][j].block_num = 0;
			cache->cache[i][j].block = NULL;
		}
		cache->next_free[i] = 0;
	}
	cache->dev = dev;
}

///////////////////////////////////////////////////////////////////////////

void bcache_done(bcache *cache)
{
	for (int i = 0; i < BCACHE_ACC; i++)
		for (int j = 0; j < BCACHE_LINELEN; j++)
		{
			delete [] cache->cache[i][j].block;
			cache->cache[i][j].block = NULL;
		}
}

///////////////////////////////////////////////////////////////////////////

BYTE *bcache_lookup(bcache *cache, DWORD block_num)
{
	// CODE FOR REMOVABLE DEVICE
	//if (cache->dev->is_removable) return NULL;
	// CODE FOR REMOVABLE DEVICE

	BYTE *cache_data = NULL;
	DWORD cline_num = block_num%BCACHE_ACC;

	for (int i = 0; i < BCACHE_LINELEN; i++)
	{
		if (cache->cache[cline_num][i].block_num == block_num
			&& cache->cache[cline_num][i].block != NULL)
		{
			cache_data = cache->cache[cline_num][i].block;
			break;
		}
	}


#ifdef _DEBUG
#ifdef _CACHESTAT
		clookup_num++;
		if (cache_data) chit_num++;
		char s[256];
		if (chit_num)
		{
			wsprintf(s, "%u\n", chit_num*100/clookup_num);
			OutputDebugString(s);
		}
#endif
#endif
	return cache_data;
}

///////////////////////////////////////////////////////////////////////////

void bcache_put(bcache *cache, DWORD block_num, BYTE *block)
{
	// CODE FOR REMOVABLE DEVICE
	//if (cache->dev->is_removable) return;
	// CODE FOR REMOVABLE DEVICE

	DWORD cline_num = block_num%BCACHE_ACC;
	DWORD free = cache->next_free[cline_num];

	delete [] cache->cache[cline_num][free].block;
	cache->cache[cline_num][free].block = block;
	cache->cache[cline_num][free].block_num = block_num;

	cache->next_free[cline_num] = (cache->next_free[cline_num] + 1)%BCACHE_LINELEN;
}

///////////////////////////////////////////////////////////////////////////