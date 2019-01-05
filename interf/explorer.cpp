#include <assert.h>
#include "stdafx.h"
#include "explorer.h"
#include "exp_core.h"
#include "eo_comp.h"

/////////////////////////////////////////////////////////////////////////

explorer *create_explorer(HWND hwndDiskbar, HWND hwndFilebar, HWND hwndStatus,  HWND hwndMainWindow)
{
	explorer *expl = new explorer;
	explorer_init_obj(expl);

	expl->hwndDiskbar = hwndDiskbar;
	expl->hwndFilebar = hwndFilebar;
	expl->hwndMainWindow = hwndMainWindow;
	expl->hwndStatus = hwndStatus;

	expl->current_eo_fb = EO_NONE_ID;
	expl->fb_data = NULL;
	expl->need_internal_refresh = -1;

	expl->rs.refresh_data = NULL;
	expl->rs.strg_indx = 0;

    expl->intchange = FALSE;
	
	if (!build_storage(expl))
	{
		delete expl;
		expl = NULL;
	}

	return expl;
}

/////////////////////////////////////////////////////////////////////////

BOOL register_explorer_obj(explorer *e, EO_ID eo_id, e_object &eo)
{
	assert(e);
	e->eo[eo_id] = eo;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////

BOOL dispatch_explorer(explorer *e, EO_OP opcode, DWORD data)
{
	assert(e);
#ifdef _DEBUG
	static int c = 0;
	c++;
	char s[20];
	wsprintf(s, "%u %u\n", c, opcode);
//	OutputDebugString(s);
//	//if (c == 2) assert(0);
#endif

	BOOL result = FALSE;
	switch (opcode)
	{
	case OP_DB_OPENEXPAND:
	case OP_DB_OPEN:
		{
			TVITEM *ptvi = (TVITEM *)data;
			if (!ptvi->hItem)
                return FALSE;

			EO_ID eo_id = GET_EO_TYPE(ptvi->lParam);
			assert(eo_id < EO_NONE_ID);

			DWORD new_fb_data;

			if (e->rs.refresh_data && eo_id == EO_FOLDER_ID)
			{
				new_fb_data = (DWORD)e->rs.refresh_data;
				e->rs.refresh_data = NULL;
			}
			else
                new_fb_data = run_e_op(e, eo_id, opcode/*OP_DB_OPEN*/, data);

			if (new_fb_data)
			{
				/* cleanup previos object's content in filebar */
				run_e_op(e, e->current_eo_fb, OP_FB_CLEANUP, 0);

				/* set current data pointer */
				e->fb_data = (void *)new_fb_data;
				e->current_eo_fb = eo_id;

				/* prepare filebar colums for new object */
				run_e_op(e, e->current_eo_fb, OP_FB_PREPARECOLUMNS, 0);

				/* set number of items in filebar*/
				DWORD item_count = run_e_op(e, e->current_eo_fb, OP_FB_GETITEMCOUNT, 0);
				ListView_SetItemCount(e->hwndFilebar, item_count);

				/* set currently opened object */
				e->hOpenObject = ptvi->hItem;

				/* set scroller start position and focus first item*/
				if (item_count)
				{
					int sel = -1;
					while ((sel = ListView_GetNextItem(e->hwndFilebar, sel, LVNI_ALL | LVNI_SELECTED)) != -1)
						ListView_SetItemState(e->hwndFilebar, sel, 0, LVIS_SELECTED);

                    ListView_SetItemState(e->hwndFilebar, 0, LVIS_FOCUSED, LVIS_FOCUSED | LVIS_SELECTED);
					ListView_EnsureVisible(e->hwndFilebar, 0, FALSE);
				}

                // show new status
                run_e_op(e, e->current_eo_fb, OP_FB_CHANGED, 0);
			}
			else
			{
				result = TRUE; /* prevent to selection to change */
			}
		}
		break;
	case OP_DB_EXPAND:
		{
			NMTREEVIEW *pnmtv = (NMTREEVIEW *)data;
			EO_ID eo_id = GET_EO_TYPE(pnmtv->itemNew.lParam);
			assert(eo_id < EO_NONE_ID);
			result = !run_e_op(e, eo_id, OP_DB_EXPAND, data);
		}
		break;

	case OP_FB_CALLBACK:
			result = run_e_op(e, e->current_eo_fb, OP_FB_CALLBACK, data);
		break;

	case OP_REFRESH:
		{
			TVITEM tvi;
			tvi.mask = TVIF_PARAM;
			tvi.hItem = e->hOpenObject;
			TreeView_GetItem(e->hwndDiskbar, &tvi);
			EO_ID eo_id = GET_EO_TYPE(tvi.lParam);
			assert(eo_id < EO_NONE_ID);
			result = run_e_op(e, eo_id, OP_REFRESH, (DWORD)e->hOpenObject);
		}
		break;

	case OP_SPEC1:
		result = run_e_op(e, EO_FOLDER_ID, OP_SPEC1, (DWORD)e->need_internal_refresh);
		break;

    case OP_FB_CHANGED:
        result = run_e_op(e, e->current_eo_fb, OP_FB_CHANGED, 0);
        break;

    case OP_FB_ENTER:
        result = run_e_op(e, e->current_eo_fb, OP_FB_ENTER, data);
        break;
	}

	if (e->need_internal_refresh != -1)
	{
		BYTE si = e->need_internal_refresh;
		e->need_internal_refresh = -1;
		PostMessage(e->hwndMainWindow, WM_INTERNAL_REFRESH, (WPARAM)si, 0);
	}

#ifdef _DEBUG
	c--;
#endif

	return result;
}

/////////////////////////////////////////////////////////////////////////

void destroy_explorer(explorer *e)
{
	assert(e);
	delete e;
}

/////////////////////////////////////////////////////////////////////////

DWORD control_explorer(explorer *e, ECTRL_OP opcode, DWORD data)
{
	assert(e);

	DWORD result = 0;
	switch (opcode)
	{
	case E_SETFBSTYLE:
		set_fb_style(e, (FB_STYLE)data);
		result = TRUE;
		break;

	case E_GETFBSTYLE:
		result =  e->fb_style;
		break;

	case E_INIT:
		init(e);
		result = 1;
		break;

	case E_OPTIONS:
		set_explorer_options(e);
		result = 1;
		break;

	case E_GETFBSORTORDER:
		result = e->fb_sort_order;
		break;

	case E_SETFBSORTORDER:
		set_fbsort_order(e, (FB_SORT_ORDER)data);
		result = 1;
		break;

	case E_SOMEFOCUSED:
		result = show_properties(e, TRUE);
		break;

	case E_SHOWPROPERTY:
		show_properties(e, FALSE);
		result = 1;
		break;

	case E_CLEANUP:
		cleanup_explorer(e);
		break;

	case E_REFRESH_BY_CLICK:
		result = refresh_by_click(e);
		break;

	case E_REFRESH_BY_RETURN:
		refresh_by_return(e);
		result = 1;
		break;

	case E_REFRESH_REMOVABLE:
		refresh_removable(e); /* go stage 2 */
		result = 1;
		break;

	case E_INTERNAL_REFRESH: /* go stage 1 */
		run_e_op(e, EO_FOLDER_ID, OP_REFRESH, (DWORD)e->strg[(BYTE)data].hTreeItem);
		break;

	// ARCH PROBLEM
	case E_COPY:
		result = explorer_copy(e, (BOOL)data);
		break;

	// ARCH PROBLEM
	case E_VIEW:
		result = explorer_view(e, (BOOL)data);
		break;

	case E_ABOUT:
		explorer_about(e);
		result = 1;
		break;

	case E_CNTX_MENU:
		context_menu(e);
		result = 1; /* allow default processing */
		break;

    case E_GETISSB:
        result = e->bstatus;
        break;

    case E_TOGGLESB:
        toggle_status(e);
        break;
	}

	if (e->need_internal_refresh != -1)
	{
		BYTE si = e->need_internal_refresh;
		e->need_internal_refresh = -1;
		PostMessage(e->hwndMainWindow, WM_INTERNAL_REFRESH, (WPARAM)si, 0);
	}

	return result;
}
