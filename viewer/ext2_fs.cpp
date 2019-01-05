#include <stdafx.h>
#include <assert.h>
#include "ext2_core.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////////


static file_system file_systems[MAX_FILE_SYSTEMS];
static DWORD num_of_fs = 0;

static inode inodes[MAX_FILES];
static DWORD num_of_inodes = 0;

//////////////////////////////////////////////////////////////////////////////

FS_HANDLE mount(const char *part_name)
{
	if (!num_of_fs)
	{
		for (DWORD i = 0; i < MAX_FILE_SYSTEMS; i++) file_systems[i].dev = NULL;
	}

	device *dev = NULL;
	ext2_super_block *super_block = NULL;
	ext2_group_desc *group_descs = NULL;
	DWORD num_group_descs;

	FS_HANDLE result = FS_INVALID_HANDLE;

	if (dev = open_dev(part_name))
	{
		if ((super_block = read_super(dev)) && check_super(super_block))
		{
			set_dev_block(dev, (1<<super_block->s_log_block_size)*2);
			if (read_group_descs(dev, super_block, group_descs, num_group_descs))
			{
				if(alloc_new_fsd(result))
				{
					DWORD index = fsh2index(result);
					file_systems[index].dev = dev;
					file_systems[index].group_descs = group_descs;
					file_systems[index].num_group_descs = num_group_descs;
					file_systems[index].super_block = super_block;
					file_systems[index].block_size = 1024<<super_block->s_log_block_size;

					init_res_cache(&file_systems[index].rc, dev);
				}
			}
		}
	}

	if (result == FS_INVALID_HANDLE) // fuck !
	{
		delete dev;
		delete super_block;
		delete group_descs;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL umount(FS_HANDLE fs_handle)
{
	return free_fsd(fs_handle);
}

//////////////////////////////////////////////////////////////////////////////

FILE_HANDLE open(char file_name)
{
	return FILE_INVALID_HANDLE;
}

//////////////////////////////////////////////////////////////////////////////

BOOL close(FILE_HANDLE handle)
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

BOOL start_enum_dirs(FS_HANDLE fsh, char *dir_name, directory *&dirs/*out*/, int &num_of_dir/*out*/)
{
	BOOL result = FALSE;
	dirs = NULL;

	int fs_index = fsh2index(fsh);
	// check filesystem handle
	if (file_systems[fs_index].dev == NULL) return FALSE;
	file_system *fs = &file_systems[fs_index];

	DWORD inode_number;
	if (inode_number = name_resolve(fs, dir_name))
	{
		// директория найдена
		inode dir_inode;
		if (iget(fs, inode_number, &dir_inode) && S_ISDIR(dir_inode.i_mode))
		{
			// inode прочитан
			// в текущей реализации ext2 жесткие ссылки на директории невозможны =>
			num_of_dir = dir_inode.i_links_count - 2;
			dirs = new directory[num_of_dir];
			assert(dir_name);
			BYTE *raw_dir = new BYTE[dir_inode.i_size];
			assert(raw_dir);

			if(read_file(&dir_inode, 0, dir_inode.i_size, raw_dir))
			{
				// прочитан файл директории
				int current_num_of_dir = 0;
				ext2_dir_entry dir_entry;
				DWORD dir_pointer = 0;
				DWORD spec_dir = 0;
				do
				{
					MoveMemory(&dir_entry, raw_dir + dir_pointer, sizeof(dir_entry));
					inode subdir_inode;

					if (iget(fs, dir_entry.inode, &subdir_inode) && S_ISDIR(subdir_inode.i_mode))
					{
						if (spec_dir >= 2)
						{
							dirs[current_num_of_dir].has_subdir = subdir_inode.i_links_count > 2;
							lstrcpyn(dirs[current_num_of_dir].dir_name,
								(char *)(raw_dir + dir_pointer + 8),
								dir_entry.name_len + 1); 
							current_num_of_dir++;
						} else spec_dir++;
					}
					dir_pointer += dir_entry.rec_len;
				} while (current_num_of_dir  < num_of_dir);
				result = TRUE;
			}
			delete [] raw_dir;
		}
	}

	if (!result) 
	{
		delete [] dirs;
		dirs = NULL;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL cancel_enum_dirs(directory *dir)
{
	delete [] dir;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

BOOL start_enum(FS_HANDLE fsh, char *dir_name, common_file *&files/*out*/, int &num_of_files/*out*/)
{
	BOOL result = FALSE;
	files = NULL;
	int fs_index = fsh2index(fsh);
	// check filesystem handle
	if (file_systems[fs_index].dev == NULL) return FALSE;
	file_system *fs = &file_systems[fs_index];

	DWORD inode_number;
	if (inode_number = name_resolve(fs, dir_name))
	{
		// директория найдена
		inode dir_inode;
		if (iget(fs, inode_number, &dir_inode) && S_ISDIR(dir_inode.i_mode))
		{
			DWORD approx_num_files = dir_inode.i_size/(sizeof(ext2_dir_entry) + 1);
			files = new common_file[approx_num_files];
			assert(files);

			BYTE *raw_dir = new BYTE[dir_inode.i_size];
			assert(raw_dir);

			if (read_file(&dir_inode, 0, dir_inode.i_size, raw_dir))
			{
				int current_num_of_files = 0;
				DWORD dir_pointer = 0;
				DWORD spec_dir = 0;
				
				ext2_dir_entry dir_entry;

				MoveMemory(&dir_entry, raw_dir + dir_pointer, sizeof(dir_entry));
				BOOL end_of_dir = dir_entry.rec_len == 0;

				while (!end_of_dir)
				{
					inode common_inode;
					result = FALSE; // assume iget failure
					if (iget(fs, dir_entry.inode, &common_inode))
					{
						if (spec_dir >= 2)
						{
							fill_file_from_inode(files + current_num_of_files, &common_inode);
							// file name
							lstrcpyn(files[current_num_of_files].file_name,
								(char *)(raw_dir + dir_pointer + 8),
								dir_entry.name_len + 1); 
							
							current_num_of_files++;
						} else spec_dir++;

						result = TRUE;
					} else break;
					dir_pointer += dir_entry.rec_len;
					MoveMemory(&dir_entry, raw_dir + dir_pointer, sizeof(dir_entry));
					end_of_dir = dir_pointer >= dir_inode.i_size;
				}
				num_of_files = current_num_of_files;
			}
			delete [] raw_dir;
		}
	}

	if (!result) 
	{
		delete [] files;
		files = NULL;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL cancel_enum(common_file *files)
{
	delete [] files;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

BOOL copy_file(FS_HANDLE fsh, char *src, BYTE *buffer, DWORD start, DWORD length)
{
	file_system *fs = &file_systems[fsh2index(fsh)];
	if (fs->dev == NULL) return FALSE; /* is fsh valid ? */

	BOOL result = FALSE;
	DWORD inode_num;
	if (inode_num = name_resolve(fs, src))
	{
		inode inode_file;
		if (iget(fs, inode_num, &inode_file) && S_ISREG(inode_file.i_mode))
		{
			if(read_file(&inode_file, start, length, buffer))
			{
				result = TRUE;
			}
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL alloc_new_fsd(FS_HANDLE &fs_handle)
{
	BOOL result = FALSE;
	for (DWORD i = 0; i < MAX_FILE_SYSTEMS; i++)
	{
		if (file_systems[i].dev == NULL)
		{
			fs_handle = index2fsh(i);
			result = TRUE;
			break;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL free_fsd(FS_HANDLE fs_handle)
{
	DWORD index = fsh2index(fs_handle);
	BOOL result = FALSE;
	if (file_systems[index].dev != NULL)
	{
		delete file_systems[index].dev;
		delete [] file_systems[index].group_descs;
		delete file_systems[index].super_block;
		file_systems[index].dev = NULL; // mark free handle
		result = TRUE;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL get_file_prop(FS_HANDLE fsh, char *src, common_file *file)
{
	file_system *fs = &file_systems[fsh2index(fsh)];
	if (fs->dev == NULL) return FALSE; /* is fsh valid ? */

	BOOL result = FALSE;
	DWORD inode_num;
	if (inode_num = name_resolve(fs, src))
	{
		inode inode_file;
		if (iget(fs, inode_num, &inode_file))
		{
			fill_file_from_inode(file, &inode_file);
			// get file name
			char *file_name;
			if (!lstrcmp("/", src)) /* handle root dir */
			{
				file_name = src;
			}
			else
			{
				int i = lstrlen(src);
				while (src[i] != '/' && i > 0) i--;
				assert(src[i] == '/');
				file_name = src + i + 1;
			}
			lstrcpyn(file->file_name, file_name, EXT2_NAME_LEN);
			result = TRUE;
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL resolve_sym_link(FS_HANDLE fsh, char *sym_link, char *buf/*out*/, DWORD buflen)
{
	file_system *fs = &file_systems[fsh2index(fsh)];
	if (fs->dev == NULL) return FALSE; /* is fsh valid ? */

	BOOL result = FALSE;
	DWORD inode_num;
	if (inode_num = name_resolve(fs, sym_link))
	{
		inode inode_sym_link;
		if (iget(fs, inode_num, &inode_sym_link) && inode_sym_link.i_size < buflen/* < because zero */)
		{
			char *link;
			BYTE block[4096];
			if (inode_sym_link.i_blocks)
			{
				if(read_file(&inode_sym_link, 0, fs->block_size, block))
				{
					link = (char *)block;
					result = TRUE;
				}
			}
			else
			{
				link = (char *)&inode_sym_link.i_block[0];
				result = TRUE;
			}

			lstrcpy(buf, link);
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////

BOOL get_mount_info(FS_HANDLE fsh, mount_info *mi/*out*/)
{
	file_system *fs = &file_systems[fsh2index(fsh)];
	if (fs->dev == NULL) return FALSE; /* is fsh valid ? */

	ext2_super_block *sb = fs->super_block;
	mi->blocks_count = sb->s_blocks_count;
	mi->checkinterval = sb->s_checkinterval;
	mi->errors = sb->s_errors;
	mi->free_blocks_count = sb->s_free_blocks_count;
	mi->free_inodes_count = sb->s_free_inodes_count;
	mi->inodes_count = sb->s_inodes_count;
	mi->inodes_per_group = sb->s_inodes_per_group;
	mi->lastcheck = sb->s_lastcheck;
	mi->block_size = 1024<<sb->s_log_block_size;
	mi->max_mnt_count = sb->s_max_mnt_count;
	mi->minor_rev_level = sb->s_minor_rev_level;
	mi->mnt_count = sb->s_mnt_count;
	mi->mtime = sb->s_mtime;
	mi->r_blocks_count = sb->s_r_blocks_count;
	mi->rev_level = sb->s_rev_level;
	mi->state = sb->s_state;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

DWORD is_removable(FS_HANDLE fsh)
{
	file_system *fs = &file_systems[fsh2index(fsh)];
	if (fs->dev == NULL) return 0; /* is fsh valid ? */

	return fs->dev->is_removable ?  DISK_REMOVABLE : DISK_FIXED;
}

//////////////////////////////////////////////////////////////////////////////
