#ifndef __VIEW_H__
#define __VIEW_H__
#include "explorer.h"
#include "ext2_fs.h"

const DWORD VIEW_OK      = 0x00;
const DWORD VIEW_FAIL    = 0x01;
const DWORD VIEW_REFRESH = 0x02;

DWORD viewfile(explorer *e, BYTE si, const char *folder_name, common_file *file);

#endif //__VIEW_H__