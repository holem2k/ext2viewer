#ifndef __EO_FOLDER_H__
#define __EO_FOLDER_H__
#include "explorer.h"
#include "ext2_fs.h"


typedef struct tag_eo_folder_data
{
	common_file *files;
	DWORD *sorted_files_order;
	DWORD num_of_files;
} eo_folder_data;

const DWORD LINK_LEN = 32768;
const DWORD PROP_LEN = 1024;

typedef enum tag_ENTER_TYPE
{
    ENTER_BY_CLICK, ENTER_BY_RETURN
} ENTER_TYPE;

DWORD eo_folder_db_open(explorer *e, DWORD data);
DWORD eo_folder_db_expand(explorer *e, DWORD data);
DWORD eo_folder_db_openexpand(explorer *e, DWORD data);
DWORD eo_folder_db_props(explorer *e, DWORD data);
DWORD eo_folder_fb_callback(explorer *e, DWORD data);
DWORD eo_folder_fb_cleanup(explorer *e, DWORD data);
DWORD eo_folder_fb_getitemcount(explorer *e, DWORD data);
DWORD eo_folder_fb_preparecolumns(explorer *e, DWORD data);
DWORD eo_folder_fb_props(explorer *e, DWORD data);
DWORD eo_folder_refresh(explorer *e, DWORD data);
DWORD eo_folder_internal_refresh(explorer *e ,DWORD data);
DWORD eo_folder_fb_copy(explorer *e, DWORD data);
DWORD eo_folder_db_copy(explorer *e, DWORD data);
DWORD eo_folder_fb_cancopy(explorer *e, DWORD data);
DWORD eo_folder_db_cancopy(explorer *e, DWORD data);
DWORD eo_folder_fb_changed(explorer *e, DWORD data);
DWORD eo_folder_fb_enter(explorer *e, DWORD data);
DWORD eo_folder_fb_canview(explorer *e, DWORD data);
DWORD eo_folder_fb_view(explorer *e, DWORD data);

void sort_fb(eo_folder_data *folder_data, FB_SORT_ORDER fb_sort_order);

#endif //__EO_FOLDER_H__