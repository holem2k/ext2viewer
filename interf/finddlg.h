#ifndef __FINDDLG_H__
#define __FINDDLG_H__
#include  "explorer.h"

const DWORD FF_CASESENSETIVE = 0x01;
const DWORD FF_WHOLEWORDS    = 0x02;

const DWORD MAX_FINDTEXT = 1024;

typedef struct tag_FIND
{
	char *buf;
	DWORD buflen;
	DWORD flags;
} FIND;

BOOL create_finddlg(HWND hParent, FIND *find);

#endif //__FINDDLG_H__