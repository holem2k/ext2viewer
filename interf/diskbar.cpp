#include <stdafx.h>
#include <assert.h>
#include "icons.h"

const char lpDiskbarClassName[] = "DiskbarWClass";

static HFONT g_hDiskbarFont = NULL;
static HWND g_hwndTreeView = NULL;
static WNDPROC lpOldWndProc;

////////////////////////////////////////////////////////////////////

HWND CreateTreeControl(HWND hwndParent, DWORD dwWidth, DWORD dwHeight)
{
    HWND hwndTV = CreateWindowEx(WS_EX_CLIENTEDGE, 
		WC_TREEVIEW, 
		NULL, 
        WS_VISIBLE | WS_CHILD |
		WS_CLIPSIBLINGS | WS_TABSTOP |
		TVS_HASLINES | TVS_HASBUTTONS |
		WS_HSCROLL | WS_VSCROLL, 
        0, DISKBAR_CONST2, dwWidth, dwHeight - DISKBAR_CONST2, 
        hwndParent,
		(HMENU)ID_DESKBAR_TV,
		(HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE),
		NULL); 

	HIMAGELIST himIcons = CreateIconList(DiskbarIcons, DiskbarIconsNum);
	TreeView_SetImageList(hwndTV, himIcons, TVSIL_NORMAL);
	return  hwndTV;
}

////////////////////////////////////////////////////////////////////

LRESULT CALLBACK DiskBarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	WORD wWidth, wHeight;
	switch(message)
	{
	case WM_SIZE:
		wWidth = LOWORD(lParam);
		wHeight = HIWORD(lParam);
		SetWindowPos(g_hwndTreeView, NULL,
			0, 0, wWidth,wHeight - DISKBAR_CONST2,
			SWP_NOMOVE | SWP_NOZORDER); 
		result = 0;
		break;
	case WM_PAINT:
		{
			RECT rc, irect;
			GetClientRect(hWnd, &rc);

			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hWnd, &ps);
			FillRect(dc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
			EndPaint(hWnd, &ps);

			SetRect(&irect, rc.left, rc.top, rc.right, rc.top + 3);
			InvalidateRect(hWnd, &irect, FALSE);
			SetRect(&irect, rc.right - 3, rc.top, rc.right, /*rc.bottom*/ rc.top + DISKBAR_CONST2);
			InvalidateRect(hWnd, &irect, FALSE);
			SetRect(&irect, rc.left, rc.top, rc.left + 2, /*rc.bottom*/ rc.top + DISKBAR_CONST2);
			InvalidateRect(hWnd, &irect, FALSE);

			result = CallWindowProc(lpOldWndProc, hWnd,  message, wParam, lParam);

			rc.top += 3; rc.left += 2; rc.right -= 2;rc.bottom =rc.top + DISKBAR_CONST2;
			InvalidateRect(hWnd, &rc, FALSE);
			dc = BeginPaint(hWnd, &ps);
			FillRect(dc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
			EndPaint(hWnd, &ps);
			result = 0;
		}
		break;
		// last message indeed...
	case WM_NCDESTROY:
		{
			CallWindowProc(lpOldWndProc, hWnd,  message, wParam, lParam);
			DeleteObject(g_hDiskbarFont);
			SetWindowLong(hWnd, GWL_WNDPROC, (LONG)lpOldWndProc);
			result = 0;
		}
		break;
	case WM_NOTIFY:
		{
			HWND hParent = (HWND)GetWindowLong(hWnd, GWL_HWNDPARENT);
			result = SendMessage(hParent, WM_NOTIFY, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		{
			// child windows still alive...
			HIMAGELIST himlIcons = TreeView_SetImageList(g_hwndTreeView, NULL, TVSIL_NORMAL);
			ImageList_Destroy(himlIcons);
			result = CallWindowProc(lpOldWndProc, hWnd,  message, wParam, lParam);
		}
		break;
	default:
		result = CallWindowProc(lpOldWndProc, hWnd,  message, wParam, lParam);
	}
	return result;
}

////////////////////////////////////////////////////////////////////

HWND CreateDiskBar(HWND hwndParent, DWORD dwTopOffset, DWORD dwWidth)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE);

	WNDCLASS wndClass;
	GetClassInfo(NULL, "Button", &wndClass);
	wndClass.style = 0;
	wndClass.hInstance = hInst;
	wndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndClass.lpszClassName = lpDiskbarClassName;
	ATOM a = RegisterClass(&wndClass);

	RECT rect;
	GetClientRect(hwndParent, &rect);
	DWORD dwHeight = rect.bottom - rect.top - dwTopOffset;

	HWND hwndDiskbar = NULL;
	hwndDiskbar = CreateWindowEx(0, lpDiskbarClassName,
		NULL,
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/,  
		0,	dwTopOffset + DISKBAR_CONST1, dwWidth, dwHeight - DISKBAR_CONST1,       
		hwndParent,  
		NULL,
		hInst, 
		NULL);
	
	lpOldWndProc = (WNDPROC)SetWindowLong(hwndDiskbar, GWL_WNDPROC, (LONG)DiskBarWndProc);

	g_hwndTreeView = CreateTreeControl(hwndDiskbar, dwWidth, dwHeight - DISKBAR_CONST1);
	

	LOGFONT lFont;
	lFont.lfHeight = 1;
	lFont.lfWidth = 0;
	lFont.lfEscapement = 0;
	lFont.lfOrientation = 0;
	lFont.lfWeight = 200; 
	lFont.lfItalic = 0;
	lFont.lfUnderline = 0;
	lFont.lfStrikeOut = 0;
	lFont.lfCharSet = ANSI_CHARSET;
	lFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lFont.lfQuality = DEFAULT_QUALITY;
	lFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	lFont.lfFaceName[0] = NULL;
	g_hDiskbarFont = CreateFontIndirect(&lFont);
	assert(g_hDiskbarFont);
	SendMessage(hwndDiskbar, WM_SETFONT, (WPARAM)g_hDiskbarFont, TRUE);

	return hwndDiskbar;
}

////////////////////////////////////////////////////////////////////

HWND GetDiskbarTreeView()
{
	assert(IsWindow(g_hwndTreeView));
	return g_hwndTreeView;
}

////////////////////////////////////////////////////////////////////
