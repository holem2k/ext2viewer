#include <stdafx.h>
#include "disk.h"
#include "partition.h"
#include "dev.h"

device *open_dev(const char *dev_name)
{
	if (!disk_init()) return NULL; /*load direct disk access libs*/

	device *dev  = NULL;
	if (lstrlen(dev_name) >= 3)
	{
		// device name :== hdDNN | fdN
		// D :== 'a'|'b'|'c'|'d'
		// NN :== number
		char name[10];
		lstrcpyn(name, dev_name, 3);
		if (!strcmp(name, "hd")) /* device is hdd*/
		{
			BYTE code;
			switch (dev_name[2])
			{
			case 'a': code = 0x80; break;
			case 'b': code = 0x81; break;
			case 'c': code = 0x82; break;
			case 'd': code = 0x83; break;
			default : code = 0x0;
			}
			if (code)
			{
				PartitionInfo pi[256];
				int num_of_partitions = disk_get_partitions(code, pi, 256);
				if (num_of_partitions)
				{
					int find = -1;
					for (int i = 0; i < num_of_partitions; i++)
					{
						if (!lstrcmp(dev_name, pi[i].part_name))
						{
							find = i; break;
						}
					}

					if (find != -1)
					{
						dev = new device;
						lstrcpy(dev->name, pi[find].part_name);
						dev->first_sector = pi[find].start;
						dev->length = pi[find].length; /*unknown*/
						dev->code = code;
						dev->log_block_mul = 0;
						dev->is_removable = FALSE;
					}
				}
			}
		}
		else if (!strcmp(name, "fd") && lstrlen(dev_name) == 3) /*device is floppy*/
		{
			BYTE code = dev_name[2] - '0';
			if (code == 0 || code == 1)
			{
				dev = new device;
				lstrcpy(dev->name, dev_name);
				dev->log_block_mul = 0;
				dev->first_sector = 0;
				dev->length = 0; /*unknown*/
				dev->code = code;
				dev->is_removable = TRUE;
			}
		}
	}

	if (!dev) disk_done(); /* unload dda libs on failure */
	return dev;
}

//////////////////////////////////////////////////////////////////////////

void close_dev(device *dev)
{

}

//////////////////////////////////////////////////////////////////////////

BOOL read_dev(device *dev, DWORD sector, DWORD num_of_sec,
			  LPBYTE buffer, DWORD bufsize)
{
	int try_count = 0;
	int max_try_count = dev->code == 0 || dev->code == 1 ? 4 : 1;

	BOOL result;
	do
	{
		result = disk_read(dev->code, dev->first_sector + sector, num_of_sec,
			buffer, bufsize);
		
	} while (!result && ++try_count < max_try_count);

	return result;
}

//////////////////////////////////////////////////////////////////////////

BOOL read_log_dev(device *dev, DWORD block, DWORD num_of_block,
				  LPBYTE buffer, DWORD bufsize)
{
	int try_count = 0;
	int max_try_count = dev->code == 0 || dev->code == 1 ? 4 : 1;

	BOOL result;
	do
	{
		result = disk_read(dev->code, dev->first_sector + block*dev->log_block_mul,
			num_of_block*dev->log_block_mul,
			buffer,	bufsize);
		
	} while (!result && ++try_count < max_try_count);

	return result;

}

//////////////////////////////////////////////////////////////////////////

void set_dev_block(device *dev, BYTE log_block_mul)
{
	dev->log_block_mul = log_block_mul;
}

//////////////////////////////////////////////////////////////////////////