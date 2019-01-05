#include <stdafx.h>
#include "stat.h"
#include "text.h"
#include "string.h"

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
	if (file->type == FT_BLK || file->type == FT_CHR)
	{
		wsprintf(s, "%u,", file->dev>>8);
		char s2[8];
		wsprintf(s2, "%u", file->dev&0xFF);
		for (int i = 0; i < 4 - lstrlen(s2); i++)
			lstrcat(s, "  ");
		lstrcat(s, s2);
	}
	else
	{
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
	}
	return s;
}

/////////////////////////////////////////////////////////////////////////

char *time_to_string(time_t t)
{
	static char s[256];
	tm *time = localtime(&t);
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

char *get_sgid_from_file(common_file *file)
{
	static char s[256];
	wsprintf(s, "%lu", file->mode & S_ISGID);
	return s;
}

/////////////////////////////////////////////////////////////////////////

char *get_suid_from_file(common_file *file)
{
	static char s[256];
	wsprintf(s, "%lu", file->mode & S_ISUID);
	return s;
}

/////////////////////////////////////////////////////////////////////////

char *get_fs_state(mount_info *mi)
{
	static char s[256];
	if (mi->state & EXT2_VALID_FS == 0 || mi->state & EXT2_ERROR_FS)
	wsprintf(s, Message(MSG_FSERRORS));
	else wsprintf(s, Message(MSG_NOFSERRORS));
	return s;
}
/////////////////////////////////////////////////////////////////////////

char *size_to_spaced_string(DWORD size)
{
	static char s[256];
	char l[256];
	BYTE cyfr;

	DWORD k = 0; DWORD lp = 0;
	do
	{
		cyfr = (BYTE)(size%10);
		size /= 10;
		if (k++ % 3 == 0 && lp) 
			l[lp++] = ' ';
		l[lp++] = '0' + cyfr;
	} while (size > 0);
	l[lp] = 0;

	wsprintf(s, "%s %s",  _strrev(l), Message(MSG_BYTE));

	return s;
}


/////////////////////////////////////////////////////////////////////////

char *size_to_string(DWORD size)
{
	static char s[256];
	if (size < 1024)
	{
		wsprintf(s, "%lu %s", size, Message(MSG_BYTE));
	}
	else if (size < 1024*1024)
	{
		wsprintf(s, "%lu,%lu K (%s)", size>>10, (int)((size/1024.0 - (size>>10))*10), size_to_spaced_string(size));
	}
	else
	{
		wsprintf(s, "%lu,%lu M (%s)", size>>20, (int)((size/1024.0/1024.0 - (size>>20))*10), size_to_spaced_string(size));
	}

	return s;
}

/////////////////////////////////////////////////////////////////////////

TCHAR *foldersize_to_string(DWORD size)
{
	static TCHAR s[256];

    s[0] = 0;
    float fsize = (float)size;
    for (int i = 0; i < 3; i++)
    {
        TCHAR *lpSizeSym1;
        TCHAR *lpSizeSym2;
        switch (i)
        {
        case 0:
            lpSizeSym1 = Message(MSG_BYTE);
            lpSizeSym2 = Message(MSG_KILOBYTE);
            break;
        case 1:
            lpSizeSym1 = Message(MSG_KILOBYTE);
            lpSizeSym2 = Message(MSG_MEGABYTE);
            break;
        case 2:
            lpSizeSym1 = Message(MSG_MEGABYTE);
            lpSizeSym2 = Message(MSG_GIGABYTE);
            break;
        }
        
        if (fsize < 1024) // меньше килобайта
        {
            wsprintf(s, "%lu %s", (int)fsize, lpSizeSym1);
        }
        else if (fsize < 1024*10) // меньше 10 килобайт
        {
            wsprintf(s, "%lu,%lu %s",
                ((int)fsize)>>10, (int)((fsize/1024.0 - (((int)fsize)>>10))*100), lpSizeSym2);
        }
        else if (fsize < 1024*100)  // меньше 100 килобайт
        {
            wsprintf(s, "%lu,%lu %s",
                ((int)fsize>>10), (int)((fsize/1024.0 - (((int)fsize)>>10))*10), lpSizeSym2);
        }
        else if (fsize < 1024*1024)  // меньше мегабайта
        {
            wsprintf(s, "%lu %s", ((int)fsize)>>10, lpSizeSym2);
        }

        if (s[0]) break;
        fsize /= 1024;
    }

    return s;
}

/////////////////////////////////////////////////////////////////////////

void time_to_filetime(time_t t, LPFILETIME pft)
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = (DWORD)(ll>>32);
}

/////////////////////////////////////////////////////////////////////////

