#ifndef __EXP_CORE_H__
#define __EXP_CORE_H__
#include "explorer.h"

#define MAKE_EO_ATTR(data, type) MAKELONG(type, data)
#define GET_EO_DATA(eo_attr) HIWORD(eo_attr)
#define GET_EO_TYPE(eo_attr) ((EO_ID)LOWORD(eo_attr))

// EO_FOLDER special
const WORD EO_FOLDER_MOUNTED  = 0x8000;
const WORD EO_FOLDER_EXPANDED = 0x4000;

inline void SET_FOLDER_INDEX(WORD &folder, BYTE index)
{
	folder = MAKEWORD(index, HIBYTE(folder));
}
inline BYTE GET_FOLDER_INDEX(WORD folder)
{
	return LOBYTE(folder);
}


const DWORD DISK_NOT_READY = 0;
const DWORD DISK_READY = 1;
const DWORD DISK_NEED_REFRESH = 2;

void init(explorer *e);
BOOL build_storage(explorer *e);
void explorer_init_obj(explorer *e);
DWORD run_e_op(explorer *e, EO_ID eo_id, EO_OP op_id, DWORD data);

void set_explorer_caption(explorer *e, LPCTSTR lpCaption);
void set_explorer_status(explorer *e, LPCTSTR lpStatus1, LPCTSTR lpStatus2);

void set_fb_style(explorer *e, FB_STYLE style);
void explorer_busy(explorer *e, BOOL busy);
void set_explorer_options(explorer *e);
void set_fbsort_order(explorer *e, FB_SORT_ORDER so);
DWORD show_properties(explorer *e, BOOL checkonly);
void cleanup_explorer(explorer *e);
int error_message(explorer *e, const char *folder_name);
int error_message2(explorer *e, const char *folder_name);
int error_message3(explorer *e, const char *folder_name);
int error_message4(explorer *e, const char *folder_name);
int error_message5(explorer *e, const char *folder_name);
int error_message6(explorer *e);

void collapse_storage(explorer *e, BYTE storage);

int refresh_by_click(explorer *e);
void refresh_by_return(explorer *e);
void refresh_removable(explorer *e);

//void internal_refresh(explorer *e, int storage);
DWORD check_disk(explorer *e, BYTE fi, BOOL ask_remount);

DWORD explorer_copy(explorer *e, BOOL onlycheck);
DWORD explorer_view(explorer *e, BOOL onlycheck);

void explorer_about(explorer *e);
void context_menu(explorer *e);
void toggle_status(explorer *expl);

#endif //__EXP_CORE_H__