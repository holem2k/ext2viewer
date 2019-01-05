#include "stdafx.h"
#include "props.h"
#include "mi_propdlg.h"
#include "file_propdlg.h"
#include "files_propdlg.h"

//////////////////////////////////////////////////////////////////

void show_file_props(explorer *e, common_file *file, char *exinfo, char *exinfotext)
{
	create_file_propdlg(e, file, exinfo, exinfotext);
}

//////////////////////////////////////////////////////////////////

void show_mount_props(explorer *e, const char *disk_name,
					  mount_info *mi, common_file *file)
{
	create_mi_propdlg(e, disk_name, mi, file);
}

//////////////////////////////////////////////////////////////////

void show_files_props(explorer *e, common_file *files,
					  DWORD *files_indx, DWORD num_of_files)
{
	create_files_propdlg(e, files, files_indx, num_of_files);
}

//////////////////////////////////////////////////////////////////
