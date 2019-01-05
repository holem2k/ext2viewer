// dio9x.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "dio9x.h"



////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hDLLInst,
		DWORD     dwReason,
		LPVOID    lpvReserved)
{
	
	BOOL bResult = TRUE;
	if (!thk_ThunkConnect32("dio9x16.dll", "dio9x.DLL",
		hDLLInst, dwReason))
	{
		bResult = FALSE;
	}

    return bResult;
}

////////////////////////////////////////////////////////////////

WORD  dioRead(BYTE bDrive,
			  DWORD dwSector,
			  WORD wSectorNumber,
			  LPBYTE lpBuffer,
			  DWORD  cbBuffSize)
{
	return Read(bDrive,
		dwSector,
		wSectorNumber,
		lpBuffer,
		cbBuffSize);
}


////////////////////////////////////////////////////////////////
// 0 - диск отсутствует
// 1 - диск присутствует

WORD dioCheck(BYTE bDrive)
{
	WORD result = 0;
	if (bDrive == 0x0/*A*/) result = 1;
	else if (bDrive == 0x1/*B*/) result = 0;
	else if (bDrive >= 0x80  && bDrive <= 0x83/*HDD*/)
	{
		BYTE buffer[512];
		result = Read(bDrive, 0, 1, buffer, 512);
	}
	return result;
}

////////////////////////////////////////////////////////////////
