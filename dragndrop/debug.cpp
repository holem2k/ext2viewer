#include "stdafx.h"
#include <assert.h>

#ifdef _DEBUG

void _TR(char *s)
{
    char ds[1024];
    wsprintf(ds, "Debug - %s\n", s);
    OutputDebugString(ds);
}

void _TR(DWORD dwNum)
{
    char ds[1024];
    wsprintf(ds, "Debug - 0x%X\n", dwNum);
    OutputDebugString(ds);
}

void _ASSERT(BOOL bExpr)
{
    assert(bExpr);
}

#endif