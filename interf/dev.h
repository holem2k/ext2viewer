#ifndef __DEV_H__
#define __DEV_H__

typedef struct tag_device
{
	DWORD first_sector;
	DWORD length;
	CHAR  name[10];
	BYTE log_block_mul;
	BYTE code;
	BOOL is_removable;
} device;

device *open_dev(const char *dev_name);
void close_dev(device *dev);

BOOL read_dev(device *dev, DWORD sector, DWORD num_of_sec,
			  LPBYTE buffer, DWORD buflen);

BOOL read_log_dev(device *dev, DWORD block, DWORD num_of_block,
				  LPBYTE buffer, DWORD buflen);

void set_dev_block(device *dev, BYTE log_block_mul);


#endif //__DEV_H__
