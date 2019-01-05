#ifndef __DIO9X_H__
#define __DIO9X_H__
#include "stdafx.h"

#ifdef DIO9X_EXPORTS
#define DLLE_I __declspec(dllexport)
#else
#define DLLE_I __declspec(dllimport)
#endif

extern "C" {

	__declspec(dllexport) WORD dioRead(BYTE bDrive,
		DWORD dwSector,
		WORD wSectorNumber,
		LPBYTE lpBuffer,
		DWORD  cbBuffSize); // 32-bit proxy function
	
	__declspec(dllexport) WORD dioCheck(BYTE bDrive);

	WORD WINAPI Read(BYTE bDrive,
		DWORD dwSector,
		WORD wSectorNumber,
		LPBYTE lpBuffer,
		DWORD  cbBuffSize); // 16-bit Dll function

	
	__declspec(dllexport) BOOL WINAPI thk_ThunkConnect32(LPSTR lpDll16,
		LPSTR     lpDll32,
		HINSTANCE hDllInst,
		DWORD     dwReason
		); // function from thunk
	
}
#endif //__DIO9X_H__