#ifndef __MI_PROPDLG_H__
#define __MI_PROPDLG_H__
#include "ext2_fs.h"
#include "explorer.h"

void create_mi_propdlg(explorer *e, const char *disk_name, mount_info *mi, common_file *file);

#endif //__MI_PROPDLG_H__