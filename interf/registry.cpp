#include <stdafx.h>


/////////////////////////////////////////////////////////////////////////

BOOL reg_getnum(const char *name, DWORD *value)
{
    HKEY hKey;
    LONG Result = RegOpenKeyEx(hHive, lpSubKey, 0, KEY_ALL_ACCESS, &hKey);

    BOOL bResult = FALSE;
    if (Result == ERROR_SUCCESS)
    {
        DWORD cbSize = sizeof(DWORD);
        DWORD dwType = REG_DWORD;

        if (RegQueryValueEx(hKey, name, 0, 
            &dwType, (LPBYTE)value, &cbSize) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////

BOOL reg_setnum(const char *name, DWORD value)
{
    HKEY hKey;

    LONG Result = RegCreateKeyEx(hHive, lpSubKey, 0, NULL,
        0, KEY_ALL_ACCESS, NULL, &hKey, NULL);

    BOOL bResult = FALSE;
    if (Result == ERROR_SUCCESS)
    {
        if(RegSetValueEx(hKey, name, 0,
            REG_DWORD, (LPBYTE)&value, sizeof(value)) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////

BOOL reg_getstr(const char *name, char *buffer, DWORD buflen)
{
    HKEY hKey;
    LONG Result = RegOpenKeyEx(hHive, lpSubKey, 0, KEY_ALL_ACCESS, &hKey);

    BOOL bResult = FALSE;
    if (Result == ERROR_SUCCESS)
    {
        DWORD cbSize = buflen;
        DWORD dwType = REG_SZ;

        if (RegQueryValueEx(hKey, name, 0, 
            &dwType, (LPBYTE)buffer, &cbSize) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////

BOOL reg_setstr(const char *name, const char *buffer)
{
    BOOL bResult = FALSE;

    HKEY hKey;
    LONG Result = RegCreateKeyEx(hHive, lpSubKey, 0, NULL,
        0, KEY_ALL_ACCESS, NULL, &hKey, NULL);

    if (Result == ERROR_SUCCESS)
    {
        if(RegSetValueEx(hKey, name, 0,
            REG_SZ, (LPBYTE)buffer, lstrlen(buffer) + 1) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////

BOOL reg_delval(const char *name)
{
    BOOL bResult = FALSE;

    HKEY hKey;
    LONG Result = RegOpenKeyEx(hHive, lpSubKey, 0, KEY_ALL_ACCESS, &hKey);

    if (Result == ERROR_SUCCESS)
    {
        bResult = RegDeleteValue(hKey, name) == ERROR_SUCCESS;
        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////

BOOL reg_setbin(const char *name, BYTE *buffer, DWORD buflen)
{
    BOOL bResult = FALSE;

    HKEY hKey;
    LONG Result = RegCreateKeyEx(hHive, lpSubKey, 0, NULL,
        0, KEY_ALL_ACCESS, NULL, &hKey, NULL);

    if (Result == ERROR_SUCCESS)
    {
        if(RegSetValueEx(hKey, name, 0,
            REG_BINARY, (LPBYTE)buffer, buflen) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////

BOOL reg_getbin(const char *name, BYTE *buffer, DWORD buflen)
{
	BOOL bResult = FALSE;

    HKEY hKey;
    LONG Result = RegOpenKeyEx(hHive, lpSubKey, 0, KEY_ALL_ACCESS, &hKey);

    if (Result == ERROR_SUCCESS)
    {
        DWORD cbSize = buflen;
        DWORD dwType = REG_BINARY;

        if (RegQueryValueEx(hKey, name, 0, 
            &dwType, (LPBYTE)buffer, &cbSize) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////