#include <stdafx.h>
#include <assert.h>
#include "ext2_core.h"


//////////////////////////////////////////////////////////////////////////////

ext2_super_block *read_super(device *dev)
{
	ext2_super_block *super_block = new ext2_super_block;
	if (super_block)
	{
		if (!read_dev(dev, 2, 2, (LPBYTE)super_block, sizeof(ext2_super_block)))
		{
			delete super_block;
			super_block = NULL;
		}
		
	}
	return super_block;
}

//////////////////////////////////////////////////////////////////////////////

BOOL check_super(ext2_super_block *super_block)
{
	BOOL result = TRUE; // assume ok
	if (super_block->s_magic != EXT2_SUPER_MAGIC  ||
	    super_block->s_log_block_size != super_block->s_log_block_size ||
		super_block->s_blocks_per_group > (DWORD)(1024<<super_block->s_log_block_size)*8 ||
		super_block->s_inodes_per_group > (DWORD)(1024<<super_block->s_log_block_size)*8
		)
		result = FALSE;


	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL read_group_descs(device *dev, 
					  ext2_super_block *super_block, 
					  ext2_group_desc *&group_descs,
					  DWORD &num_group_desc)
{
	num_group_desc = (super_block->s_blocks_count -
		super_block->s_first_data_block +
		super_block->s_blocks_per_group - 1)/super_block->s_blocks_per_group;

	DWORD desc_per_block = (1024<<super_block->s_log_block_size)/sizeof(ext2_group_desc);
	DWORD db_count = (num_group_desc - 1)/desc_per_block + 1;
	
	BOOL  result = FALSE;
	group_descs = new ext2_group_desc[db_count*desc_per_block];
	if (group_descs)
	{
		DWORD desc_group_block = 1 + super_block->s_first_data_block;
		if (read_log_dev(dev, desc_group_block, db_count, (unsigned char *)group_descs, db_count*(1024<<super_block->s_log_block_size)))
		{
			result = TRUE;
		}
		else
		{
			delete [] group_descs;
			group_descs = NULL;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

/*
 * Чтение inode c заданным номером
 */
BOOL iget(file_system *fs, DWORD inum, inode *inode/*out*/)
{
	BOOL result = FALSE;

	if (inum <= fs->super_block->s_inodes_count ||
		inum >= fs->super_block->s_first_data_block ||
		inum == EXT2_ROOT_INO
		)
	{
		DWORD  block_size = fs->block_size;
		
		DWORD group = (inum - 1)/fs->super_block->s_inodes_per_group;
		if (group < fs->num_group_descs)
		{
			
			/* номер inode в группе (нач. c 0)*/
			DWORD inode_in_group = (inum - 1)%fs->super_block->s_inodes_per_group; 
			
			/* число inode  в блоке */
			DWORD inode_per_block = (block_size)/fs->super_block->s_inode_size;
			
			/* номер блока, содержащего inode */
			DWORD inode_block = fs->group_descs[group].bg_inode_table +
				inode_in_group/inode_per_block; 
			
			/* номер inode в блоке, его содержащем (с 0) */
			DWORD inode_in_block = inode_in_group%inode_per_block; 
			
			BYTE *block = new BYTE[block_size];
			
			if (read_log_dev(fs->dev, inode_block, 1, block, block_size))
			{
				ext2_inode raw_inode;
				MoveMemory(&raw_inode,
					block + inode_in_block*fs->super_block->s_inode_size,
					fs->super_block->s_inode_size);
				inode->fs = fs;
				inode->i_atime = raw_inode.i_atime;
				inode->i_ctime = raw_inode.i_ctime;
				inode->i_dtime = raw_inode.i_dtime;
				inode->i_mtime = raw_inode.i_mtime;
				inode->i_blocks = raw_inode.i_blocks;
				inode->i_flags = raw_inode.i_flags;
				inode->i_uid = raw_inode.i_uid;
				inode->i_gid = raw_inode.i_gid;
				inode->i_links_count = raw_inode.i_links_count;
				inode->i_mode = raw_inode.i_mode;
				inode->i_size = raw_inode.i_size;
				for (int i = 0; i < EXT2_N_BLOCKS; i++)
				{
					inode->i_block[i] = raw_inode.i_block[i];
				}
				result = TRUE;
			}
			delete [] block;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

/*
 * Нахождение номера блока в ФС по известному номеру блока в файле(кот. нум с нуля)
 * failure: return 0
 */
DWORD bresolve(inode *inode, DWORD block)
{
	file_system *fs = inode->fs;

	DWORD result = 0;
	if (inode->i_size > 0)
	{
		/* Число блоков в файле */
		DWORD num_blocks = (inode->i_size - 1)/fs->block_size + 1;
		if (block < num_blocks)
		{
			DWORD num_direct = EXT2_NDIR_BLOCKS;
			if (block < num_direct)
			{
				result = inode->i_block[block];
			}
			else 
			{
				/* Число указателей в блоке*/
				DWORD pointer_per_block = fs->block_size/sizeof(inode->i_block[0]);
				DWORD relative_block = block - num_direct;
			
				DWORD p = 0; /* степень 'indirection' блока - 1*/
				DWORD p_size = pointer_per_block; /* число указателей */
				while (relative_block > p_size)
				{
					relative_block -= p_size;
					p++;
					p_size *= pointer_per_block;
				}
				assert(p >= 0 && p <= 2);


				DWORD pointer_num = inode->i_block[EXT2_NDIR_BLOCKS + p];

				DWORD pointer_block[1024]; /* 1024 -  макс. число указателей в блоке */
				while (p-- > 0)
				{
					if(read_log_dev(fs->dev, pointer_num, 1, (unsigned char *)pointer_block, fs->block_size))
					{
						p_size /= pointer_per_block;
						DWORD N = relative_block/p_size;
						pointer_num = pointer_block[N];
						relative_block = relative_block%p_size;
					}
					else break;
				}

				if(read_log_dev(fs->dev, pointer_num, 1, (unsigned char *)pointer_block, fs->block_size))
				{
					result = pointer_block[relative_block];
				}
			}
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL read_file(inode *inode, DWORD start, DWORD length, LPBYTE buffer/*out*/)
{
	file_system *fs = inode->fs;
	DWORD block_size = fs->block_size;

	DWORD first_block = start/block_size;
	DWORD end = start + length - 1;
	DWORD last_block = end/block_size;

	DWORD bytes_written = 0;
	for (DWORD i = first_block; i <= last_block; i++)
	{
		if (DWORD phys_block = bresolve(inode, i))
		{	
			BYTE block[4096];
			if (read_log_dev(fs->dev, phys_block, 1, block, block_size))
			{
				/* смещение от начала блока */
				int dstart = start - block_size*i; 
				if (dstart < 0) dstart = 0;
				
				/* смещение от конца блока */
				int dend = block_size*(i + 1) - end - 1; 
				if (dend < 0) dend = 0;
				
				MoveMemory(buffer + bytes_written, block + dstart, block_size - dstart - dend);
				bytes_written += block_size - dstart - dend;
			}
			else break;
		}
		else break;
	}

	return length == bytes_written;
}

//////////////////////////////////////////////////////////////////////////////

/*
 * Преобразование 'имя файла' -> номер inode
 * failure: 0
 */

DWORD name_resolve(file_system *fs, char *file_name)
{
	if (file_name[0] != '/') return 0;
	if (!lstrcmp(file_name, "/")) return EXT2_ROOT_INO;

	// try to get inode number from cache
	DWORD inode_num;
	if (get_from_res_cache(&fs->rc, file_name, inode_num))
	{
		return inode_num;
	}

	inode_num = EXT2_ROOT_INO;
	DWORD result = 0;
	DWORD parse_char = 0;
	DWORD error;
	do
	{
		error = 1;

		DWORD i = 1;
		while (file_name[parse_char + i] != 0 && 
			file_name[parse_char + i] != '/') i++;

		char current_dir[EXT2_NAME_LEN + 1];
		lstrcpyn(current_dir, file_name + parse_char + 1, i);
		parse_char += i;

		inode inode;
		if (iget(fs, inode_num, &inode) && S_ISDIR(inode.i_mode))
		{
			assert(inode.i_size);
			BYTE *raw_dir = new BYTE[inode.i_size];
			if (read_file(&inode, 0, inode.i_size, raw_dir))
			{
				DWORD dir_pointer = 0;
				ext2_dir_entry dir_entry;

				MoveMemory(&dir_entry, raw_dir + dir_pointer, sizeof(dir_entry));
				BOOL end_of_dir = dir_entry.rec_len == 0;
				while (!end_of_dir) 
				{
					char directory[EXT2_NAME_LEN + 1];
					
					lstrcpyn(directory,
						(char *)(raw_dir + dir_pointer + 8),
						dir_entry.name_len + 1);
					
					if (!lstrcmp(directory, current_dir) && dir_entry.rec_len)
					{
						inode_num = dir_entry.inode;
						// fill cache line
						put_into_res_cache(&fs->rc, file_name, inode_num);
						error = 0;
						break;
					}	

					dir_pointer += dir_entry.rec_len;
					MoveMemory(&dir_entry, raw_dir + dir_pointer, sizeof(dir_entry));
					end_of_dir = dir_pointer >= inode.i_size;
				}
			}
			delete [] raw_dir;
		}
	} while (file_name[parse_char] && !error); /* не конец строки */

	return error ? 0 : inode_num;
}

//////////////////////////////////////////////////////////////////////////////

void fill_file_from_inode(common_file *file, inode *inode)
{
	file->i_size = inode->i_size;
	file->i_uid = inode->i_uid;
	file->i_gid = inode->i_gid;
	file->i_atime = inode->i_atime;
	file->i_ctime = inode->i_ctime;
	file->i_dtime = inode->i_ctime;
	file->i_links_count = inode->i_links_count;
	file->i_mtime = inode->i_mtime;
	file->rights = inode->i_mode & 0x1FF;
	
	// file type
	WORD mode = inode->i_mode;
	FILE_TYPE type;
	if (S_ISREG(mode)) type = FT_REG;
	else if (S_ISDIR(mode)) type = FT_DIR;
	else if (S_ISBLK(mode)) type = FT_BLK;
	else if (S_ISCHR(mode)) type = FT_CHR;
	else if (S_ISLNK(mode)) type = FT_LNK;
	else if (S_ISFIFO(mode)) type = FT_FIFO;
	else if (S_ISSOCK(mode)) type = FT_SOCK;
	file->type = type;
}

//////////////////////////////////////////////////////////////////////////////