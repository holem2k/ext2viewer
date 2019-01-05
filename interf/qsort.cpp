#include "stdafx.h"
#include "qsort.h"
#include "stdlib.h"

static common_file *g_files; //:(

/////////////////////////////////////////////////////////////////////

void sort(common_file *files, DWORD *sorted_order, DWORD num_of_files, SORT_ORDER sort_order)
{
	g_files = files;
	int ( *sort_func)(const void *elem1, const void *elem2); 
	switch(sort_order)
	{
	case SORT_BY_NAME:
		sort_func = sort_by_name;
		break;
	case SORT_BY_SIZE:
		sort_func = sort_by_size;
		break;
	case SORT_BY_TYPE:
		sort_func = sort_by_type;
		break;
	case SORT_BY_DATE:
		sort_func = sort_by_date;
		break;
	case SORT_BY_NONE:
		sort_func = sort_by_none;
		break;
	default:
		assert(0);
	}
	qsort(sorted_order, num_of_files, sizeof(DWORD), sort_func);
}

/////////////////////////////////////////////////////////////////////

int __cdecl sort_by_name(const void *elem1, const void *elem2)
{
	DWORD n1 = *((DWORD *)elem1);
	common_file *file1 = g_files + n1;
	DWORD n2 = *((DWORD *)elem2);
	common_file *file2 = g_files + n2;

	int c1;
	if (file1->type == FT_DIR)
        c1 = file2->type == FT_DIR ? 0 : -1;
	else
        c1 = file2->type == FT_DIR ? 1 : 0;

	int c2 = lstrcmp(file1->file_name, file2->file_name);

	return c1 ? c1 : c2;
}

/////////////////////////////////////////////////////////////////////

int __cdecl sort_by_size(const void *elem1, const void *elem2)
{
	DWORD n1 = *((DWORD *)elem1);
	common_file *file1 = g_files + n1;
	DWORD n2 = *((DWORD *)elem2);
	common_file *file2 = g_files + n2;

	int c1;
	if (file1->type == FT_DIR)
        c1 = file2->type == FT_DIR ? 0 : -1;
	else
        c1 = file2->type == FT_DIR ? 1 : 0;

	int c2 = file1->i_size - file2->i_size;

	return c1 ? c1 : c2;
}

/////////////////////////////////////////////////////////////////////

int __cdecl sort_by_type(const void *elem1, const void *elem2)
{
	DWORD n1 = *((DWORD *)elem1);
	common_file *file1 = g_files + n1;
	DWORD n2 = *((DWORD *)elem2);
	common_file *file2 = g_files + n2;

	int c1, c2;
	c1 = file2->type - file1->type;
	if (!c1)
		c2 = lstrcmp(file1->file_name, file2->file_name);

	return c1 ? c1 : c2;
}

/////////////////////////////////////////////////////////////////////

int __cdecl sort_by_date(const void *elem1, const void *elem2)
{
	DWORD n1 = *((DWORD *)elem1);
	common_file *file1 = g_files + n1;
	DWORD n2 = *((DWORD *)elem2);
	common_file *file2 = g_files + n2;

	int c1;
	if (file1->type == FT_DIR)	
        c1 = file2->type == FT_DIR ? 0 : -1;
	else
        c1 = file2->type == FT_DIR ? 1 : 0;

	int c2 = file1->i_mtime - file2->i_mtime;

	return c1 ? c1 : c2;
}

/////////////////////////////////////////////////////////////////////

int __cdecl sort_by_none(const void *elem1, const void *elem2)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////
