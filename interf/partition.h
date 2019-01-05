#ifndef __PARTITION_H__
#define __PARTITION_H__
#include "disk.h"

#pragma pack(push, __partition_h__)



const unsigned short MAGIC = 0xAA55;

#pragma pack(1)
typedef struct tagCHS
{
	unsigned char head;
	unsigned char sector;
	unsigned char cylinder;
} CHS;


#pragma pack(1)
typedef struct tagPartition
{
	unsigned char boot_ind;
	CHS chs_start;
	unsigned char type;
	CHS chs_end;
	unsigned long start;
	unsigned long length;
} Partition;



#pragma pack(1)
typedef struct tagPartitionTable
{
	unsigned char boot_code[446];
	Partition partitions[4];
	unsigned short magic;
} PartitionTable;


#pragma pack(pop, __partition_h__)

int is_partition_valid(unsigned char type);
int is_partition_extended(unsigned char type);

int	enum_extended_partition(int drive,
							unsigned long start,
							PartitionInfo *partitions_info,
							int part_num,
							int &part_found_num,
							int &ext_part_num);


typedef WORD (* readfunc)(BYTE bDrive,
		DWORD dwSector,
		WORD wSectorNumber,
		LPBYTE lpBuffer,
		DWORD  cbBuffSize);
typedef	WORD (* checkfunc)(BYTE bDrive);
typedef WORD (* writefunc)();


typedef struct tagdisk_op
{
	readfunc read;
	checkfunc check;
	writefunc write; // not realized;
} disk_op;


//const char lpDirectDiskAccessLib[2][255] = {"dio9x.dll", "diont.dll"};


#endif //__PARTITION_H__