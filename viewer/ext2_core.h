#ifndef __EXT2_CORE_H__
#define __EXT2_CORE_H__
#include "dev.h"
#include "ext2_fs.h"
#include "res_cache.h"

/*
 * Constants relative to the data blocks
 */
const DWORD EXT2_NDIR_BLOCKS	= 12;
const DWORD EXT2_IND_BLOCK		= EXT2_NDIR_BLOCKS;
const DWORD EXT2_DIND_BLOCK		= (EXT2_IND_BLOCK + 1);
const DWORD EXT2_TIND_BLOCK		= (EXT2_DIND_BLOCK + 1);
const DWORD EXT2_N_BLOCKS		= (EXT2_TIND_BLOCK + 1);


/*
 * Inode flags
 */
const DWORD EXT2_SECRM_FL		= 0x00000001; /* Secure deletion */
const DWORD EXT2_UNRM_FL		= 0x00000002; /* Undelete */
const DWORD EXT2_COMPR_FL		= 0x00000004; /* Compress file */
const DWORD EXT2_SYNC_FL		= 0x00000008; /* Synchronous updates */
const DWORD EXT2_IMMUTABLE_FL	= 0x00000010; /* Immutable file */
const DWORD EXT2_APPEND_FL		= 0x00000020; /* writes to file may only append */
const DWORD EXT2_NODUMP_FL		= 0x00000040; /* do not dump file */
const DWORD EXT2_NOATIME_FL		= 0x00000080; /* do not update atime */
/* Reserved for compression usage... */
const DWORD EXT2_DIRTY_FL		= 0x00000100;
const DWORD EXT2_COMPRBLK_FL	 = 0x00000200; /* One or more compressed clusters */
const DWORD EXT2_NOCOMP_FL		= 0x00000400; /* Don't compress */
const DWORD EXT2_ECOMPR_FL		= 0x00000800; /* Compression error */
/* End compression flags --- maybe not all used */	
const DWORD EXT2_BTREE_FL		= 0x00001000; /* btree format dir */
const DWORD EXT2_RESERVED_FL	= 0x80000000; /* reserved for ext2 lib */

/*
 * Special inodes numbers
 */
const DWORD	EXT2_BAD_INO		= 1;		/* Bad blocks inode */
const DWORD	EXT2_ROOT_INO		= 2;		/* Root inode */
const DWORD	EXT2_ACL_IDX_INO	= 3;		/* ACL inode */
const DWORD	EXT2_ACL_DATA_INO	= 4;		/* ACL inode */
const DWORD	EXT2_BOOT_LOADER_INO = 5;	/* Boot loader inode */
const DWORD	EXT2_UNDEL_DIR_INO	=6;		/* Undelete directory inode */

#pragma pack(push, __ext2fs_h__)
#pragma pack(1)
struct ext2_inode {
	WORD	i_mode;		/* File mode */
	WORD	i_uid;		/* Owner Uid */
	DWORD	i_size;		/* Size in bytes */
	DWORD	i_atime;	/* Access time */
	DWORD	i_ctime;	/* Creation time */
	DWORD	i_mtime;	/* Modification time */
	DWORD	i_dtime;	/* Deletion Time */
	WORD	i_gid;		/* Group Id */
	WORD	i_links_count;	/* Links count */
	DWORD	i_blocks;	/* Blocks count */
	DWORD	i_flags;	/* File flags */
	union {
		struct {
			DWORD  l_i_reserved1;
		} linux1;
		struct {
			DWORD  h_i_translator;
		} hurd1;
		struct {
			DWORD  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	DWORD	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	DWORD	i_version;	/* File version (for NFS) */
	DWORD	i_file_acl;	/* File ACL */
	DWORD	i_dir_acl;	/* Directory ACL */
	DWORD	i_faddr;	/* Fragment address */
	union {
		struct {
			BYTE	l_i_frag;	/* Fragment number */
			BYTE	l_i_fsize;	/* Fragment size */
			WORD	i_pad1;
			DWORD	l_i_reserved2[2];
		} linux2;
		struct {
			BYTE	h_i_frag;	/* Fragment number */
			BYTE	h_i_fsize;	/* Fragment size */
			WORD	h_i_mode_high;
			WORD	h_i_uid_high;
			WORD	h_i_gid_high;
			DWORD	h_i_author;
		} hurd2;
		struct {
			BYTE	m_i_frag;	/* Fragment number */
			BYTE	m_i_fsize;	/* Fragment size */
			WORD	m_pad1;
			DWORD	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
};


#pragma pack(1)
typedef struct tag_ext2_group_desc
{
	DWORD	bg_block_bitmap;		/* Blocks bitmap block */
	DWORD	bg_inode_bitmap;		/* Inodes bitmap block */
	DWORD	bg_inode_table;		    /* Inodes table block  */
	WORD	bg_free_blocks_count;	/* Free blocks count   */
	WORD	bg_free_inodes_count;	/* Free inodes count   */
	WORD	bg_used_dirs_count;	    /* Directories count   */
	WORD	bg_pad;
	DWORD	bg_reserved[3];
} ext2_group_desc;


const WORD EXT2_SUPER_MAGIC	= 0xEF53;

#pragma pack(1)
struct ext2_super_block
{
	DWORD	s_inodes_count;		/* Inodes count */
	DWORD	s_blocks_count;		/* Blocks count */
	DWORD	s_r_blocks_count;	/* Reserved blocks count */
	DWORD	s_free_blocks_count;	/* Free blocks count */
	DWORD	s_free_inodes_count;	/* Free inodes count */
	DWORD	s_first_data_block;	/* First Data Block */
	DWORD	s_log_block_size;	/* Block size */
	int	s_log_frag_size;	/* Fragment size */
	DWORD	s_blocks_per_group;	/* # Blocks per group */
	DWORD	s_frags_per_group;	/* # Fragments per group */
	DWORD	s_inodes_per_group;	/* # Inodes per group */
	DWORD	s_mtime;		/* Mount time */
	DWORD	s_wtime;		/* Write time */
	WORD	s_mnt_count;		/* Mount count */
	short	s_max_mnt_count;	/* Maximal mount count */
	WORD	s_magic;		/* Magic signature */
	WORD	s_state;		/* File system state */
	WORD	s_errors;		/* Behaviour when detecting errors */
	WORD	s_minor_rev_level; 	/* minor revision level */
	DWORD	s_lastcheck;		/* time of last check */
	DWORD	s_checkinterval;	/* max. time between checks */
	DWORD	s_creator_os;		/* OS */
	DWORD	s_rev_level;		/* Revision level */
	WORD	s_def_resuid;		/* Default uid for reserved blocks */
	WORD	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */	
	DWORD	s_first_ino; 		/* First non-reserved inode */
	WORD   s_inode_size; 		/* size of inode structure */
	WORD	s_block_group_nr; 	/* block group # of this superblock */
	DWORD	s_feature_compat; 	/* compatible feature set */
	DWORD	s_feature_incompat; 	/* incompatible feature set */
	DWORD	s_feature_ro_compat; 	/* readonly-compatible feature set */
	BYTE	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	DWORD	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	BYTE	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	BYTE	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	WORD	s_padding1;
	DWORD	s_reserved[204];	/* Padding to the end of the block */
};

const DWORD EXT2_NAME_LEN  = 255;

const DWORD S_IFMT		= 0170000;
const DWORD S_IFSOCK	= 0140000;
const DWORD S_IFLNK		= 0120000;
const DWORD S_IFREG		= 0100000;
const DWORD S_IFBLK		= 0060000;
const DWORD S_IFDIR		= 0040000;
const DWORD S_IFCHR		= 0020000;
const DWORD S_IFIFO		= 0010000;
const DWORD S_ISUID		= 0004000;
const DWORD S_ISGID		= 0002000;
const DWORD S_ISVTX		= 0001000;

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

struct ext2_dir_entry
{
	DWORD	inode;					/* Inode number */
	WORD	rec_len;				/* Directory entry length */
	BYTE	name_len;				/* Name length */
//	char	name[EXT2_NAME_LEN];	/* File name */
};

#pragma pack(pop, __ext2fs_h__)

typedef struct tag_file_system
{
	device *dev;

	ext2_super_block *super_block;
	ext2_group_desc *group_descs;
	DWORD num_group_descs;

	DWORD block_size;
	res_cache rc;

} file_system;



typedef struct tag_inode
{
	WORD	i_mode;		/* File mode */
	WORD	i_uid;		/* Owner Uid */
	DWORD	i_size;		/* Size in bytes */
	DWORD	i_atime;	/* Access time */
	DWORD	i_ctime;	/* Creation time */
	DWORD	i_mtime;	/* Modification time */
	DWORD	i_dtime;	/* Deletion Time */
	WORD	i_gid;		/* Group Id */
	WORD	i_links_count;	/* Links count */
	DWORD	i_blocks;	/* Blocks count */
	DWORD	i_flags;	/* File flags */
	DWORD	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */

	file_system *fs;
} inode;

const DWORD MAX_FILE_SYSTEMS = 10;
const DWORD MAX_FILES = 100;


ext2_super_block *read_super(device *dev);
BOOL check_super(ext2_super_block *super_block);
BOOL read_group_descs(device *dev, 
					  ext2_super_block *super_block, 
					  ext2_group_desc *&group_descs,
					  DWORD &num_group_desc);
BOOL alloc_new_fsd(FS_HANDLE &num_of_fs);
BOOL free_fsd(FS_HANDLE fs_handle);

BOOL iget(file_system *fs, DWORD inum, inode *inode/*out*/);
DWORD bresolve(inode *inode, DWORD block);
BOOL read_file(inode *inode, DWORD start, DWORD length, LPBYTE buffer/*out*/);
DWORD name_resolve(file_system *fs, char *file_name);

void fill_file_from_inode(common_file *file, inode *inode);

inline FS_HANDLE index2fsh(DWORD index)
{
	return index + 10;
}

inline DWORD fsh2index(FS_HANDLE fsh)
{
	return fsh - 10;
}

inline FILE_HANDLE index2fh(DWORD index)
{
	return index + 10;
}

inline DWORD fh2index(FS_HANDLE fh)
{
	return fh - 10;
}




#endif //__EXT2_CORE_H__