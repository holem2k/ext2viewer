#include <stdafx.h>
#include "finddlg.h"
#include "registry.h"

/////////////////////////////////////////////////////////////////////////////

const DWORD HISTORY_SIZE   = 5;
const DWORD HISTORY_BINLEN = HISTORY_SIZE*MAX_FINDTEXT + MAX_FINDTEXT + 10;

/////////////////////////////////////////////////////////////////////////////

static FIND *g_find;

/////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK FindDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hwndDlg, IDC_FIND_COMBO1), CB_LIMITTEXT, MAX_FINDTEXT - 1, 0);

		// load 'find history' to combobox
		BYTE lpHistory[HISTORY_BINLEN];
		if (reg_getbin(PVAL_VHISTORY, lpHistory, HISTORY_BINLEN))
		{
			char szHistoryItem[MAX_FINDTEXT];
			lstrcpy(szHistoryItem, "");

			DWORD i, k;
			i = k = 0;
			while (i < HISTORY_BINLEN)
			{
				while (lpHistory[i]  && i < HISTORY_BINLEN)
					i++;

				if (lpHistory[i] == 0)
				{
					if (i == k)
						break; // second last zero;

					DWORD dwLen = i - k;
					if (dwLen > MAX_FINDTEXT - 1)
						dwLen = MAX_FINDTEXT - 1;
					MoveMemory(szHistoryItem, lpHistory + k, dwLen);
					szHistoryItem[dwLen] = 0;
					SendMessage(GetDlgItem(hwndDlg, IDC_FIND_COMBO1), CB_INSERTSTRING, 0, (WPARAM)szHistoryItem);
					k = ++i;
				}
			}

			SendMessage(GetDlgItem(hwndDlg, IDC_FIND_COMBO1), WM_SETTEXT, 0, (WPARAM)szHistoryItem);
			SendMessage(GetDlgItem(hwndDlg, IDC_FIND_COMBO1), CB_SETEDITSEL, 0, -1);
			if (g_find->flags & FF_CASESENSETIVE)
				SendMessage(GetDlgItem(hwndDlg, IDC_FIND_CHECK1), BM_SETCHECK, 1, 0);
			if (g_find->flags & FF_WHOLEWORDS)
				SendMessage(GetDlgItem(hwndDlg, IDC_FIND_CHECK2), BM_SETCHECK, 1, 0);
		}

		bResult = TRUE;
		break;
		
	case WM_COMMAND:
		{
			switch (wParam)
			{
			case IDOK:
				{
					// set FIND structure 
					g_find->flags = 0;

					LRESULT lState;
					lState = SendMessage(GetDlgItem(hwndDlg, IDC_FIND_CHECK1), BM_GETCHECK, 0, 0);
					if (lState == BST_CHECKED)
						g_find->flags |= FF_CASESENSETIVE;

					lState = SendMessage(GetDlgItem(hwndDlg, IDC_FIND_CHECK2), BM_GETCHECK, 0, 0);
					if (lState == BST_CHECKED)
						g_find->flags |= FF_WHOLEWORDS;

					if (GetDlgItemText(hwndDlg, IDC_FIND_COMBO1, g_find->buf, g_find->buflen))
					{
						// save 'find history'
						BYTE lpHistory[HISTORY_BINLEN];
						DWORD cbLen = 0;
						DWORD dwCount = SendMessage(GetDlgItem(hwndDlg, IDC_FIND_COMBO1), CB_GETCOUNT, 0, 0);
						if (dwCount > HISTORY_SIZE - 1)
							dwCount = HISTORY_SIZE - 1;

						for (DWORD i = 0; i < dwCount; i++)
						{
							char szHistoryItem[MAX_FINDTEXT];
							SendMessage(GetDlgItem(hwndDlg, IDC_FIND_COMBO1), CB_GETLBTEXT, dwCount - i - 1, (LPARAM)szHistoryItem);
							if (lstrcmp(szHistoryItem, g_find->buf))
							{
								MoveMemory(lpHistory + cbLen, szHistoryItem, lstrlen(szHistoryItem));
								cbLen += lstrlen(szHistoryItem);
								lpHistory[cbLen++] = 0;
							}
						}
						MoveMemory(lpHistory + cbLen, g_find->buf, lstrlen(g_find->buf));
						cbLen += lstrlen(g_find->buf);
						lpHistory[cbLen++] = 0;
						lpHistory[cbLen++] = 0; // second ending zero

						reg_setbin(PVAL_VHISTORY, lpHistory, cbLen);
						EndDialog(hwndDlg, 1);
					}
					else
						EndDialog(hwndDlg, 0);

					bResult = TRUE;
				}
				break;

			case IDCANCEL:
				//g_find->flags = 0;
				EndDialog(hwndDlg, 0);
				bResult = TRUE;
				break;
			}
		}
		break;
	}
	return bResult;
}		

/////////////////////////////////////////////////////////////////////////////

BOOL create_finddlg(HWND hParent, FIND *find)
{
	g_find = find;
	return DialogBox((HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE),
		MAKEINTRESOURCE(IDD_DIALOG_FIND),
		hParent,
		FindDialogProc);
}

/////////////////////////////////////////////////////////////////////////////