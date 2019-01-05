#include <stdafx.h>
#include <richedit.h>
#include <commdlg.h>
#include "showdlg.h"
#include "finddlg.h"
#include "registry.h"
#include "stat.h"

static DLGDATA g_Data[MAX_SHOWDLGS];
static DWORD g_cDataNum = 0;
static BOOL bRegWin = FALSE;

////////////////////////////////////////////////////////////////////////////////

static int getindx(HWND hView)
{
	int indx = -1;
	for (int i = 0; i < MAX_SHOWDLGS; i++)
	{
		if (g_Data[i].hView == hView && !g_Data[i].bEmpty)
		{
			indx = i;
			break;
		}
	}
	return indx;
}

////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK ViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lResult;
	switch (message)
	{
	case WM_FINALCREATE:
		{
			// g_Data is not valid for this dialog yet
			RECT rc;
			GetClientRect(hWnd, &rc);

			// create status bar
			HWND hStatusBar = CreateStatusWindow(WS_VISIBLE | WS_CHILD,
				NULL, hWnd, IDC_VIEW_STATUSBAR);
			int pPartWidth[3];
			pPartWidth[0] = 150;
			pPartWidth[1] = 230;
			pPartWidth[2] = 2000;
			SendMessage(hStatusBar, SB_SETPARTS, 3, (LPARAM)pPartWidth);

			RECT rcs;
			GetClientRect(hStatusBar, &rcs);

			// create rich control
			HWND hEdit = CreateWindowEx(0, "RichEdit", 0,
				WS_VISIBLE  | WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER |
				ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_READONLY,
				0, 0, rc.right - rc.left, rc.bottom - rc.top - rcs.bottom + rcs.top, 
				hWnd,
				(HMENU)IDC_VIEW_RICHEDIT,
				NULL, 0);
			SendMessage(hEdit, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS);
			SendMessage(hEdit, EM_SETTEXTMODE, TM_PLAINTEXT | TM_SINGLELEVELUNDO, 0);
			SetFocus(hEdit);

			lResult = 0;
		}
		break;

	case WM_LOADRICHEDIT:
		{
			int indx = getindx(hWnd);
			if (indx == -1)
				break;
			
			HWND hEdit = g_Data[indx].hEdit;

			// get selection
			DWORD x, y;
			SendMessage(hEdit, EM_GETSEL, (WPARAM)&x, (LPARAM)&y);

			// recode
			DWORD cbRecodeBuf = g_Data[indx].cbBuf;

			//char *lpRecodeBuf = new char[cbRecodeBuf + 1];
			//char *lpRecodeBuf = (char *)HeapAlloc(GetProcessHeap(), 0, cbRecodeBuf + 1);
			char *lpRecodeBuf = (char *)VirtualAlloc(NULL, cbRecodeBuf + 1, MEM_COMMIT, PAGE_READWRITE);

			if (!lpRecodeBuf)
			{
				lResult = 0;
				break;
			}

			char *lpBuf = g_Data[indx].lpBuf;

			CODE_ENUM code = g_Data[indx].code;
			for (DWORD i = 0; i < cbRecodeBuf; i++)
				lpRecodeBuf[i] = (BYTE)lpBuf[i] < 0x80 ? lpBuf[i] 
				: CodeTables[code][(BYTE)lpBuf[i] - 0x80];

			lpRecodeBuf[cbRecodeBuf] = 0;

			// set default font
			CHARFORMAT cf;
			if (!reg_getbin(PVAL_FONT, (BYTE *)&cf, sizeof(cf)))
			{
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_SIZE |
					CFM_CHARSET | CFM_FACE;
				cf.dwEffects = CFE_AUTOCOLOR;
				cf.yHeight = 200; // 10 pt
				cf.yOffset = 0;
				cf.bCharSet = DEFAULT_CHARSET;
				cf.bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
				lstrcpy(cf.szFaceName, "Courier New");
			}
			SendMessage(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

			// set rich edit text
			HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
			SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)lpRecodeBuf);
			VirtualFree(lpRecodeBuf, 0, MEM_RELEASE);
			SetCursor(hCursor);
		
			// set selection
			SendMessage(hEdit, EM_SETSEL, x, y);
			
			// set status bar 'code' text
			code = g_Data[indx].code;
			SendMessage(g_Data[indx].hBar, SB_SETTEXT, 1, (LPARAM)szCodeName[code]);

			lResult = 1;
		}
		break;

	case WM_SHOWCARETPOS:
		{
			int i = getindx(hWnd);
			if (i == -1)
				break;
			
			DWORD x, y, n;
			// get caret line
			HWND hEdit = g_Data[i].hEdit;
			DWORD dwPos;
			SendMessage(hEdit, EM_GETSEL, NULL, (LPARAM)&dwPos);
			x = SendMessage(hEdit, EM_LINEFROMCHAR, dwPos, 0) + 1;
			
			// get caret row
			y = -SendMessage(hEdit, EM_LINEINDEX, -1, 0) + dwPos + 1;

			// get line number
			n = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);

			// set status bar 'position' text
			char szText[100];
			wsprintf(szText, Message(MSG_COLANDROW), x, n, y);
			SendMessage(g_Data[i].hBar, SB_SETTEXT, 2, (LPARAM)szText);
		}
		break;

		// wParam = buf, lParam = buflen; return: 1 - OK, 0 - FAIL.
	case WM_LOADBUFFER: 
		{
			int indx = getindx(hWnd);
			if (indx == -1)
				break;

			BYTE *lpBuf = (BYTE *)wParam;
			DWORD cbBuf = lParam;

			HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

			DWORD dwNewLines = 0;
			for (DWORD i = 0; i < cbBuf; i++)
			{
				// count lines (ends by 0x0A[Linux] or  0x0D 0x0A  [Win])
				if (lpBuf[i] == '\n')
					dwNewLines++;

				// eliminate zero characters for use by WM_SETTEXT
				if (lpBuf[i] == 0)
					lpBuf[i] = ' ';
			}

			char *lpNewBuf = (char *)VirtualAlloc(NULL, cbBuf + dwNewLines + 1/*last zero*/,
				MEM_COMMIT, PAGE_READWRITE);

			if (!lpNewBuf)
			{
				g_Data[indx].lpBuf = NULL;
				g_Data[indx].cbBuf = 0;
				SetCursor(hCursor);
				lResult = 0;
				break;
			}

			// write text to a new buffer, all lines will end by 0x0D 0x0A
			DWORD k, ck, l;
			k = ck = l = 0;
			do
			{
				while (lpBuf[k] != 0x0A && k < cbBuf) 
					k++;

				// copy line
				DWORD n = k - ck;
				MoveMemory(lpNewBuf + l, lpBuf + ck, n);
				l += n;

				if (lpBuf[k] != 0x0A)
					break;

				if (k > 0)
				{
					if (lpBuf[k - 1] == 0x0D)
						lpNewBuf[l++] = 0x0A;
					else
					{
						lpNewBuf[l] = 0x0D;
						lpNewBuf[++l] = 0x0A;
						l++;
					}
				}
				else
				{
					lpNewBuf[0] = 0x0D;
					lpNewBuf[1] = 0x0A;
					l += 2;
				}
				ck = ++k;
			} while (k < cbBuf);

			g_Data[indx].lpBuf = lpNewBuf;
			g_Data[indx].cbBuf = l;

			SendMessage(g_Data[indx].hBar, SB_SETTEXT, 0,
				(LPARAM)size_to_spaced_string(cbBuf));

			SetCursor(hCursor);

			lResult = 1;
		}
		break;
		
		// wParam, lParam - not used; return value - not used.
	case WM_LOCATEWINDOW: 
		{
			int indx = getindx(hWnd);
			if (indx == -1)
				break;

			WINDOWPLACEMENT wndpl;

			BOOL bRes = reg_getbin(PVAL_VPLACEMENT, (BYTE *)&wndpl, sizeof(wndpl));
			if (!bRes)
				break;

			if (g_cDataNum == 1)
			{
				// first window...
				SetWindowPlacement(hWnd, &wndpl);
			}
			else
			{
				WINDOWPLACEMENT wndpl2, wndpl3;

				wndpl2.length = sizeof(wndpl2);
				GetWindowPlacement(hWnd, &wndpl2);

				RECT rect, *oldrect = &wndpl.rcNormalPosition; 
				rect.left = wndpl2.rcNormalPosition.left;
				rect.top  = wndpl2.rcNormalPosition.top;
				rect.right = rect.left + oldrect->right - oldrect->left;
				rect.bottom = rect.top + oldrect->bottom - oldrect->top;

				wndpl3.length = sizeof(wndpl3);
				wndpl3.flags = 0;
				wndpl3.showCmd = SW_RESTORE;
				wndpl3.ptMinPosition = wndpl2.ptMinPosition;
				wndpl3.ptMaxPosition = wndpl2.ptMaxPosition;
				wndpl3.rcNormalPosition = rect;

				SetWindowPlacement(hWnd, &wndpl3);
			}
		}
		break;

	case WM_SIZE:
		{
			int i = getindx(hWnd);
			if (i == -1)
				break;

			RECT rc;
			GetClientRect(hWnd, &rc);
			RECT rcs;
			GetClientRect(g_Data[i].hBar, &rcs);
			
			// resize rich edit
			MoveWindow(g_Data[i].hEdit,
				0, 0, 
				rc.right - rc.left, rc.bottom - rcs.bottom + rcs.top, 
				TRUE);
			
			// resize status bar
			SendMessage(g_Data[i].hBar, WM_SIZE, 0, 0);
			lResult = 0; // processed
		}
		//break; // fall through...to WM_MOVE

		/// DON'T PLACE 'case' HERE !

	case WM_MOVE:
		{
			int indx = getindx(hWnd);
			if (indx == -1)
				break;

			WINDOWPLACEMENT wndpl;
			wndpl.length = sizeof(wndpl);
			GetWindowPlacement(hWnd, &wndpl);
			g_Data[indx].wndpl = wndpl;
			lResult = 0; // processed
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR  pnmh = (LPNMHDR)lParam; 
			switch (pnmh->code)
			{
			case EN_MSGFILTER:
				{
					MSGFILTER *msgf = (MSGFILTER *)lParam;
					switch (msgf->msg)
					{
					case WM_KEYUP:
					case WM_LBUTTONUP:
						SendMessage(hWnd, WM_SHOWCARETPOS, 0, 0);
						break;

//					case WM_KEYDOWN:
//						if (msgf->wParam == VK_ESCAPE)
//							//DestroyWindow(hWnd);
//						break;

					case WM_RBUTTONDOWN:
						{
							POINT p;
							GetCursorPos(&p);

							HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
							HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_POPUPS));
							HMENU hPopup = GetSubMenu(hMenu, 2);
							RECT rc;
							TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
								p.x, p.y, 0, hWnd, &rc);
							DestroyMenu(hMenu);
						}
						break;
					}
					
					lResult = 0;
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
			case IDM_VIEW_EXIT:
				DestroyWindow(hWnd);
				break;

			case IDM_VIEW_FIND:
				{
					int indx = getindx(hWnd);
					if (indx == -1)
						break;

					char szFindText[MAX_FINDTEXT];
					FIND find;
					find.buf = szFindText;
					find.buflen = MAX_FINDTEXT;
					find.flags = g_Data[indx].dwFindOptions;
					if (create_finddlg(hWnd, &find))
					{
						// set search conditions
						g_Data[indx].dwFindOptions = find.flags;
						lstrcpyn(g_Data[indx].szFindText, szFindText, MAX_FINDTEXT);

						// search...
						if (lstrcmp(szFindText, ""))
							SendMessage(hWnd, WM_COMMAND, IDM_VIEW_FINDMORE, 0);

						SetFocus(g_Data[indx].hEdit);
					}
				}
				break;

			case IDM_VIEW_FINDMORE:
				{
					int indx = getindx(hWnd);
					if (indx == -1)
						break;

					if (!lstrcmp(g_Data[indx].szFindText, ""))
					{
						// search string was not defined
						SendMessage(hWnd, WM_COMMAND, IDM_VIEW_FIND, 0);
					}
					else
					{
						// set search option
						DWORD dwOptions = FR_DOWN;
						if (g_Data[indx].dwFindOptions & FF_CASESENSETIVE)
							dwOptions |= FR_MATCHCASE;
						if (g_Data[indx].dwFindOptions & FF_WHOLEWORDS)
							dwOptions |= FR_WHOLEWORD;

						HWND hEdit = g_Data[indx].hEdit;

						// get search start position
						LONG lStart;
						SendMessage(hEdit, EM_GETSEL, 0, (LPARAM)&lStart);

						// search...
						FINDTEXTEX ft;
						ft.lpstrText = g_Data[indx].szFindText;
						ft.chrg.cpMin = lStart;
						ft.chrg.cpMax = -1;
						LRESULT lFindRes = SendMessage(hEdit, EM_FINDTEXTEX, dwOptions, (LPARAM)&ft);
						if (lFindRes == -1)
						{
							char szText[MAX_FINDTEXT + 100];
							wsprintf(szText, "%s \"%s\"", Message(MSG_SAMPLENOTFOUND), g_Data[indx].szFindText);
							HWND hwndFocused = GetFocus();
							MessageBox(hWnd, szText, Message(MSG_STRINGNOTFOUND), MB_ICONEXCLAMATION | MB_OK );
							SetFocus(hwndFocused);
						}
						else
							SendMessage(hEdit, EM_SETSEL, ft.chrgText.cpMin, ft.chrgText.cpMax);
					}
				}
				break;

			case IDM_VIEW_COPY:
				{
					int indx = getindx(hWnd);
					if (indx == -1)
						break;
					SendMessage(g_Data[indx].hEdit, WM_COPY, 0, 0);
				}
				break;

			case IDM_VIEW_FONT:
				{
					int i = getindx(hWnd);
					if (i == -1)
						break;

					HWND hEdit = g_Data[i].hEdit;
					HDC hDC = GetDC(hEdit);

					// get current font
					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);
					DWORD lStart, lEnd;
					SendMessage(hEdit, EM_GETSEL, (WPARAM)&lStart, (LPARAM)&lEnd);
					SendMessage(hEdit, EM_HIDESELECTION, TRUE, 0);
					SendMessage(hEdit, EM_SETSEL, 0, -1);
					SendMessage(hEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);
					SendMessage(hEdit, EM_SETSEL, lStart, lEnd);
					SendMessage(hEdit, EM_HIDESELECTION, FALSE, 0);

					// try to select a new one
					LOGFONT font;
					font.lfHeight = -MulDiv(cf.yHeight/20, GetDeviceCaps(hDC, LOGPIXELSY), 72);
					font.lfCharSet = cf.bCharSet; // if DEFAULT_CHARSET it works only under NT 
					if (font.lfCharSet == DEFAULT_CHARSET)
						GetObject(GetStockObject(SYSTEM_FONT), sizeof(font), &font);
					font.lfPitchAndFamily = cf.bPitchAndFamily;
					font.lfItalic = (BYTE)(cf.dwEffects & CFE_ITALIC);
					if (cf.dwEffects & CFE_BOLD)
						font.lfWeight = FW_BOLD;
					lstrcpy(font.lfFaceName, cf.szFaceName);

					CHOOSEFONT cfont;
					ZeroMemory(&cfont, sizeof(cfont));
					cfont.lStructSize = sizeof(cf);
					cfont.hwndOwner = hWnd;
					cfont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
					cfont.lpLogFont = &font;


					if (ChooseFont(&cfont))
					{
						// set new font
						cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_SIZE |
							CFM_CHARSET | CFM_FACE;
						cf.dwEffects = CFE_AUTOCOLOR;
						if (font.lfItalic) 
							cf.dwEffects |= CFE_ITALIC;
						if (font.lfWeight == FW_BOLD)
							cf.dwEffects |= CFE_BOLD;
						cf.yHeight = -20*MulDiv(font.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
						cf.yOffset = 0;
						cf.bCharSet = font.lfCharSet;
						cf.bPitchAndFamily = font.lfPitchAndFamily;
						lstrcpy(cf.szFaceName, font.lfFaceName);

						// save current font to registry
						reg_setbin(PVAL_FONT, (BYTE *)&cf, sizeof(cf));

						SendMessage(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
					}

					ReleaseDC(hEdit, hDC);
				}
				break;

			case IDM_VIEW_SELECTALL:
				{
					int indx = getindx(hWnd);
					if (indx == -1)
						break;

					SendMessage(g_Data[indx].hEdit, EM_SETSEL, 0, -1);
				}
				break;

			case IDM_VIEW_CODE_CP1251:
				{
					int indx = getindx(hWnd);
					if (indx == -1)
						break;

					g_Data[indx].code = CODE_CP1251;
					if (!SendMessage(hWnd, WM_LOADRICHEDIT,  0, 0))
					{
						MessageBox(hWnd, Message(MSG_NOMEMORYFORVIEW), "", MB_OK | MB_ICONEXCLAMATION);
						DestroyWindow(hWnd);
					}
				}
				break;

			case IDM_VIEW_CODE_KOI8:
				{
					int indx = getindx(hWnd);
					if (indx == -1)
						break;

					g_Data[indx].code = CODE_KOI8;
					if (!SendMessage(hWnd, WM_LOADRICHEDIT,  0, 0))
					{
						MessageBox(hWnd, Message(MSG_NOMEMORYFORVIEW), "", MB_OK | MB_ICONEXCLAMATION);
						DestroyWindow(hWnd);
					}
				}
				break;
			}
		}
		break;

	case WM_INITMENUPOPUP:
		{
			int indx = getindx(hWnd);
			if (indx == -1)
				break;

			HMENU hPopupMenu = (HMENU)wParam;
			for (int i = 0; i < GetMenuItemCount(hPopupMenu); i++)
			{
				MENUITEMINFO mii;
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_SUBMENU;
				GetMenuItemInfo(hPopupMenu, i, TRUE, &mii);

				if (mii.hSubMenu)
					continue;

				UINT uNewItemState = MFS_ENABLED;
				switch (mii.wID)
				{
				case IDM_VIEW_CODE_CP1251:
					if (g_Data[indx].code == CODE_CP1251)
						uNewItemState = MFS_ENABLED | MFS_CHECKED;
					break;

				case IDM_VIEW_CODE_KOI8:
					if (g_Data[indx].code == CODE_KOI8)
						uNewItemState = MFS_ENABLED | MFS_CHECKED;
					break;

				}

				mii.cbSize = sizeof(mii); //!
				mii.fMask = MIIM_STATE;
				mii.fState = uNewItemState;
				SetMenuItemInfo(hPopupMenu, i, TRUE, &mii);
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(hWnd);
		lResult = 0;
		break;

	case WM_DESTROY:
		{
			int indx = getindx(hWnd);
			if (indx == -1)
				break;

			// free text buffer
			VirtualFree(g_Data[indx].lpBuf, 0, MEM_RELEASE);
			g_Data[indx].lpBuf = NULL;

			// save window position
			reg_setbin(PVAL_VPLACEMENT, (BYTE *)&g_Data[indx].wndpl, sizeof(g_Data[indx].wndpl));

			SetWindowText(g_Data[indx].hEdit, "");
			// mark global data free
			g_Data[indx].bEmpty = TRUE;
			g_cDataNum--;

			lResult = 0; // processed
		}
		break;

	default:
		lResult = DefWindowProc(hWnd, message, wParam, lParam);
	}

	return lResult;
}

////////////////////////////////////////////////////////////////////////////////

void create_view(explorer *e, BYTE *buf, DWORD buflen, char *filename)
{
	if (g_cDataNum == 0)
	{
		for (int i = 0; i < MAX_SHOWDLGS; i++)
			g_Data[i].bEmpty = TRUE;
	}

	int indx = -1;
	for (int i = 0; i < MAX_SHOWDLGS; i++)
	{
		if (g_Data[i].bEmpty)
		{
			indx = i;
			g_cDataNum++;
			break;
		}
	}

	if (indx == -1)
	{
		MessageBox(e->hwndMainWindow, Message(MSG_TOOMANYVIEWS), "", MB_OK);
		return;
	}
	
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(e->hwndMainWindow, GWL_HINSTANCE);

	char szWindowClass[] = "ext2ViewWindow";

	if (!bRegWin)
	{
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(wcex); 
		wcex.style			= 0;
		wcex.lpfnWndProc	= (WNDPROC)ViewWndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_INTERF);
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= szWindowClass;
		wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
		
		if (!RegisterClassEx(&wcex))
			return;
		bRegWin = TRUE;
	}

	char szTitle[500];
	lstrcpy(szTitle, Message(MSG_VIEWTITLE));
	lstrcat(szTitle, filename);

	HWND hView = CreateWindow(szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL,
		LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_VIEW)),
		hInstance, NULL);
	
	if (!hView)
		return;
	
	g_Data[indx].bEmpty = FALSE;
	g_Data[indx].cbBuf = 0;
	g_Data[indx].lpBuf = NULL;
	g_Data[indx].hView = hView;
	g_Data[indx].code = CODE_KOI8;
	g_Data[indx].dwFindOptions = 0;
	lstrcpy(g_Data[indx].szFindText, "");
	
	SendMessage(hView, WM_FINALCREATE, 0, 0);
	
	g_Data[indx].hEdit = GetDlgItem(hView, IDC_VIEW_RICHEDIT);
	g_Data[indx].hBar = GetDlgItem(hView, IDC_VIEW_STATUSBAR);

	SendMessage(hView, WM_LOCATEWINDOW, 0, 0);

	ShowWindow(hView, SW_SHOW);
	UpdateWindow(hView);
	
	if (!SendMessage(hView, WM_LOADBUFFER, (WPARAM)buf, (LPARAM)buflen))
	{
		MessageBox(hView, Message(MSG_NOMEMORYFORVIEW), "", MB_OK | MB_ICONEXCLAMATION);
		DestroyWindow(hView);
		return;
	}

	if (!SendMessage(hView, WM_LOADRICHEDIT,  0, 0))
	{
		MessageBox(hView, Message(MSG_NOMEMORYFORVIEW), "", MB_OK | MB_ICONEXCLAMATION);
		DestroyWindow(hView);
		return;
	}

	SendMessage(hView, WM_SHOWCARETPOS, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
