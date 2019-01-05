#ifndef __EO_FOLDER_HLP_H__
#define __EO_FOLDER_HLP_H__
#include "ext2_fs.h"
#include "time.h"

DWORD get_icon_from_file(common_file *file);
char *get_type_from_file(common_file *file);
char *get_size_from_file(common_file *file);
char *get_attr_from_file(common_file *file);

char *time_to_string(time_t t);
char *group_to_string(DWORD user);
char *user_to_string(DWORD group);

char *get_suid_from_file(common_file *file);
char *get_sgid_from_file(common_file *file);
char *get_fs_state(mount_info *mi);
char *size_to_string(DWORD size);
char *size_to_spaced_string(DWORD size);

TCHAR *foldersize_to_string(DWORD size);
void time_to_filetime(time_t t, LPFILETIME pft);

#endif //__EO_FOLDER_HLP_H__