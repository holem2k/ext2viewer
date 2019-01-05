#include <windows.h>
#include "dio9x16.h"


////////////////////-Thunk entry point-///////////////////////////////////////

#pragma argused
BOOL WINAPI __export DllEntryPoint(DWORD dwReason,
									WORD  hInst,
									WORD  wDS,
									WORD  wHeapSize,
									DWORD dwReserved1,
									WORD  wReserved2)
{
	return thk_ThunkConnect16("dio9x16.dll", "dio9x.dll", hInst, dwReason);
}

//////////////////////-entry point-///////////////////////////////////////////


WORD FAR PASCAL _export Read(BYTE bDrive,
							 DWORD dwSector,
							 WORD wSectorNumber,
							 LPBYTE lpBuffer,
							 DWORD  cbBuffSize)
{
	BOOL   bResult;
	RMCS   callStruct;

	DWORD  gdaBuffer;
	LPBYTE RMlpBuffer;
	LPBYTE PMlpBuffer;

	DWORD  gdaPacket;
	disk_packet *RMlpPacket;
	disk_packet *PMlpPacket;

	if (lpBuffer == NULL || cbBuffSize < SECTOR_SIZE*wSectorNumber)	return FALSE;

	gdaBuffer = GlobalDosAlloc (cbBuffSize);
	if (!gdaBuffer) return FALSE;

	RMlpBuffer = (LPBYTE)MAKELONG(0, HIWORD(gdaBuffer));
	PMlpBuffer = (LPBYTE)MAKELONG(0, LOWORD(gdaBuffer));

	gdaPacket = GlobalDosAlloc (sizeof(disk_packet));
	if (!gdaPacket)	return FALSE;
	RMlpPacket = (disk_packet *)MAKELONG(0, HIWORD(gdaPacket));
	PMlpPacket = (disk_packet *)MAKELONG(0, LOWORD(gdaPacket));

	PMlpPacket->packet_size = 16;
	PMlpPacket->reserved = 0;
	PMlpPacket->block_count = wSectorNumber;
	PMlpPacket->block_num_lo = dwSector;
	PMlpPacket->block_num_hi = 0;
	PMlpPacket->buffer_addr  = (unsigned long)RMlpBuffer;


	BuildRMCS (&callStruct);
	callStruct.eax = 0x4200;
	callStruct.edx = bDrive;
	callStruct.esi = LOWORD(RMlpPacket);
	callStruct.ds  = HIWORD(RMlpPacket);

	if ((bResult = SimulateRM_Int(0x13, &callStruct)) == TRUE)
	{
		if (!(callStruct.wFlags & CARRY_FLAG))
		{
			 MemMove(lpBuffer, PMlpBuffer, cbBuffSize);
		}
		else		bResult = FALSE;
	}


	GlobalDosFree (LOWORD(gdaBuffer));
	GlobalDosFree (LOWORD(gdaPacket));

	return bResult;
}

//////////////////////-entry point-///////////////////////////////////////////

int  FAR PASCAL _export Check()
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FAR PASCAL BuildRMCS (LPRMCS lpCallStruct)
{
	ZeroMem(lpCallStruct, sizeof(RMCS));
}

///////////////////////////////////////////////////////////////////////////////

BOOL FAR PASCAL SimulateRM_Int(BYTE bIntNum, LPRMCS lpCallStruct)
{
	BOOL fRetVal = FALSE;

	__asm {
			push di

			mov  ax, 0x0300; 	//	DPMI Simulate Real Mode Int
			mov  bl, bIntNum;	// Number of the interrupt to simulate
			mov  bh, 0x01;    // Bit 0 = 1; all other bits must be 0
			xor  cx, cx; 		//	No words to copy
			les  di, lpCallStruct
			int  0x31; 			//	Call DPMI
			jc   END1; 			//	CF set if error occurred
			mov  fRetVal, TRUE
		END1:
			pop di
		  }

	return fRetVal;
}

///////////////////////////////////////////////////////////////////////////////

void MemMove(void *lpDest, void *lpSrc, WORD size)
{
	for (int i = 0; i<size; i++)
	 ((BYTE *)lpDest)[i] = ((BYTE *)lpSrc)[i];
}

///////////////////////////////////////////////////////////////////////////////

void ZeroMem(void *lpDest, WORD size)
{
	for (int i = 0; i<size; i++)
	 ((BYTE *)lpDest)[i] = 0;
}

///////////////////////////////////////////////////////////////////////////////
