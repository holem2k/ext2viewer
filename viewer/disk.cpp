#include "stdafx.h"
#include "partition.h"
#include "disk.h"
#include "assert.h"

////////////////////////////////////////////////////////////////////////


static HMODULE hDiskioDll = 0;
static int ref_count = 0;
static disk_op op;

////////////////////////////////////////////////////////////////////////

int disk_init()
{
	int result = 1; /* assume OK*/
	if (!ref_count) /* if not loaded yet*/
	{
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (!GetVersionEx(&osvi)) return 0;
		
		char *lpLibrary;
		
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			lpLibrary = (char *)lpDirectDiskAccessLib[0];
		}
		else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			lpLibrary = (char *)lpDirectDiskAccessLib[1];
		}
		else return 0;
		
		
		hDiskioDll = LoadLibrary(lpLibrary);
		assert(hDiskioDll);
		
		result = 1;
		if (hDiskioDll)
		{
			if (!(op.read = (readfunc)GetProcAddress(hDiskioDll, "dioRead")))
			{
				result = 0;
			}
			assert(op.read);
			op.write = NULL;
			if (!(op.check = (checkfunc)GetProcAddress(hDiskioDll, "dioCheck")))
			{
				result = 0;
			}
			assert(op.check);
		}
		else result = 0; 
	}


	if (result) ref_count++;
	return result;
}

////////////////////////////////////////////////////////////////////////

int disk_done()
{
	if (--ref_count == 0)
	{
		FreeLibrary(hDiskioDll);
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////

int disk_get_partitions(BYTE drive, PartitionInfo *partitions_info, int part_num)
{
	assert(drive >= 0x80);
	PartitionTable part_table;
	int result;

	result = op.read(drive, 0, 1, (unsigned char *)&part_table, sizeof(part_table));
	if (!result) return 0;
	
	int ext_part_num = 5;   // first logical disk number - hdx5
	int part_found_num = 0; // число найденных разделов
	result = 1;

	for (int i = 0; i < 4 && result; i++)
	{
		unsigned char type = part_table.partitions[i].type;
		if (is_partition_valid(type))
		{
			if (is_partition_extended(type))
			{
				if (!enum_extended_partition(drive, part_table.partitions[i].start,
					partitions_info, part_num, part_found_num, ext_part_num))
				{
					result = 0;
					break;
				}
			}
			else
			{
				PartitionInfo part_info;
				part_info.length = part_table.partitions[i].length;
				part_info.start = part_table.partitions[i].start;
				part_info.type = part_table.partitions[i].type;
				wsprintf(part_info.part_name, "hd%c%u", 'a' + drive - 0x80, i + 1);

				if (part_found_num < part_num)	
					partitions_info[part_found_num++] = part_info;
				else
				{
					result = 0;
					break;
				}
			}
		}
		else break;

	}
	return result*part_found_num;
}

////////////////////////////////////////////////////////////////////////

WORD disk_read(BYTE bDrive,
		DWORD dwSector,
		WORD wSectorNumber,
		LPBYTE lpBuffer,
		DWORD  cbBuffSize)
{
	return op.read(bDrive, dwSector, wSectorNumber, lpBuffer, cbBuffSize);
}

////////////////////////////////////////////////////////////////////////

WORD disk_check(BYTE bDrive)
{
	return op.check(bDrive);
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int is_partition_extended(unsigned char type)
{
	return type == PARTITION_EXTENDED ||
		type == PARTITION_XINT13_EXTENDED ||
		type == PARTITION_LINUX_EXT;
}

////////////////////////////////////////////////////////////////////////

int is_partition_valid(unsigned char type)
{
	return type != PARTITION_ENTRY_UNUSED;
}

////////////////////////////////////////////////////////////////////////

int	enum_extended_partition(int drive,
							unsigned long start,
							PartitionInfo *partitions_info,
							int part_num,
							int &part_found_num,
							int &ext_part_num)
{
	PartitionTable part_table;
	int not_last;
	unsigned long log_start;
	int result;

	log_start = 0; // start of logic disk
	
	do
	{
		result = op.read(drive,	start + log_start,	1,
			(unsigned char *)&part_table, sizeof(part_table));
		if (!result) return 0;

		if (is_partition_valid(part_table.partitions[0].type))
		{
			PartitionInfo part_info;
			part_info.length = part_table.partitions[0].length;
			part_info.start = part_table.partitions[0].start + log_start + start;
			part_info.type = part_table.partitions[0].type;
			wsprintf(part_info.part_name, "hd%c%u", 'a' + drive - 0x80, ext_part_num);

			ext_part_num++;
			if (part_found_num < part_num)	
				partitions_info[part_found_num++] = part_info;
			else
			{
				result = 0;
				break;
			}
			
		}

		if (is_partition_extended(part_table.partitions[1].type))
			log_start = part_table.partitions[1].start;
		else not_last = 0;
		
	} while (not_last);

	return result;
}