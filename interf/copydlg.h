#ifndef __COPYDLG_H__
#define __COPYDLG_H__
#include "explorer.h"


typedef struct tagThreadData
{
	DWORD (*lpfThreadFunc)(void *lpData); // copy function 
	void *lpData; // pointer to data for this function
} ThreadData;


const DWORD DLG_STATE_NORMAL  = 0;
const DWORD DLG_STATE_VISIBLE = 1;
const DWORD DLG_STATE_HIDDEN  = 2;


void CreateCopyDlg(explorer *e, ThreadData *lpThreadData, volatile DWORD *lpThreadEndFlag, DWORD *dwResult);
void SetCopyDlgData(char *lpLinuxName, char *lpWindowsName, DWORD dwNumOfFiles,
					DWORD dwPercent, DWORD dwState);

#endif //__COPYDLG_H__