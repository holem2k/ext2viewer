#ifndef __RES_H__
#define __RES_H__
#include "resource.h"

const DWORD ID_FILEBAR_LV = 5000;
const DWORD ID_DESKBAR_TV = 5001;
const DWORD ID_CAPTIONTOOLBAR = 5003;
const DWORD IDC_STATUSBAR = 5004;

const int DISKBAR_CONST1 = 2;
const int DISKBAR_CONST2 = 20;
const int SPLITTER_WIDTH = 3;
const int SPL_WIN_MIN_SIZE = 30; 

const int DiskbarIconsNum = 6;
const int DiskbarIcons[DiskbarIconsNum] = {IDI_COMPUTER_16, IDI_FOLDER_16, IDI_OFOLDER_16, IDI_FD3_5_16, IDI_FD5_25_16, IDI_HDD_16};
enum DiskbarIconsEnum {ICO_DB_COMPUTER, ICO_DB_FOLDER, ICO_DB_OFOLDER, ICO_DB_FD3_5, ICO_DB_FD5_25, ICO_DB_HDD};

const int FilebarIconsNum = 10;
const int FilebarIcons[FilebarIconsNum] = {IDI_FD3_5, IDI_FD5_25, IDI_HDD, IDI_FOLDER,  IDI_REGFILE, IDI_CHARFILE, IDI_BLOCKFILE, IDI_FIFOFILE, IDI_LNKFILE, IDI_SOCKFILE};
const int FilebarIconsSmall[FilebarIconsNum] = {IDI_FD3_5_16, IDI_FD5_25_16, IDI_HDD_16, IDI_FOLDER_16,  IDI_REGFILE_16, IDI_CHARFILE_16, IDI_BLOCKFILE_16, IDI_FIFOFILE_16, IDI_LNKFILE_16, IDI_SOCKFILE_16};
enum FilebarIconsEnum {ICO_FB_FD3_5, ICO_FB_FD5_25, ICO_FB_HDD, ICO_FB_FOLDER, ICO_FB_REGFILE, ICO_FB_CHARFILE, ICO_FB_BLOCKFILE, ICO_FB_FIFOFILE, ICO_FB_LNKFILE, ICO_FB_SOCKFILE};

const char lpApplicationName[] = "ext2viewer";

const DWORD MAX_STATUS = 200;

// resistry constants
const HKEY hHive = HKEY_CURRENT_USER;
const char lpSubKey[256] = "Software\\ext2viewer\\0.6";

const DWORD MAXFILENAME = 32768;

const char PVAL_PATH[]      = "Path";
const char PVAL_STYLE[]     = "Style";
const char PVAL_SORTBY[]    = "SortBy";
const char PVAL_STATUS[]    = "Status";
const char PVAL_FONT[]      = "Font";
const char PVAL_VHISTORY[]  = "Viewer History";
const char PVAL_VPLACEMENT[]  = "Viewer Placement";

const char DLL_DIONT[] = "diont.dll";
const char DLL_DIO9x[] = "dio9x.dll";
const char DLL_DIO9x16[] = "dio9x16.dll";

const DWORD SLOW_DEVICE_CHUNK = 1024*1024*2; // 2 MB
const DWORD FAST_DEVICE_CHUNK = 1024*64;     // 64 KB

#endif //__RES_H__
