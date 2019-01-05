#include <stdafx.h>
#include "stat.h"
#include "text.h"


/////////////////////////////////////////////////////////////////////////

DWORD get_icon_from_file(common_file *file)
{
	DWORD result;
	switch(file->type)
	{
	case FT_DIR:
		result = ICO_FB_FOLDER;
		break;
	case FT_SOCK:
		result = ICO_FB_SOCKFILE;
		break;
	case FT_LNK:
		result = ICO_FB_LNKFILE;
		break;
	case FT_REG:
		result = ICO_FB_REGFILE;
		break;
	case FT_BLK:
		result = ICO_FB_BLOCKFILE;
		break;
	case FT_CHR:
		result = ICO_FB_CHARFILE;
		break;
	case FT_FIFO:
		result = ICO_FB_FIFOFILE;
		break;
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////

char *get_attr_from_file(common_file *file)
{
	static char s[256];
	lstrcpy(s, "----------");
	DWORD rights = file->mode;


	if (rights & S_IXOTH) s[9] = 'x';
	if (rights & S_IWOTH) s[8] = 'w';
	if (rights & S_IROTH) s[7] = 'r';

	if (rights & S_IXGRP) s[6] = 'x';
	if (rights & S_IWGRP) s[5] = 'w';
	if (rights & S_IRGRP) s[4] = 'r';

	if (rights & S_IXUSR) s[3] = 'x';
	if (rights & S_IWUSR) s[2] = 'w';
	if (rights & S_IRUSR) s[1] = 'r';

	switch(file->type)
	{
	case FT_REG:
		break;
	case FT_DIR:
		s[0] = 'd';
		break;
	case FT_LNK:
		s[0] = 'l';
		break;
	case FT_BLK:
		s[0] = 'b';
		break;
	case FT_CHR:
		s[0] = 'c';
		break;
	case FT_SOCK:
		s[0] = 's';
		break;
	case FT_FIFO:
		s[0] = 'f';
		break;
	}

	return s;
}

/////////////////////////////////////////////////////////////////////////

char *get_type_from_file(common_file *file)
{
	char *result;
	switch(file->type)
	{
	case FT_REG:
		result = Message(MSG_FILETYPE_REG);
		break;
	case FT_DIR:
		result = Message(MSG_FILETYPE_DIR);
		break;
	case FT_BLK:
		result = Message(MSG_FILETYPE_BLK);
		break;
	case FT_CHR:
		result = Message(MSG_FILETYPE_CHR);
		break;
	case FT_LNK:
		result = Message(MSG_FILETYPE_LNK);
		break;
	case FT_FIFO:
		result = Message(MSG_FILETYPE_FIFO);
		break;
	case FT_SOCK:
		result = Message(MSG_FILETYPE_SOCK);
		break;
	default:
		assert(0);
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////

char *get_size_from_file(common_file *file)
{
	static char s[256];
	if (file->i_size < 1024*1024/*<1M*/)
	{
		wsprintf(s, "%lu", file->i_size);
	}
	else if (file->i_size < 1024*1024*1024/*1G*/)
	{
		wsprintf(s, "%lu K", file->i_size>>10);
	}
	else
	{
		wsprintf(s, "%lu M", file->i_size>>20);
	}
	return s;
}

/////////////////////////////////////////////////////////////////////////

char *time_to_string(time_t t)
{
	static char s[256];
	tm *time = gmtime(&t);
	if (tm)	strftime(s, 256, "%d.%m.%y %H:%M", time);
	else wsprintf(s, Message(MSG_UNKNOWN));
	return s;
}

/////////////////////////////////////////////////////////////////////////

char *group_to_string(DWORD user)
{
	static char s[256];
	wsprintf(s, "%lu", user);
	return s;
}

/////////////////////////////////////////////////////////////////////////

char *user_to_string(DWORD group)
{
	static char s[256];
	wsprintf(s, "%lu", group);
	return s;
}

/////////////////////////////////////////////////////////////////////////
