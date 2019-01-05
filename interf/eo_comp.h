#ifndef __EO_COMP_H__
#define __EO_COMP_H__
#include "explorer.h"

typedef struct tag_eo_comp_data
{
	DWORD num_of_strg; 
} eo_comp_data;


DWORD eo_comp_db_open(explorer *e, DWORD data);
DWORD eo_comp_db_expand(explorer *e, DWORD data);
DWORD eo_comp_db_openexpand(explorer *e, DWORD data);
DWORD eo_comp_db_props(explorer *e, DWORD data);
DWORD eo_comp_fb_callback(explorer *e, LPARAM lParam);
DWORD eo_comp_fb_cleanup(explorer *e, DWORD data);
DWORD eo_comp_fb_getitemcount(explorer *e, DWORD data);
DWORD eo_comp_fb_preparecolumns(explorer *e, DWORD data);
DWORD eo_comp_fb_props(explorer *e, DWORD data);
DWORD eo_comp_refresh(explorer *e, DWORD data);
DWORD eo_comp_fb_cancopy(explorer *e, DWORD data);
DWORD eo_comp_fb_changed(explorer *e, DWORD data);

#endif //__EO_COMP_H__