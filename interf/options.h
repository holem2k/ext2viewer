#ifndef __OPTIONS_H__
#define __OPTIONS_H__
#include "explorer.h"

typedef struct tag_options
{
	DWORD fb_columns[COL_MAX];
} options;

DWORD CreateOptionsDialog(HWND hwndParent, options *opt);

#endif //__OPTIONS_H__