#include <stdafx.h>

void ShowSplitter(RECT &rc, BOOL bDrawNew = TRUE);

static HWND hwndLeft = NULL;
static HWND hwndRight = NULL;
static BOOL bMouseCaptured = FALSE;
static BOOL bSplitterWork = FALSE;
static RECT rSplitterRect;

const char lpSplitterWClass[] = "SplitterWClass";


LRESULT CALLBACK SplitterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	switch(message)
	{
	case WM_LBUTTONDOWN:
		{
			// capture mice input
			SetCapture(hWnd);
			bMouseCaptured = TRUE;

			// show splitter
			HWND hwndParent = (HWND)GetWindowLong(hWnd, GWL_HWNDPARENT);
			HDC dc = GetDC(hwndParent);
			// get splitter size
			RECT rc;
			GetWindowRect(hWnd, &rc);
			//POINT a;
			//a.x = rc.left; a.y = rc.top;
			//ScreenToClient(hwndParent, &a);
			
			//RECT rrc;
			//rrc.left = a.x;
			//rrc.top = a.y;
			//rrc.bottom = a.y + rc.bottom - rc.top;
			//rrc.right = a.x + SPLITTER_WIDTH;
			
			ShowSplitter(rc);
			
			ReleaseDC(hwndParent, dc);
		}
		break;
	case WM_LBUTTONUP:
		if (bSplitterWork)
		{
			// hide splitter
			HWND hwndParent = (HWND)GetWindowLong(hWnd, GWL_HWNDPARENT);
			RECT rc;
			GetWindowRect(hWnd, &rc);
			ShowSplitter(rc, FALSE);
		
			int dx = -rc.left + rSplitterRect.left;

			POINT  a, b;
			RECT rcleft, rcright;
			// resize left window
			GetWindowRect(hwndLeft, &rcleft);
			a.x = rcleft.left; a.y = rcleft.top;
			b.x = rcleft.right; b.y = rcleft.bottom;
			ScreenToClient(hwndParent, &a);
			ScreenToClient(hwndParent, &b);
			SetWindowPos(hwndLeft, NULL,
				0, 0, b.x - a.x + dx, 
				b.y - a.y, SWP_NOMOVE | SWP_NOZORDER);
			// resize right window
			GetWindowRect(hwndRight, &rcright);

			a.x = rcright.left; a.y = rcright.top;
			b.x = rcright.right; b.y = rcright.bottom;
			ScreenToClient(hwndParent, &a);
			ScreenToClient(hwndParent, &b);
			SetWindowPos(hwndRight, NULL,
				a.x + dx, a.y, b.x - a.x - dx, 
				b.y - a.y, SWP_NOZORDER);
			// resize splitter
			a.x = rc.left; a.y = rc.top;
			b.x = rc.right; b.y = rc.bottom;
			ScreenToClient(hwndParent, &a);
			ScreenToClient(hwndParent, &b);
			SetWindowPos(hWnd, NULL,
				a.x + dx, a.y, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER);
			InvalidateRect(hWnd, NULL, TRUE);
			
		}
		
		ReleaseCapture();
		bMouseCaptured = FALSE;
		break;	
	case WM_MOUSEMOVE:
		if (bMouseCaptured)
		{
			short micex =  LOWORD(lParam);

			// show splitter
			HWND hwndParent = (HWND)GetWindowLong(hWnd, GWL_HWNDPARENT);

			RECT rcleft, rcright;
			GetWindowRect(hwndLeft, &rcleft);
			GetWindowRect(hwndRight, &rcright);

			int dx = micex - SPLITTER_WIDTH/2;

			if (rcleft.right - rcleft.left + dx > SPL_WIN_MIN_SIZE &&
				rcright.right - rcright.left - dx > SPL_WIN_MIN_SIZE)
			{
				RECT rc;
				GetWindowRect(hWnd, &rc);
				rc.left += dx;
				rc.right += dx;
				ShowSplitter(rc);
			}
		}
		break;
	default:
		result = DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}

 
HWND CreateVSplitter(HWND hwndParent, HWND hwndLeftWin, HWND hwndRightWin, 
				DWORD dwLeftOffset, DWORD dwStartY, DWORD dwEndY)
{

	HWND hWnd = NULL;

	HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwndParent, GWL_HINSTANCE);
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_SAVEBITS*0;
	wcex.lpfnWndProc	= (WNDPROC)SplitterWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_SIZEWE);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= lpSplitterWClass;
	wcex.hIconSm		= NULL;

	if (RegisterClassEx(&wcex))
	{
		hWnd = CreateWindowEx(0,
			lpSplitterWClass,
			NULL,
			WS_CHILD  | WS_OVERLAPPED | WS_VISIBLE ,
			dwLeftOffset, dwStartY, SPLITTER_WIDTH, dwEndY - dwStartY,
			hwndParent,
			NULL,
			hInst,
			NULL);
		hwndLeft = hwndLeftWin;
		hwndRight = hwndRightWin;
	}
	return hWnd;
}


void ShowSplitter(RECT &rc, BOOL bDrawNew)
{
	HDC dc = GetDC(NULL);
	HDC hmdc = CreateCompatibleDC(dc);
	HBITMAP hbmp = CreateCompatibleBitmap(dc, 8, 8);
	HBITMAP hobmp = (HBITMAP)SelectObject(hmdc, hbmp);
	
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			COLORREF cf;
			cf =  (i + j)&0x1 ? RGB(0, 0, 0) : RGB(255, 255, 255); 
			SetPixel(hmdc, i, j, cf);
		}
	}
	
	HBRUSH hBrush = CreatePatternBrush(hbmp);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(dc, hBrush);
	
	SelectObject(hmdc, hobmp);
	DeleteObject(hbmp);
	DeleteDC(hmdc);
	if (!bDrawNew || rc.left != rSplitterRect.left || !bSplitterWork)
	{
		
		if (bSplitterWork)
		{
			PatBlt(dc, rSplitterRect.left,
				rSplitterRect.top,
				rSplitterRect.right - rSplitterRect.left,
				rSplitterRect.bottom -  rSplitterRect.top, 
				PATINVERT);
			bSplitterWork = FALSE;
		}
		
		if (bDrawNew)
		{
			PatBlt(dc, rc.left,
				rc.top,
				rc.right - rc.left,
				rc.bottom -  rc.top, 
				PATINVERT);
			rSplitterRect = rc;
			bSplitterWork = TRUE;
		}
	}
	SelectObject(dc, hOldBrush);
	DeleteObject(hBrush);
	ReleaseDC(NULL, dc);
}