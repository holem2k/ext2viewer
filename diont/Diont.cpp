// diont.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "diont.h"

////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	int i;
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			for (i = 0; i < MAXDISKNUM; i++)
				hDiskHandle[i] = INVALID_HANDLE_VALUE;
			break;

		case DLL_PROCESS_DETACH:
			for (i = 0; i < MAXDISKNUM; i++)
				if (hDiskHandle[i] != INVALID_HANDLE_VALUE)
					CloseHandle(hDiskHandle[i]);
			break;

		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////

WORD dioRead(BYTE bDrive,
			 DWORD dwSector,
			 WORD wSectorNumber,
			 LPBYTE lpBuffer,
			 DWORD  cbBuffSize)
{
	if (SECTOR_SIZE*wSectorNumber < cbBuffSize) return 0;

	WORD  Result = 0;
	DWORD  dwDriveIndex = bDrive >= 0x80 ? bDrive - 0x80 + 0x02 : bDrive;

	if (hDiskHandle[dwDriveIndex] == INVALID_HANDLE_VALUE)
	{
		const CHAR *lpFileName = (CHAR *)Drives[dwDriveIndex];
		
		hDiskHandle[dwDriveIndex] = CreateFile(lpFileName,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	}

	HANDLE hFile = hDiskHandle[dwDriveIndex];

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD   NumberOfBytesRead;
		__int64 iPosition = (__int64)dwSector*SECTOR_SIZE;
		LARGE_INTEGER li;
		li.QuadPart = iPosition;
		li.LowPart = SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
		if (li.LowPart == -1 && GetLastError() != NO_ERROR)
		{
			// it looks clumsy but works faster^
		} else	if (ReadFile(hFile, lpBuffer, wSectorNumber*SECTOR_SIZE, &NumberOfBytesRead, NULL)) Result = 1;

		//CloseHandle(hFile);
	}
	return Result;
}

////////////////////////////////////////////////////////////////////////////
// 0 - устройство отсутствует
// 1 - устройство присутствует
// 2 - устройство присутствует, доступ запрещен

WORD dioCheck(BYTE bDrive)
{
	WORD  Result = 0;
	DWORD  dwDriveIndex = bDrive >= 0x80 ? bDrive - 0x80 + 0x2 : bDrive;
	const CHAR *lpFileName = (CHAR *)Drives[dwDriveIndex];
	DWORD dwAccess = bDrive >= 0x80 ? GENERIC_READ : 0;

	HANDLE hFile = CreateFile(lpFileName,
		dwAccess,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == 5) Result = 2;
	}
	else
	{
		CloseHandle(hFile);
		Result = 1;
	}
	
	return Result;
}

////////////////////////////////////////////////////////////////////////////