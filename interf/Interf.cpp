// interf.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "menu.h"
#include "diskbar.h"
#include "filebar.h"
#include "splitter.h"
#include "explorer.h"
#include "eo_comp.h"
#include "eo_folder.h"
#include "showdlg.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

const DWORD MAX_LOADSTRING = 256;
const IDT_TIMER1 = 1234;

/////////////////////////////////////////////////////////////////////////////

static HINSTANCE hInst;					

static TCHAR szTitle[MAX_LOADSTRING];		
static const TCHAR szWindowClass[MAX_LOADSTRING] = "ext2viewerWClass";

static HWND hwndTopRebar = NULL;
static HWND hwndDiskBar = NULL;
static HWND hwndFileBar = NULL;
static HWND hwndSplitter =  NULL;
static HWND hwndStatusBar = NULL;

static explorer *g_explorer;
static HTREEITEM g_hCurrentTreeItem = NULL;
static HTREEITEM g_hNewTreeItem = NULL;
static BOOL g_bBackSel = FALSE;
static BOOL g_bOpenAndExpand;
static HINSTANCE g_hRichEditLib;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////////////////////////////////////////////////////////////////////

ATOM RegisterCustomClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	
	wcex.cbSize = sizeof(WNDCLASSEX); 
	
	wcex.style			= 0;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_INTERF);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm = NULL;
	
	return RegisterClassEx(&wcex);
}

/////////////////////////////////////////////////////////////////////////////

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	RegisterCustomClass(hInstance);
	
	hInst = hInstance; // Store instance handle in our global variable
	
	
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);
	// init RichEdit control
	g_hRichEditLib = LoadLibrary("RICHED32.DLL");

	
	HWND hWnd;
	hWnd = CreateWindow(szWindowClass,
		lpApplicationName,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL);
	
	if (!hWnd)
        return FALSE;
	
	hwndTopRebar = CreateCoolMenu(hWnd, hInst, IDC_INTERF);
	RECT rect;
	GetWindowRect(hwndTopRebar, &rect);
	hwndDiskBar = CreateDiskBar(hWnd, rect.bottom - rect.top, 160);
	hwndFileBar = CreateFileBar(hWnd, 160 + SPLITTER_WIDTH, rect.bottom - rect.top);
	hwndSplitter = CreateVSplitter(hWnd, hwndDiskBar, hwndFileBar, 160, rect.bottom - rect.top, 200);
    hwndStatusBar = CreateStatusBar(hWnd, hInst);
	
	
	g_explorer = create_explorer(GetDiskbarTreeView(), hwndFileBar, hwndStatusBar, hWnd);
	
	if (g_explorer)
	{
		e_object eo;
		// register computer object
		eo.op[OP_DB_OPEN]      = eo_comp_db_open;
		eo.op[OP_DB_EXPAND]    = eo_comp_db_expand;
		eo.op[OP_DB_OPENEXPAND]= eo_comp_db_openexpand;
		eo.op[OP_FB_CALLBACK]  = eo_comp_fb_callback;
		eo.op[OP_FB_CLEANUP]   = eo_comp_fb_cleanup;
		eo.op[OP_FB_GETITEMCOUNT]   = eo_comp_fb_getitemcount;
		eo.op[OP_FB_PREPARECOLUMNS] = eo_comp_fb_preparecolumns;
		eo.op[OP_DB_PROPS] = NULL;
		eo.op[OP_FB_PROPS] = eo_comp_fb_props;
		eo.op[OP_REFRESH]  = eo_comp_refresh;
		eo.op[OP_SPEC1]    = NULL;
		eo.op[OP_FB_COPY]  = NULL;
		eo.op[OP_FB_CANCOPY]  = eo_comp_fb_cancopy;
		eo.op[OP_DB_COPY]     = NULL;
		eo.op[OP_DB_CANCOPY]  = NULL;
        eo.op[OP_FB_CHANGED]  = eo_comp_fb_changed;
        eo.op[OP_FB_ENTER]   = NULL;
		eo.op[OP_FB_CANVIEW] = NULL;
		eo.op[OP_FB_VIEW]    = NULL;

		register_explorer_obj(g_explorer, EO_COMPUTER_ID, eo);
		
		// register folder object
		eo.op[OP_DB_OPEN]      = eo_folder_db_open;
		eo.op[OP_DB_EXPAND]    = eo_folder_db_expand;
		eo.op[OP_DB_OPENEXPAND]     = eo_folder_db_openexpand;
		eo.op[OP_FB_CALLBACK]       = eo_folder_fb_callback;
		eo.op[OP_FB_GETITEMCOUNT]   = eo_folder_fb_getitemcount;
		eo.op[OP_FB_CLEANUP]        = eo_folder_fb_cleanup;
		eo.op[OP_FB_PREPARECOLUMNS] = eo_folder_fb_preparecolumns;
		eo.op[OP_DB_PROPS] = eo_folder_db_props;
		eo.op[OP_FB_PROPS] = eo_folder_fb_props;
		eo.op[OP_REFRESH]  = eo_folder_refresh;
		eo.op[OP_SPEC1]    = NULL;
		eo.op[OP_FB_COPY]  = eo_folder_fb_copy;
		eo.op[OP_FB_CANCOPY]  = eo_folder_fb_cancopy;
		eo.op[OP_DB_COPY]     = eo_folder_db_copy;
		eo.op[OP_DB_CANCOPY]  = eo_folder_db_cancopy;
		eo.op[OP_FB_CHANGED]  = eo_folder_fb_changed;
        eo.op[OP_FB_ENTER]    = eo_folder_fb_enter;
		eo.op[OP_FB_CANVIEW]  = eo_folder_fb_canview;
		eo.op[OP_FB_VIEW]     = eo_folder_fb_view;
		
		register_explorer_obj(g_explorer, EO_FOLDER_ID, eo);
		
		control_explorer(g_explorer, E_INIT, 0);
	}
	else 
        PostMessage(hWnd, IDM_EXIT, 0, 0); 

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	SetFocus(GetDiskbarTreeView());

	///////////
#ifdef _DEBUG
	//char s[] = "111\r\n222\r\n333\r\n444\r\nНи хрена себе";
//	char s[] = "111\n";
//	create_view(g_explorer, (BYTE *)s, lstrlen(s), "2.txt");
#endif
	///////////
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

UINT CheckCmdState(UINT uState)
{
	UINT uResult;
	switch (uState)
	{
	case IDM_EXIT:
		uResult = MFS_ENABLED;
		break;

		//  filebar style:
	case IDM_FB_ICON:
		if (control_explorer(g_explorer, E_GETFBSTYLE, 0) == FB_STYLE_ICON)
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
		break;

	case IDM_FB_SMALLICON:
		if (control_explorer(g_explorer, E_GETFBSTYLE, 0) == FB_STYLE_SMALLICON) 
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
		break;

	case IDM_FB_REPORT:
		if (control_explorer(g_explorer, E_GETFBSTYLE, 0) == FB_STYLE_REPORT) 
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
		break;

	case IDM_FB_LIST:
		if (control_explorer(g_explorer, E_GETFBSTYLE, 0) == FB_STYLE_LIST) 
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
		break;

    case IDM_STATUSBAR:
		if (control_explorer(g_explorer, E_GETISSB, 0)) 
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
        break;
		//
	case IDM_OPTIONS:
		uResult = MFS_ENABLED;
		break;
		// filebar  sort order
	case IDM_SORT_BY_NAME:
        uResult =  control_explorer(g_explorer, E_GETFBSORTORDER, 0) == FB_SORT_BY_NAME
            ? MFS_ENABLED | MFS_CHECKED : MFS_ENABLED;
		break;

	case IDM_SORT_BY_TYPE:
		if (control_explorer(g_explorer, E_GETFBSORTORDER, 0) == FB_SORT_BY_TYPE) 
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
		break;

	case IDM_SORT_BY_SIZE:
		if (control_explorer(g_explorer, E_GETFBSORTORDER, 0) == FB_SORT_BY_SIZE) 
            uResult =  MFS_ENABLED | MFS_CHECKED;
		else
            uResult = MFS_ENABLED;
		break;

	case IDM_SORT_BY_DATE:
		if (control_explorer(g_explorer, E_GETFBSORTORDER, 0) == FB_SORT_BY_DATE)
			uResult =  MFS_ENABLED | MFS_CHECKED;
		else
			uResult = MFS_ENABLED;
		break;

	case IDM_SORT_BY_NONE:
		if (control_explorer(g_explorer, E_GETFBSORTORDER, 0) == FB_SORT_BY_NONE) 
			uResult =  MFS_ENABLED | MFS_CHECKED;
		else
			uResult = MFS_ENABLED;
		break;

	case IDM_COPY:
		uResult = control_explorer(g_explorer, E_COPY, TRUE)
            ? MFS_ENABLED : MFS_DISABLED;
		break;

		// properties
	case IDM_PROPS:
		uResult = control_explorer(g_explorer, E_SOMEFOCUSED, 0)
            ? MFS_ENABLED : MFS_DISABLED;
		break;

	case IDM_REFRESH:
		uResult = MFS_ENABLED;
		break;

	case IDM_VIEW:
		uResult = control_explorer(g_explorer, E_VIEW, TRUE)
            ? MFS_ENABLED : MFS_DISABLED;
		break;
	default:
		uResult = MFS_ENABLED;
	}
	return uResult;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	RECT rect;
	DWORD dwMainClW, dwMainClH;
	DWORD dwTopOffset;
	DWORD dwDiskbarWidth;
    DWORD dwBottomOffset;
	HDWP hdwp;
	
	if(PreProcessMessage(hWnd, message, wParam, lParam)) return 0;
	
	switch (message) 
	{
	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			switch (wmId)
			{
				// exit...:(
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;

				// set filebar style
			case IDM_FB_REPORT:
				control_explorer(g_explorer, E_SETFBSTYLE, FB_STYLE_REPORT);
				break;

			case IDM_FB_LIST:
				control_explorer(g_explorer, E_SETFBSTYLE, FB_STYLE_LIST);
				break;

			case IDM_FB_ICON:
				control_explorer(g_explorer, E_SETFBSTYLE, FB_STYLE_ICON);
				break;

			case IDM_FB_SMALLICON:
				control_explorer(g_explorer, E_SETFBSTYLE, FB_STYLE_SMALLICON);
				break;

				// gonna options
			case IDM_OPTIONS:
				control_explorer(g_explorer, E_OPTIONS, 0);
				break;

				// set filebar sort order
			case IDM_SORT_BY_NAME:
				control_explorer(g_explorer, E_SETFBSORTORDER, FB_SORT_BY_NAME);
				break;

			case IDM_SORT_BY_TYPE:
				control_explorer(g_explorer, E_SETFBSORTORDER, FB_SORT_BY_TYPE);
				break;

			case IDM_SORT_BY_SIZE:
				control_explorer(g_explorer, E_SETFBSORTORDER, FB_SORT_BY_SIZE);
				break;

			case IDM_SORT_BY_DATE:
				control_explorer(g_explorer, E_SETFBSORTORDER, FB_SORT_BY_DATE);
				break;

			case IDM_SORT_BY_NONE:
				control_explorer(g_explorer, E_SETFBSORTORDER, FB_SORT_BY_NONE);
				break;

			case IDM_PROPS:
				control_explorer(g_explorer, E_SHOWPROPERTY, 0);
				break;

			case IDM_REFRESH:
				dispatch_explorer(g_explorer, OP_REFRESH, 0);
				break;

			case IDM_COPY:
				control_explorer(g_explorer, E_COPY, FALSE);
				break;

			case IDM_ABOUT:
				control_explorer(g_explorer, E_ABOUT, 0);
				break;

            case IDM_STATUSBAR:
                control_explorer(g_explorer, E_TOGGLESB, 0);
                break;

			case IDM_VIEW:
				control_explorer(g_explorer, E_VIEW, FALSE);
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO *lpMMI = (MINMAXINFO *)lParam;
            lpMMI->ptMinTrackSize.y += 100;
            lpMMI->ptMinTrackSize.x += 80;
        }
        break;
		
	case WM_SIZE:

        // resize status bar
        SendMessage(hwndStatusBar, WM_SIZE, wParam, lParam);

		dwMainClW = LOWORD(lParam);
		dwMainClH = HIWORD(lParam);

        GetWindowRect(hwndStatusBar, &rect);
        dwBottomOffset = rect.bottom - rect.top;
        if (!control_explorer(g_explorer, E_GETISSB, 0)) dwBottomOffset = 0;

		hdwp = BeginDeferWindowPos(4);
		// resize menu bar
		GetWindowRect(hwndTopRebar, &rect);
		dwTopOffset = rect.bottom - rect.top;
		DeferWindowPos(hdwp, hwndTopRebar, NULL, 0, 0,
			dwMainClW, dwTopOffset, SWP_NOMOVE | SWP_NOZORDER);
		// resize disk bar
		GetWindowRect(hwndDiskBar, &rect);
		dwDiskbarWidth = rect.right - rect.left;
		DeferWindowPos(hdwp, hwndDiskBar, NULL, 0, 0,
			dwDiskbarWidth,
			dwMainClH - dwTopOffset - DISKBAR_CONST1 - dwBottomOffset,
			SWP_NOMOVE | SWP_NOZORDER);
		// resize file bar
		DeferWindowPos(hdwp, hwndFileBar, NULL, 0, 0,
			dwMainClW - dwDiskbarWidth - SPLITTER_WIDTH,
			dwMainClH - dwTopOffset - 3 - dwBottomOffset,
			SWP_NOMOVE | SWP_NOZORDER);
		// resize splitter
		GetWindowRect(hwndSplitter, &rect);
		DeferWindowPos(hdwp, hwndSplitter, NULL, 0, 0,
			SPLITTER_WIDTH,
			dwMainClH - dwTopOffset - dwBottomOffset,
			SWP_NOMOVE | SWP_NOZORDER);
		EndDeferWindowPos(hdwp);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR)lParam;
			switch(pnmh->code)
			{
			case LVN_GETDISPINFO:
				dispatch_explorer(g_explorer, OP_FB_CALLBACK, lParam);
				// no return value
				break;

			case LVN_ODFINDITEM:
                TR("LVN_ODFINDITEM");
				// TODO
				return -1; // no item is found
				break;

            case LVN_ODSTATECHANGED:
                TR("LVN_ODSTATECHANGED");
                return 0;
                break;

            case  LVN_ITEMCHANGED:
                TR("LVN_ITEMCHANGED");
                dispatch_explorer(g_explorer, OP_FB_CHANGED, 0);
                return 0;

			case TVN_SELCHANGED:
				///////
				TR("Sel changed\n");
				/////////
				g_bOpenAndExpand = FALSE;
				if (!g_bBackSel)
				{
					KillTimer(hWnd, IDT_TIMER1);
					g_hNewTreeItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
					if (!g_hNewTreeItem)
                        g_hCurrentTreeItem = NULL;
					else 
                    {
                        DWORD cTime = g_explorer->intchange ? 1, g_explorer->intchange = FALSE
                            : 250;
                        SetTimer(hWnd, IDT_TIMER1, cTime, NULL);
                    }
				}
				else
                    g_bBackSel = FALSE;
				break;

			case TVN_ITEMEXPANDING:
				{
					//////////
					TR("Expanding\n");
					//////////
					BOOL bResult;
					NMTREEVIEW *pnmtv = (NMTREEVIEW*)lParam;
					if (pnmtv->action == TVE_EXPAND)
					{
						if (!g_hNewTreeItem) 
							bResult = dispatch_explorer(g_explorer, OP_DB_EXPAND, lParam);
						else // если folder выделен, но еще не  открыт
						{
							bResult = TRUE; 
							g_bOpenAndExpand = TRUE; // откладываем expand до open
						}
					}
					else
                        bResult = FALSE; // allow to expand or collapse
					return bResult; 
				}
				break;

			case NM_CLICK:
				{
					//////////
					TR("On click\n");
					////////
					int result = 0;
					if (pnmh->hwndFrom == GetDiskbarTreeView())
						result = control_explorer(g_explorer, E_REFRESH_BY_CLICK, 0);		
					return result; /* 0 = allow default processing */
				}
				break;

			case NM_DBLCLK:
				/////////
				TR("Dbl click\n");
				//////////
                if (pnmh->hwndFrom == g_explorer->hwndFilebar)
                    dispatch_explorer(g_explorer, OP_FB_ENTER, ENTER_BY_CLICK);

				return 0; /* not used */
				break;

			case NM_RETURN:
				{
					int result = 0;
					if (pnmh->hwndFrom == GetDiskbarTreeView())
						result = control_explorer(g_explorer, E_REFRESH_BY_RETURN, 0);		
                    else if (pnmh->hwndFrom == g_explorer->hwndFilebar)
                        result = dispatch_explorer(g_explorer, OP_FB_ENTER, ENTER_BY_RETURN);

					return result; /* 0 = allow default processing (TREEVIEW) */
				}
				break;

			case NM_RCLICK:
				TR("Right click\n");
				control_explorer(g_explorer, E_CNTX_MENU, 0);		
				break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
                break;
			}
		}
		break;
	case WM_INITMENUPOPUP:
		{
			HMENU hPopupMenu = (HMENU)wParam;
			for (int i = 0; i < GetMenuItemCount(hPopupMenu); i++)
			{
				MENUITEMINFO mii;
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_SUBMENU;
				GetMenuItemInfo(hPopupMenu, i, TRUE, &mii);
				
				UINT uNewItemState = mii.hSubMenu ? MFS_ENABLED : CheckCmdState(mii.wID);
				mii.cbSize = sizeof(mii); //!
				mii.fMask = MIIM_STATE;
				mii.fState = uNewItemState;
				SetMenuItemInfo(hPopupMenu, i, TRUE, &mii);
			}
			return 0; // message processed
		}
		break;
	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_TIMER1:
				TR("On timer\n");
				KillTimer(hWnd, IDT_TIMER1);

				TVITEM tvi;
				tvi.mask = TVIF_PARAM;
				tvi.hItem = g_hNewTreeItem;
				TreeView_GetItem(GetDiskbarTreeView(), &tvi);

				EO_OP op = g_bOpenAndExpand ? OP_DB_OPENEXPAND : OP_DB_OPEN;
				if (dispatch_explorer(g_explorer, op, (DWORD)&tvi))
				{
					g_bBackSel = TRUE;
					TreeView_SelectItem(GetDiskbarTreeView(), g_hCurrentTreeItem);
				}
				else
				{
					g_hCurrentTreeItem = g_hNewTreeItem;
					g_hNewTreeItem = NULL;
					if (g_bOpenAndExpand) TreeView_Expand(GetDiskbarTreeView(), g_hCurrentTreeItem, TVE_EXPAND);
				}
				break;
			}
		}
		break;
		
	case WM_INTERNAL_REFRESH:
		control_explorer(g_explorer, E_INTERNAL_REFRESH, wParam);
		break;

	case WM_REFRESH:
		control_explorer(g_explorer, E_REFRESH_REMOVABLE, 0);
		break;

	case WM_SYSCOLORCHANGE:
		// forward messages to controls
		SendMessage(hwndFileBar, WM_SYSCOLORCHANGE, 0, 0);
		SendMessage(hwndDiskBar, WM_SYSCOLORCHANGE, 0, 0);
		SendMessage(hwndTopRebar, WM_SYSCOLORCHANGE, 0, 0);
		SendMessage(GetDiskbarTreeView(), WM_SYSCOLORCHANGE, 0, 0);
		CoolMenuRecalc();
		break;

	case WM_ACTIVATE:
		{
			static HWND hFocusedWnd = 0;
			if (wParam == WA_INACTIVE)
				hFocusedWnd = GetFocus();
			else
				SetFocus(hFocusedWnd);
		}
		break;

	case WM_DESTROY:
		control_explorer(g_explorer, E_CLEANUP, 0);
		PostQuitMessage(0);
		break;

	case WM_NCDESTROY:
		destroy_explorer(g_explorer);
		// fall through
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

BOOL check_install()
{
    char *ppCheckList[3];
    ppCheckList[0] = (char *)DLL_DIONT;
    ppCheckList[1] = (char *)DLL_DIO9x;
    ppCheckList[2] = (char *)DLL_DIO9x16;

    BOOL bResult = TRUE;
    for (int i = 0; i < 3; i++)
    {
        WIN32_FIND_DATA data;
        HANDLE hFind = FindFirstFile(ppCheckList[i], &data);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            char buffer[512];
            wsprintf(buffer, Message(MSG_LIBNOTFOUND), ppCheckList[i]);
            MessageBox(NULL, buffer, lpApplicationName, MB_OK | MB_ICONERROR);
            bResult = FALSE;
            break;
        }
        FindClose(hFind);
    }
    return bResult;
}

/////////////////////////////////////////////////////////////////////////////

void check_current_dir()
{
	char filename[MAX_PATH];
	GetModuleFileName(NULL, filename, MAX_PATH);

	int i = lstrlen(filename);
	while (filename[i] != '\\' && i > 0) 
		i--;

	filename[i] = 0;

	SetCurrentDirectory(filename);
}

/////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	check_current_dir();

#ifndef _DEBUG
    if (!check_install())
        return 1;
#endif

	if (InitInstance(hInstance, nCmdShow)) 
	{
		HACCEL  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_INTERF);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			// try translate accel
			HWND hwndFrame = (HWND)GetWindowLong(msg.hwnd, GWL_HWNDPARENT);
			if (hwndFrame && TranslateAccelerator(hwndFrame, hAccelTable, &msg)) 
			{
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	
	return 1;
}

