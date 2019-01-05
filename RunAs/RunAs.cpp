// RunAs.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "malloc.h"

const TCHAR lpApplication[] = TEXT("ext2viewer.exe");

BOOL IsUnderAdmin()
{
	BOOL bResult = FALSE;
	HANDLE hAccessToken = INVALID_HANDLE_VALUE;
	PSID psidAdministrators = NULL;

	__try 
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hAccessToken))
			__leave;

		TOKEN_GROUPS *ptg;
		DWORD dwInfoLen;

		DWORD cbTokenGroups;
		if (GetTokenInformation(hAccessToken, TokenGroups, NULL, 0, &cbTokenGroups))
			__leave;
		else
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				__leave;
		}

		ptg = (TOKEN_GROUPS *)_alloca(cbTokenGroups);
		if (!ptg)
			__leave;

		if (!GetTokenInformation(hAccessToken, TokenGroups, ptg, cbTokenGroups, &dwInfoLen))
			__leave;

		SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
		if (!AllocateAndInitializeSid(&siaNtAuthority, 2,
					SECURITY_BUILTIN_DOMAIN_RID,
					DOMAIN_ALIAS_RID_ADMINS,
					0, 0, 0, 0, 0, 0,
					&psidAdministrators))
		{
			__leave;
		}

		for(UINT i = 0; i < ptg->GroupCount; i ++)
		{
			if (EqualSid(psidAdministrators, ptg->Groups[i].Sid))
			{
				bResult = TRUE;
				break;
			}
		}
	}
	__finally
	{
		if (psidAdministrators)
			FreeSid(psidAdministrators);
		if (hAccessToken != INVALID_HANDLE_VALUE)
			CloseHandle(hAccessToken);
	}
	return bResult;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionEx(&info);
	if (info.dwPlatformId == VER_PLATFORM_WIN32_NT && info.dwMajorVersion >= 5 && !IsUnderAdmin())
	{
		SHELLEXECUTEINFO exinfo;
		ZeroMemory(&exinfo, sizeof(exinfo));
		exinfo.cbSize = sizeof(exinfo);
		exinfo.lpFile = lpApplication;
		exinfo.lpVerb = TEXT("runas");
		exinfo.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&exinfo);
	}
	else
		ShellExecute(0, TEXT("open"), lpApplication, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}



