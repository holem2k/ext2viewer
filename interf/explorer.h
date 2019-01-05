#ifndef __EXPLORER_H__
#define __EXPLORER_H__
#include "ext2_fs.h"
#include "time.h"

const DWORD WM_INTERNAL_REFRESH = WM_USER + 100;
const DWORD WM_REFRESH =  WM_USER + 101;

/* стили  файлбара */
enum FB_STYLE {FB_STYLE_ICON, FB_STYLE_SMALLICON, FB_STYLE_LIST, FB_STYLE_REPORT};

/* команды объектов exплорера*/
enum EO_OP {OP_DB_OPEN, OP_DB_EXPAND, OP_FB_CALLBACK,
            OP_FB_CLEANUP, OP_FB_GETITEMCOUNT, OP_FB_PREPARECOLUMNS,
			OP_DB_PROPS, OP_FB_PROPS, OP_REFRESH,
			OP_SPEC1, OP_DB_OPENEXPAND, OP_FB_COPY,
			OP_DB_COPY, OP_FB_CANCOPY, OP_DB_CANCOPY,
            OP_FB_CHANGED, OP_FB_ENTER, 
			OP_FB_CANVIEW, OP_FB_VIEW, MAX_E_OP};
/*
==========================================
OP_DB_OPEN - вызывается при выборе объекта в дискбаре
установливает соответствующее число столбцов в файлбаре,
Возвращает новое значение fb_data или NULL(в этом случае состояние explorera
должно оставаться прежним)
Параметр  data - указатель на NMTREEVIEW??(TVITEM)
==========================================
OP_DB_EXPAND - вызывается при раскрытии объекта
Возвращает TRUE, если объект раскрыт, иначе FALSE
Параметр data - указатель на TVITEM (must be valid: lParam, hItem)
==========================================
OP_DB_OPENEXPAND - используется только при рефреше,
когда надо одновременно и открыть каталог ипоказать его подкаталоги.
*/

enum EO_ID {EO_COMPUTER_ID, EO_FOLDER_ID, EO_HDD, EO_FILE_ID, EO_NONE_ID};

/* команды управления експлорером */
enum ECTRL_OP {E_GETFBSTYLE, E_SETFBSTYLE, E_INIT,
               E_OPTIONS, E_GETFBSORTORDER, E_SETFBSORTORDER,
			   E_SOMEFOCUSED, E_SHOWPROPERTY, E_REFRESH_BY_CLICK,
			   E_REFRESH_BY_RETURN, E_REFRESH_REMOVABLE, E_INTERNAL_REFRESH,
			   E_CLEANUP, E_COPY, E_ABOUT,
			   E_CNTX_MENU, E_GETISSB, E_TOGGLESB,
			   E_VIEW
};

typedef struct tag_e_object
{
	void *op[MAX_E_OP];
} e_object;

typedef struct tag_storage
{
	FilebarIconsEnum icon;
	FS_HANDLE fs_handle;
	char storage_name[256];
	char common_name[256];
	HTREEITEM hTreeItem;
	time_t atime;
} storage;

typedef struct  tag_refresh_struct
{
	BYTE strg_indx;
	BOOL expand;
	//BOOL setroot;
	void *refresh_data;
	//HWND hwndFocused;
	HTREEITEM hFailureItem;
} refresh_struct;

const DWORD MAX_STORAGE_NUM = 256;
const DWORD MAX_E_OBJECT = 16;

enum FB_COLUMNS {COL_NAME, COL_TYPE, COL_SIZE, COL_RIGHTS, COL_CTIME, COL_MTIME, COL_ATIME, COL_USER, COL_GROUP, COL_MAX};
enum FB_SORT_ORDER {FB_SORT_BY_NAME, FB_SORT_BY_TYPE, FB_SORT_BY_SIZE, FB_SORT_BY_DATE, FB_SORT_BY_NONE};

typedef struct tag_explorer
{
	HWND hwndFilebar;
	HWND hwndDiskbar;
	HWND hwndMainWindow;
    HWND hwndStatus;

	storage strg[MAX_STORAGE_NUM];
	int num_strg;

	FB_STYLE fb_style;
	FB_SORT_ORDER fb_sort_order;
    BOOL bstatus;

	e_object eo[MAX_E_OBJECT];
	EO_ID current_eo_fb; // тип объекта, содержимое которого отбражается в Filebar
	DWORD fb_columns[COL_MAX];

	void *fb_data;
	HTREEITEM hOpenObject;

	int need_internal_refresh; // storage to refresh (-1 - nothing)
	refresh_struct rs;

    BOOL intchange;
} explorer;

typedef DWORD( *EXPLOREROP)(void *explorer, DWORD data);

explorer *create_explorer(HWND hwndDiskbar, HWND hwndFilebar, HWND hwndStatus,  HWND hwndMainWindow);
BOOL register_explorer_obj(explorer *e, EO_ID eo_id, e_object &eo);
void destroy_explorer(explorer *e);
BOOL dispatch_explorer(explorer *e, EO_OP opcode, DWORD data);
DWORD control_explorer(explorer *e, ECTRL_OP opcode, DWORD data);

#endif //__EXPLORER_H__