#ifndef __PROPS_H__
#define __PROPS_H__
#include "ext2_fs.h"
#include "explorer.h"

void show_file_props(explorer *e, common_file *file, char *exinfo, char *exinfotext);
void show_mount_props(explorer *e, const char *disk_name,
					  mount_info *mi, common_file *file);
void show_files_props(explorer *e, common_file *files,
					  DWORD *files_indx, DWORD num_of_files);

#endif //__PROPS_H__