
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DIONT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DIONT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DIONT_EXPORTS
#define DIONT_API __declspec(dllexport)
#else
#define DIONT_API __declspec(dllimport)
#endif

const DWORD SECTOR_SIZE = 512;
const DWORD MAXDISKNUM = 6;
const CHAR Drives[MAXDISKNUM][50] = 
{
	"\\\\.\\A:",
		"\\\\.\\B:",
		"\\\\.\\PHYSICALDRIVE0",
		"\\\\.\\PHYSICALDRIVE1",
		"\\\\.\\PHYSICALDRIVE2",
		"\\\\.\\PHYSICALDRIVE3"
};


HANDLE hDiskHandle[MAXDISKNUM];

extern "C"
{
	DIONT_API WORD dioRead(BYTE bDrive,
		DWORD dwSector,
		WORD wSectorNumber,
		LPBYTE lpBuffer,
		DWORD  cbBuffSize); 

	DIONT_API WORD dioCheck(BYTE bDrive);

}
