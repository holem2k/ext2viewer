#ifndef __COPYCORE_H__
#define __COPYCORE_H__
#include "ext2_fs.h"
#include "explorer.h"

const DWORD RAWCOPY_OK      = 0x00;
const DWORD RAWCOPY_FAIL    = 0x01;
const DWORD RAWCOPY_REFRESH = 0x02;

const DWORD COPY_ASK            = 0x01;
const DWORD COPY_SKIP_ALL       = 0x02;
const DWORD COPY_OVERWRITE_ALL  = 0x04; 


typedef struct tag_dirname
{
    char data[EXT2_FILENAME_LEN];
} dirname;


typedef struct tag_dirinfo
{
    DWORD cur;
    DWORD numdirs;
    dirname *data;
    DWORD *indx;
    tag_dirinfo *updir;
} dirinfo;

typedef void ( *LPFNSETDLGDATA)(char *, char *, DWORD, DWORD, DWORD);

typedef struct tag_copyfilepar
{
    explorer *e;
	BYTE si;
	char *src;
	char *dest;
	common_file *file;

	BYTE *buf;      //  дисковый буфер
	DWORD buflen;   //  размер дискового буфера

	volatile DWORD *endflag;
	LPFNSETDLGDATA lpfnSetCopyDlgData;
} copyfilepar;


typedef struct tag_copyfilespar
{
    explorer *e;
    BYTE si;
	char *srcdir;
	char *destdir;
    common_file *files;
    DWORD *indx;
    DWORD numfiles;

	BYTE *buf;     // дисковый буфер
	DWORD buflen;  // размер дискового буфера

	volatile DWORD *endflag;
	LPFNSETDLGDATA lpfnSetCopyDlgData;
} copyfilespar;



#define USE_LONGNAMES \
    static int on_nt = -1; \
    if (on_nt == -1) \
    { \
        OSVERSIONINFO osvi; \
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); \
        GetVersionEx(&osvi); \
        on_nt = osvi.dwPlatformId == VER_PLATFORM_WIN32_NT ? 1 : 0; \
    }
 
#define CALL_LONGNAME_PROC(proc_name, par_list, disk_name, buflen, result) \
    if (on_nt == 1) \
    { \
        CHAR ntdisk_name[buflen]; \
        lstrcpyA(ntdisk_name, "\\\\?\\"); \
        lstrcatA(ntdisk_name, disk_name); \
        WCHAR buffer[buflen]; \
        MultiByteToWideChar(CP_ACP, 0, ntdisk_name, -1, buffer, buflen); \
        { \
           WCHAR disk_name[buflen]; \
           lstrcpyW(disk_name, buffer); \
           result = proc_name##W##par_list; \
        } \
    } \
    else  \
        result = proc_name##par_list;



BOOL getfilename(HWND hwndOwner, char *buf, DWORD buflen);
BOOL getdirname(HWND hwndOwner, char *buf, DWORD buflen);

void allocbuffer(BYTE *&buffer, DWORD &buflen, BOOL removable);
void freebuffer(BYTE *buffer);

dirinfo *create_rootdirinfo(DWORD *indx);
dirinfo *create_dirinfo(dirinfo *updir, common_file *files, DWORD *indx, DWORD numfiles);
void kill_dirinfo(dirinfo *info);

void build_reldir(char *buf, dirinfo *info);
void build_windir(const char *dir, const char *reldir, char *buf);
void build_lindir(const char *dir, const char *reldir, char *buf);
void build_linfile(const char *dir, const char *file, char *buf);
void build_winfile(const char *dir, const char *file, char *buf);
BOOL create_dir(char *directory);
BOOL file_exist(char *file_name);
DWORD rawcopy(explorer *e, BYTE si, char *src,  common_file *file,
			  char *dest, BYTE *buf, DWORD buflen, 
			  volatile DWORD *endflag,
			  LPFNSETDLGDATA lpfnSetCopyDlgData, DWORD fileprocesed);


#endif //__COPYCORE_H__