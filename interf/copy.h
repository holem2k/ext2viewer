#ifndef __COPY_H__
#define __COPY_H__
#include "explorer.h"
#include "ext2_fs.h"

const DWORD COPY_OK      = 0x00;
const DWORD COPY_FAIL    = 0x01;
const DWORD COPY_REFRESH = 0x02;


DWORD copyfile(explorer *e, BYTE si, char *basedir, common_file *file);
DWORD copyfiles(explorer *e, BYTE si, char *basedir, common_file *files,
                DWORD *indx, DWORD numfiles);


#endif //__COPY_H__