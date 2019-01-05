#ifndef __QSORT_H__
#define __QSORT_H__
#include "ext2_fs.h"

enum SORT_ORDER {SORT_BY_NAME, SORT_BY_SIZE, SORT_BY_TYPE, SORT_BY_DATE, SORT_BY_NONE};

void sort(common_file *files, DWORD *sorted_order, DWORD num_of_files, SORT_ORDER sort_order);

int __cdecl sort_by_name(const void *elem1, const void *elem2);
int __cdecl sort_by_size(const void *elem1, const void *elem2);
int __cdecl sort_by_type(const void *elem1, const void *elem2);
int __cdecl sort_by_date(const void *elem1, const void *elem2);
int __cdecl sort_by_none(const void *elem1, const void *elem2);

#endif //__QSORT_H__