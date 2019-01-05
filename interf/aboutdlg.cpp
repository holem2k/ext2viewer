#include "stdafx.h"
#include "aboutdlg.h"
#include "shellapi.h"

static BOOL g_bHighligtedMail;
static HBRUSH g_hBrush;

///////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		g_bHighligtedMail = FALSE;
		g_hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		bResult = TRUE;
		break;

	case WM_COMMAND:
		{
			switch (wParam)
			{
			case IDOK:
				EndDialog(hwndDlg, IDOK);
				bResult = 0;
				break;
			}
		}
		break;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_ABOUT_MAIL))
		{
			if (g_bHighligtedMail)
				SetTextColor((HDC)wParam, RGB(0, 0, 255));
			else
				SetTextColor((HDC)wParam, RGB(0, 0, 0));

			SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));

			bResult = (BOOL)g_hBrush;
		}
		else
			bResult = (BOOL)NULL;
		
		break;

	case WM_MOUSEMOVE:
		{
			RECT rect;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_ABOUT_MAIL), &rect);

			POINT p;
			POINTSTOPOINT(p, MAKEPOINTS(lParam));
			ClientToScreen(hwndDlg, &p);

			BOOL bPrevios = g_bHighligtedMail;
			if (PtInRect(&rect, p))
				g_bHighligtedMail = TRUE;
			else
				g_bHighligtedMail = FALSE;

			if (bPrevios != g_bHighligtedMail)
			{
				GetClientRect(GetDlgItem(hwndDlg, IDC_ABOUT_MAIL), &rect);
				MapWindowPoints(GetDlgItem(hwndDlg, IDC_ABOUT_MAIL), 
					hwndDlg, (LPPOINT)&rect, 2);
				
				InvalidateRect(hwndDlg, &rect, FALSE);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			RECT rect;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_ABOUT_MAIL), &rect);

			POINT p;
			POINTSTOPOINT(p, MAKEPOINTS(lParam));
			ClientToScreen(hwndDlg, &p);

			if (PtInRect(&rect, p))
			{
				ShellExecute(NULL, "open", "mailto:holem2k@hotmail.com", 
					NULL, NULL, SW_SHOWNORMAL);
			}
		}
		break;

	case WM_ACTIVATE:
		g_bHighligtedMail = FALSE;
		bResult = 0;
		break;
	}

	return bResult;
}		

//////////////////////////////////////////////////////////////////////////

void create_aboutdlg(HWND hParent)
{
	DialogBox((HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_ABOUT),
		hParent,
		AboutDialogProc);
}

//////////////////////////////////////////////////////////////////////////

