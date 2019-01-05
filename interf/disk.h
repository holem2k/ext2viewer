#ifndef __DISK_H__
#define __DISK_H__

const int MAX_PARTITION_NAME = 10;

typedef struct tagPartitionInfo
{
	unsigned long start;	
	unsigned long length;
	unsigned char type;
	char part_name[MAX_PARTITION_NAME]; 
} PartitionInfo;


const unsigned char PARTITION_ENTRY_UNUSED    = 0x00; // Entry unused
const unsigned char PARTITION_FAT_12          = 0x01; // 12-bit FAT entries
const unsigned char PARTITION_XENIX_1         = 0x02; // Xenix
const unsigned char PARTITION_XENIX_2         = 0x03; // Xenix
const unsigned char PARTITION_FAT_16          = 0x04; // 16-bit FAT entries
const unsigned char PARTITION_EXTENDED        = 0x05; // *Extended partition entry
const unsigned char PARTITION_HUGE            = 0x06; // Huge partition MS-DOS V4
const unsigned char PARTITION_IFS             = 0x07; // IFS Partition
const unsigned char PARTITION_FAT32           = 0x0B; // FAT32
const unsigned char PARTITION_FAT32_XINT13    = 0x0C; // FAT32 using extended int13 services
const unsigned char PARTITION_XINT13          = 0x0E; // Win95 partition using extended int13 services
const unsigned char PARTITION_XINT13_EXTENDED = 0x0F; // *Same as type 5 but uses extended int13 services
const unsigned char PARTITION_PREP            = 0x41; // PowerPC Reference Platform (PReP) Boot Partition
const unsigned char PARTITION_LDM             = 0x42; // Logical Disk Manager partition
const unsigned char PARTITION_UNIX            = 0x63; // Unix
const unsigned char PARTITION_LINUX           = 0x83; // Linux
const unsigned char PARTITION_LINUXSWAP       = 0x82; // Linux swap
const unsigned char PARTITION_LINUX_EXT       = 0x85; // *Linux extended

const DWORD PARTITION_MAGIC = 0xAA55;
const DWORD PARTITION_MAGIC_MAGIC = 0xF6;

int disk_init();
int disk_done();
int disk_get_partitions(BYTE drive, PartitionInfo *part_info, int part_num);
WORD disk_read(BYTE bDrive,
		  DWORD dwSector,
		  WORD wSectorNumber,
		  LPBYTE lpBuffer,
		  DWORD  cbBuffSize);
WORD disk_check(BYTE bDrive);

const BYTE DISK_FD0 = 0x0;
const BYTE DISK_FD1 = 0x1;
const BYTE DISK_HDA = 0x80;
const BYTE DISK_HDB = 0x81;
const BYTE DISK_HDC = 0x82;
const BYTE DISK_HDD = 0x83;

const DWORD DISK_NOTEXIST     = 0;
const DWORD DISK_EXIST        = 1;
const DWORD DISK_ACCESSDENIED = 2;
#endif //__DISK_H__