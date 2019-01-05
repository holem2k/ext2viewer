#ifndef __DEBUG_H__
#define __DEBUG_H__

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


#endif //__DEBUG_H__