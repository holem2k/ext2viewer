#ifndef __EXT2_FS_H__

typedef DWORD FS_HANDLE;
typedef DWORD FILE_HANDLE;

const FS_INVALID_HANDLE = 0;
const FILE_INVALID_HANDLE = 0;

typedef struct tag_directory
{
	char dir_name[256]; // это не полное имя !!!
	BOOL has_subdir;
} directory;


enum FILE_TYPE {FT_SOCK, FT_LNK, FT_REG, FT_BLK, FT_DIR, FT_CHR, FT_FIFO};

const WORD S_IRWXU = 00700;
const WORD S_IRUSR = 00400;
const WORD S_IWUSR = 00200;
const WORD S_IXUSR = 00100;

const WORD S_IRWXG = 00070;
const WORD S_IRGRP = 00040;
const WORD S_IWGRP = 00020;
const WORD S_IXGRP = 00010;

const WORD S_IRWXO = 00007;
const WORD S_IROTH = 00004;
const WORD S_IWOTH = 00002;
const WORD S_IXOTH = 00001;

typedef struct tag_common_file
{
	WORD	i_gid;		
	WORD	i_uid;		

	DWORD	i_size;		
	DWORD	i_atime;	
	DWORD	i_ctime;	
	DWORD	i_mtime;	
	DWORD	i_dtime;	
	WORD	i_links_count;	

	FILE_TYPE type;
	WORD rights;
	char file_name[256];
} common_file;

typedef struct mount_info
{
	DWORD	inodes_count;		/* Inodes count */
	DWORD	blocks_count;		/* Blocks count */
	DWORD	r_blocks_count;		/* Reserved blocks count */
	DWORD	free_blocks_count;	/* Free blocks count */
	DWORD	free_inodes_count;	/* Free inodes count */
	DWORD	block_size;			/* Block size (in BYTEs)*/

	DWORD	inodes_per_group;	/* # Inodes per group */
	DWORD	mtime;				/* Mount time */
	WORD	mnt_count;			/* Mount count */
	short	max_mnt_count;		/* Maximal mount count */
	WORD	state;				/* File system state */
	WORD	errors;				/* Behaviour when detecting errors */
	WORD	minor_rev_level; 	/* minor revision level */
	DWORD	lastcheck;			/* time of last check */
	DWORD	checkinterval;		/* max. time between checks */
	DWORD	rev_level;			/* Revision level */
} mount_info;


DWORD const	DISK_REMOVABLE = 0x1;
DWORD const DISK_FIXED     = 0x2;


FS_HANDLE mount(const char *part_name);
BOOL umount(FS_HANDLE fs_handle);

BOOL start_enum_dirs(FS_HANDLE fsh, char *dir_name, directory *&dirs/*out*/, int &num_of_dir/*out*/);
BOOL cancel_enum_dirs(directory *dir);

BOOL start_enum(FS_HANDLE fsh, char *dir_name, common_file *&files/*out*/, int &num_of_files/*out*/);
BOOL cancel_enum(common_file *files);


BOOL copy_file(FS_HANDLE fsh, char *src, char *buffer, DWORD start, DWORD length);
BOOL get_file_prop(FS_HANDLE fsh, char *src, common_file *file);
BOOL resolve_sym_link(FS_HANDLE fsh, char *sym_link, char *buf/*out*/, DWORD buflen);

BOOL get_mount_info(FS_HANDLE, mount_info *mi/*out*/);
DWORD is_removable(FS_HANDLE fsh);

#define __EXT2_FS_H__

#endif //__EXT2_FS_H__