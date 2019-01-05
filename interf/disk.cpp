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
			lpLibrary = (char *)DLL_DIO9x;
		else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			lpLibrary = (char *)DLL_DIONT;
		else 
			return 0;
		
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
		else 
			result = 0; 
	}

	if (result)
		ref_count++;
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
//	LOG((file, "disk_get_partitions(...), drive = 0x%X\n", drive));

	assert(drive >= 0x80);
	PartitionTable part_table;
	int result;

	result = op.read(drive, 0, 1, (unsigned char *)&part_table, sizeof(part_table));
	if (!result) 
	{
//		LOG((file, "Ошибка чтения MBR\n"));
		return 0;
	}

	if (part_table.magic != PARTITION_MAGIC)
	{
//		LOG((file, "Неверное значение Magic в MBR\n"));
		return 0;
	}

	int ext_part_num = 5;   // first logical disk number - hdx5
	int part_found_num = 0; // число найденных разделов
	result = 1;

	int primary_found = 0;

	for (int i = 0; i < 4 && result; i++)
	{
		Partition *partition = part_table.partitions + i;

		if (partition->type == PARTITION_ENTRY_UNUSED || !partition->length)
			continue;

		unsigned char type = partition->type;
		if (is_partition_extended(type))
		{
			if (!enum_extended_partition(drive, partition->start,
				partitions_info, part_num, part_found_num, ext_part_num))
			{
				result = 0;
				break;
			}
		}
		else
		{
			PartitionInfo part_info;
			part_info.length = partition->length;
			part_info.start = partition->start;
			part_info.type = partition->type;
            
            /*
             * BUG FIXED 9.01.03 by holem 
			 * wsprintf(part_info.part_name, "hd%c%u", 'a' + drive - 0x80, ++primary_found);
            */
            wsprintf(part_info.part_name, "hd%c%u", 'a' + drive - 0x80, i + 1);
			
//			LOG((file, "Раздел %s (Primary):\n\t- тип 0x%X\n\t- начало 0x%X\n\t- длина 0x%X\n\n", part_info.part_name, partition->type, partition->start, partition->length));

			if (part_found_num < part_num)	
				partitions_info[part_found_num++] = part_info;
			else
			{
//				LOG((file, "Слишком много разделов\n"));
				result = 0;
				break;
			}
		}
		
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
	LOG((file, "enum_extended_partition(...), start = 0x%X\n\n", start));

	PartitionTable part_table;
	int not_last;
	unsigned long log_start;
	int result;

	log_start = 0; // start of logic disk
	
	do
	{
		LOG((file, "read(...), sector = 0x%X\n", start + log_start));
		result = op.read(drive,	start + log_start,	1,
			(unsigned char *)&part_table, sizeof(part_table));

		if (!result) 
		{
//			LOG((file, "Ошибка чтения EBR\n"));
			return 0;
		}

		// some Partition Magic weird features
		if (part_table.magic == PARTITION_MAGIC_MAGIC)
		{
			return 1;
		}

		if (part_table.magic != PARTITION_MAGIC)
		{
			LOG((file, "Неверное значение Magic в MBR\n"));
			return 0;
		}

		Partition *partition = part_table.partitions;
		if (partition->type != PARTITION_ENTRY_UNUSED && partition->length)
		{
			PartitionInfo part_info;
			part_info.length = partition->length;
			part_info.start = partition->start + log_start + start;
			part_info.type = partition->type;
			wsprintf(part_info.part_name, "hd%c%u", 'a' + drive - 0x80, ext_part_num);

//			LOG((file, "Раздел %s (Logic):\n\t- тип 0x%X\n\t- начало 0x%X\n\t- длина 0x%X\n\n", part_info.part_name, partition->type, part_info.start, part_info.length));
//			LOG((file, "Entry 0:\n\t- начало 0x%X\n\t- длина 0x%X\n", part_table.partitions[0].start, part_table.partitions[0].length));
//			LOG((file, "Entry 1:\n\t- начало 0x%X\n\t- длина 0x%X\n", part_table.partitions[1].start, part_table.partitions[1].length));
//			LOG((file, "---------------------------\n"));
			ext_part_num++;
			if (part_found_num < part_num)	
				partitions_info[part_found_num++] = part_info;
			else
			{
//				LOG((file, "Слишком много разделов\n"));
				result = 0;
				break;
			}
		}

		if (is_partition_extended(part_table.partitions[1].type))
			log_start = part_table.partitions[1].start;
		else 
			not_last = 0;
		
	} while (not_last);

	return result;
}

////////////////////////////////////////////////////////////////////////
