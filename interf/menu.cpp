#include "stdafx.h"
#include <assert.h>

BOOL MenuInputFilter(MSG *msg);

const DWORD ID_TOPTOOLBAR = 13469;

// I know it's bad...
static HWND g_hwndToolbar;
static HWND g_hwndRebar;
static WNDPROC g_lpOldWndProc;
static DWORD g_MenuId;
static HHOOK g_MsgHook;
static int g_iItem = -1; // текущее меню
static HINSTANCE g_hInst = NULL;
static HWND g_hwndParent;
static HFONT g_hMenuFont = NULL;
static BOOL bStringsLoaded = FALSE;

// menu specific
static int g_MenuItemCount = -1;         // число пунктов в MenuBar
static LPARAM g_LastMouseMovePoint = 0;
static BOOL g_FakeCancelMode = FALSE;    // переключение меню, а не его закрытие
static HMENU g_hSubMenu = NULL;
static HWND g_hwndFocusFrom = NULL;      // окно, с которого перешел фокус на тулбар
static int g_EscapePressOnItem = -1;     // кнопка на которой нажали Escape

////////////////////////////////////////////////////////////////////

void UpdateFont()
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	if (g_hMenuFont) 
		DeleteObject(g_hMenuFont);
	g_hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);
	SendMessage(g_hwndToolbar, WM_SETFONT, (WPARAM)g_hMenuFont, TRUE);
}


////////////////////////////////////////////////////////////////////

void ResizeToolbar()
{
	DWORD dwSize = SendMessage(g_hwndToolbar, TB_GETBUTTONSIZE, 0, 0);
	TR(HIWORD(dwSize));
	MoveWindow(g_hwndToolbar, 10, 2, 200, HIWORD(dwSize), TRUE);
}

////////////////////////////////////////////////////////////////////

void LoadMenu()
{
   SendMessage(g_hwndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
   SendMessage(g_hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(1, 1));
   SendMessage(g_hwndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(0, 0));

   HMENU  hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(g_MenuId));
   g_MenuItemCount = GetMenuItemCount(hMenu);
   for (int i = 0; i < g_MenuItemCount; i++)
   {
	   char lpMenuItemString[257];

	   if (!bStringsLoaded)
	   {
		   GetMenuString(hMenu, i, lpMenuItemString, 256, MF_BYPOSITION);
		   lpMenuItemString[lstrlen(lpMenuItemString) + 1] = 0;
		   SendMessage(g_hwndToolbar, TB_ADDSTRING, 0, (LPARAM)lpMenuItemString); 
	   }
	   
	   TBBUTTON button;
	   button.iBitmap = -1; 
	   button.idCommand = i; 
	   button.fsState = TBSTATE_ENABLED; 
	   button.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE /*| TBSTYLE_DROPDOWN*/; 
	   button.dwData = 0; 
	   button.iString = (int)i;//lpMenuItemString; 

	   SendMessage(g_hwndToolbar, TB_ADDBUTTONS, 1, (LPARAM)&button);
   }

   DestroyMenu(hMenu);
   bStringsLoaded = TRUE;
}

////////////////////////////////////////////////////////////////////

HWND CreateMenuToolBar(HWND hwndParent, DWORD MenuId)
{
	g_hwndToolbar = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME,
		(LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | 
		TBSTYLE_FLAT | TBSTYLE_LIST | 
		CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_TOP | CCS_NORESIZE,
		0, 0, 200, 19,
		hwndParent, 
        (HMENU)ID_TOPTOOLBAR,
		g_hInst,
		NULL); 

	UpdateFont();
	
   g_MenuId = MenuId;
   LoadMenu();
   ResizeToolbar();

   SendMessage(g_hwndToolbar, TB_AUTOSIZE, 0, 0);
   return g_hwndToolbar;
}

////////////////////////////////////////////////////////////////////
// хук:)
//

LRESULT CALLBACK MessageProc(int code,  WPARAM wParam,  LPARAM lParam)
{
	return (code == MSGF_MENU && MenuInputFilter((MSG *)lParam)) ? TRUE
		: CallNextHookEx(g_MsgHook, code, wParam, lParam);
};

////////////////////////////////////////////////////////////////////
// нахождение кнопки меню(тулбара), по координате
//
int GetItemFromPoint(POINT &p)
{
	ScreenToClient(g_hwndToolbar, &p);
	RECT rc;
	GetClientRect(g_hwndToolbar, &rc);
	int item = -1;
	if (PtInRect(&rc, p)) item = SendMessage(g_hwndToolbar, TB_HITTEST, 0, (LPARAM)&p);
	if (item < 0 || item >= g_MenuItemCount) item = -1;
	// TODO
	// check is this toolbar item is visible inside  window
	/////////
	//char s[256];
	//wsprintf(s, "GetItemFromPoint -- %u\n", item);
	//OutputDebugString(s);
	//////////

	return item;
}

///////////////////////////////////////////////////////////////////
//  всплытие  пункта основного меню - g_iItem, 
//  конец, когда g_iItem == -1
//                    (изменяется хуком в MenuInputFilter(...));

void TrackPopup()
{
	HMENU hMenu = LoadMenu(NULL, MAKEINTRESOURCE(g_MenuId));
	g_MenuItemCount = GetMenuItemCount(hMenu); 
	while (g_iItem >= 0)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, g_iItem);
		g_hSubMenu = hSubMenu;
		/////////
		//char s[256];
		//wsprintf(s, "TrackPopup -- %u\n", g_iItem);
		//OutputDebugString(s);
		//////////
		assert(hMenu && hSubMenu);
		RECT rc;
		SendMessage(g_hwndToolbar, TB_GETRECT,
			(WPARAM)g_iItem, (LPARAM)&rc);
		
		MapWindowPoints(g_hwndToolbar, HWND_DESKTOP, (LPPOINT)&rc, 2);                         
		TPMPARAMS tpm;
		tpm.cbSize = sizeof(TPMPARAMS);
		tpm.rcExclude = rc;
		if (rc.left < 0) rc.left = 0;

		int iItemPressed = g_iItem;
		SendMessage(g_hwndToolbar, TB_PRESSBUTTON, g_iItem, TRUE);

		g_MsgHook = SetWindowsHookEx(WH_MSGFILTER, MessageProc, g_hInst, GetCurrentThreadId());
		TrackPopupMenuEx(hSubMenu,
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,               
			rc.left, rc.bottom, g_hwndParent, &tpm);
		UnhookWindowsHookEx(g_MsgHook);

		SendMessage(g_hwndToolbar, TB_PRESSBUTTON, iItemPressed, FALSE);
	}
	DestroyMenu(hMenu);

	//g_MenuItemCount = -1;
	g_hSubMenu = NULL;
}

////////////////////////////////////////////////////////////////////
// фильтрация сообщений от хука, поставленнрого при всплытии popup menu

BOOL MenuInputFilter(MSG *msg)
{
	BOOL result = FALSE;
	switch (msg->message)
	{
	case WM_MOUSEMOVE:
		{
			POINT p;
			p.x = LOWORD(msg->lParam); p.y = HIWORD(msg->lParam);
			int iItem = GetItemFromPoint(p);
			if (iItem >= 0 && g_iItem != iItem && g_LastMouseMovePoint != msg->lParam) 
			{
				g_FakeCancelMode = TRUE;
				PostMessage(g_hwndParent, WM_CANCELMODE, 0, 0);
				g_iItem = iItem;
			}
			g_LastMouseMovePoint = msg->lParam;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			POINT p;
			p.x = LOWORD(msg->lParam); p.y = HIWORD(msg->lParam);
			int iItem = GetItemFromPoint(p);
			if (iItem == g_iItem)
			{
				g_FakeCancelMode = TRUE;
				PostMessage(g_hwndParent, WM_CANCELMODE, 0, 0);
				result = TRUE;
			}
			g_iItem= -1;
		}
		break;
	case WM_KEYDOWN:
		{
			BOOL bHighLightedSubMenu = FALSE;
			BOOL bHighlighted = FALSE;

			for (int i = 0; i < GetMenuItemCount(g_hSubMenu) && !bHighlighted; i++)
			{
				MENUITEMINFO mii;
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_SUBMENU | MIIM_STATE;
				GetMenuItemInfo(g_hSubMenu, i, TRUE, &mii);
				bHighlighted = mii.fState == MFS_HILITE;
				bHighLightedSubMenu = bHighlighted && mii.hSubMenu;
			}
			
			switch (msg->wParam)
			{
			case VK_RIGHT:
				if (!bHighLightedSubMenu && g_MenuItemCount > 0)
				{
					g_iItem = (g_iItem + 1)%g_MenuItemCount;
					g_FakeCancelMode = TRUE;
					PostMessage(g_hwndParent, WM_CANCELMODE, 0, 0);
					result = TRUE;
				}
				break;
			case VK_LEFT:
				if (!bHighLightedSubMenu && g_iItem >= 0)
				{
					g_FakeCancelMode = TRUE;
					g_iItem = g_iItem > 0 ? g_iItem - 1 : g_MenuItemCount - 1;
					PostMessage(g_hwndParent, WM_CANCELMODE, 0, 0);
					result = TRUE;
				}
				break;
			case VK_RETURN:
				if (!bHighlighted)
				{
					g_FakeCancelMode = TRUE;
					g_iItem = - 1;
					PostMessage(g_hwndParent, WM_CANCELMODE, 0, 0);
					result = TRUE;
				}
				else
				{
					g_iItem = -1; //  выбран 
				}
				break;
			case VK_ESCAPE:
				{
					g_EscapePressOnItem = g_iItem;
					g_iItem = -1;
					PostMessage(g_hwndParent, WM_CANCELMODE, 0, 0);
					result = TRUE;
				}
				break;
			}
		}
		break;
	}
	return result;
}

////////////////////////////////////////////////////////////////////
// оконная фунция тулбара
//

LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (message)
	{
	case WM_LBUTTONDOWN:
		{
			POINT p;
			p.x = LOWORD(lParam); p.y = HIWORD(lParam);
			ClientToScreen(g_hwndToolbar, &p);
			g_iItem = GetItemFromPoint(p);
			g_LastMouseMovePoint = MAKELONG(p.x, p.y);
			
			if (g_iItem >= 0) TrackPopup();
		}
		break;
		// поступают при tracking'е
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
		case VK_DOWN:
			{
				int iHotItem = SendMessage(g_hwndToolbar, TB_GETHOTITEM, 0, 0);
				SendMessage(g_hwndToolbar, TB_SETHOTITEM, -1, 1);
				g_iItem = iHotItem;
				TrackPopup();
				SendMessage(g_hwndToolbar, TB_SETHOTITEM, g_EscapePressOnItem, 1);
			}
			break;
		case VK_ESCAPE:
			if (IsWindow(g_hwndFocusFrom)) SetFocus(g_hwndFocusFrom);
			break;
		default:
			result = CallWindowProc(g_lpOldWndProc, hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		DeleteObject(g_hMenuFont);
		break;
	case WM_NCDESTROY:
		//TODO - unsubclass
	default:
		result = CallWindowProc(g_lpOldWndProc, hWnd, message, wParam, lParam);
	}
	return result;
}


////////////////////////////////////////////////////////////////////
// это надо запускать из window proc  основного окна
// return true - если сообщение обработано

BOOL PreProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL result = FALSE;
	if (message == WM_SYSCOMMAND && wParam == SC_KEYMENU)
	{
			g_hwndFocusFrom = SetFocus(g_hwndToolbar);
			result = TRUE;
	}
	else if (message == WM_CANCELMODE)
	{
		if (!g_FakeCancelMode) g_iItem= -1;
			else g_FakeCancelMode = FALSE;
	}

	return result;
}

////////////////////////////////////////////////////////////////////
//
//

HWND CreateCoolMenu(HWND hwndParent, HINSTANCE hInst, DWORD MenuId)
{
	REBARINFO     rbi;
	REBARBANDINFO rbBand;
	HWND   hwndRB;

	g_MenuId = MenuId;	   
	g_hInst = hInst;
	g_hwndParent = hwndParent;

	RECT rc;
	GetClientRect(hwndParent, &rc);
	hwndRB = CreateWindowEx(WS_EX_TOOLWINDOW,
		   REBARCLASSNAME,
		   NULL,
		   WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER |
		   WS_CLIPCHILDREN | 
		   CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_TOP |
		   RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_DBLCLKTOGGLE | RBS_REGISTERDROP,
		   0, 0, 0, 0,
		   hwndParent,
		   NULL,
		   hInst,
		   NULL);
	   if(!hwndRB)  return NULL;
	   g_hwndRebar = hwndRB;	   
	   
	   rbi.cbSize = sizeof(REBARINFO);
	   rbi.fMask  = 0;
	   rbi.himl   = (HIMAGELIST)NULL;
	   if(!SendMessage(hwndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))  return NULL;
	   
	   
	   rbBand.cbSize = sizeof(REBARBANDINFO);  
	   rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE | RBBIM_SIZE; 
	   rbBand.fStyle = RBBS_CHILDEDGE | RBBS_BREAK | RBBS_GRIPPERALWAYS ;

	   g_hwndToolbar = CreateMenuToolBar(hwndRB, MenuId);
	   
	   GetWindowRect(g_hwndToolbar, &rc);
	   rbBand.hwndChild  = g_hwndToolbar;
	   rbBand.cxMinChild = 100;
	   rbBand.cyMinChild = rc.bottom - rc.top;
	   rbBand.cx = rc.right  -  rc.left;
	   
	   SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
	   SetWindowPos(g_hwndToolbar, NULL, 10, 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	   
	   g_lpOldWndProc = (WNDPROC)SetWindowLong(g_hwndToolbar, GWL_WNDPROC, (LONG)MenuWndProc);	   
	   return hwndRB;
}

////////////////////////////////////////////////////////////////////

void CoolMenuRecalc()
{
	SendMessage(g_hwndToolbar, WM_SYSCOLORCHANGE, 0, 0);
//	while (SendMessage(g_hwndToolbar, TB_DELETEBUTTON, 0, 0));
//	UpdateFont();
//	LoadMenu();
//	ResizeToolbar();
}

////////////////////////////////////////////////////////////////////


HWND CreateStatusBar(HWND hwndParent, HINSTANCE hInst)
{
    HWND hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL, hwndParent, IDC_STATUSBAR);

    if (hwndStatusBar)
    {
        int *pPartWidth = (int *)LocalAlloc(LPTR, sizeof(int)*2);
        pPartWidth[0] = 150;
        pPartWidth[1] = 2000;
        SendMessage(hwndStatusBar, SB_SETPARTS, 2, (LPARAM)pPartWidth);
        LocalFree((HANDLE)pPartWidth);
    }
    return hwndStatusBar;
}

////////////////////////////////////////////////////////////////////