#include <stdafx.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////

HIMAGELIST CreateIconList(const int Icon[], int iIconsNum, BOOL bLarge)
{
	//int iSize = bLarge ? GetSystemMetrics(SM_CXICON) :GetSystemMetrics(SM_CXSMICON);
	int iSize = bLarge ? 32 : 16;
	HIMAGELIST himlIcons = ImageList_Create(iSize, iSize, ILC_COLOR8 | ILC_MASK,  16, 8);
	assert(himlIcons);

	if (himlIcons)
	{
		for (int i = 0; i < iIconsNum; i++)
		{
			HICON hIcon;
			if (bLarge) 
                hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(Icon[i]));
			else
                hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(Icon[i]), IMAGE_ICON, iSize, iSize, LR_DEFAULTCOLOR);

			assert(hIcon);
			ImageList_AddIcon(himlIcons, hIcon);
		}
	}
	return himlIcons;
}

////////////////////////////////////////////////////////////////////////////
