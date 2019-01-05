#ifndef __TRACE_H__
#define __TRACE_H__
#include  <stdio.h>

#ifdef _DEBUG
#define TR(A) _TR(A)
#define ASSERT(A) _ASSERT(A)
#else
#define TR(A)
#define ASSERT(A)
#endif

#ifdef _DEBUG

void _TR(char *s);
void _TR(DWORD dwNum);
void ASSERT(BOOL bExpr);

#endif

//#define LOG(s) \
//	{ \
//		FILE *file = fopen("debug.log", "a+"); \
//		fprintf##s; \
//		fclose(file); \
//	}

#define LOG(s)


#endif //__TRACE_H__