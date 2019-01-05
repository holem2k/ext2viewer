#ifndef _DIO9X16_H_
#define _DIO9X16_H_

#define SECTOR_SIZE 512
#define CARRY_FLAG 0x0001
#define MAKEWORD(low, high) \
			  ((WORD)((((WORD)(high)) << 8) | ((BYTE)(low))))

typedef struct tagRMCS
{
	DWORD edi, esi, ebp, RESERVED, ebx, edx, ecx, eax;
	WORD  wFlags, es, ds, fs, gs, ip, cs, sp, ss;
} RMCS, FAR* LPRMCS;


typedef struct tag_disk_packet
{
	unsigned char packet_size;
	unsigned char reserved;
	int  block_count;
	unsigned long buffer_addr;
	unsigned long block_num_lo;
	unsigned long block_num_hi;
} disk_packet;


void FAR PASCAL BuildRMCS (LPRMCS lpCallStruct);
BOOL FAR PASCAL SimulateRM_Int(BYTE bIntNum, LPRMCS lpCallStruct);

void MemMove(void *lpDest, void *lpSrc, WORD size);
void ZeroMem(void *lpDest, WORD size);

extern "C"
{
	BOOL WINAPI __export DllEntryPoint(DWORD dwReason,
		WORD  hInst,
		WORD  wDS,
		WORD  wHeapSize,
		DWORD dwReserved1,
		WORD  wReserved2);
	
	BOOL FAR PASCAL _export thk_ThunkConnect16(LPSTR lpDll16,
		LPSTR lpDll32,
		WORD  hInst,
		DWORD dwReason);
	
	WORD FAR PASCAL _export Read(BYTE bDrive,
		DWORD dwSector,
		WORD wSectorNumber,
		LPBYTE lpBuffer,
		DWORD  cbBuffSize);
		
		int  FAR PASCAL _export Check();
	
}


#endif
