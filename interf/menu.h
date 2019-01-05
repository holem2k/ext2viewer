#ifndef __CONTROLS_H__
#define __CONTROLS_H__

HWND CreateCoolMenu(HWND hwndParent, HINSTANCE hInst, DWORD MenuId);
void CoolMenuRecalc();
BOOL PreProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND CreateStatusBar(HWND hwndParent, HINSTANCE hInst);

#endif //__CONTROLS_H__