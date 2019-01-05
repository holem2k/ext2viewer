#ifndef __REGISTRY_H__
#define __REGISTRY_H__

BOOL reg_getnum(const char *name, DWORD *value);
BOOL reg_setnum(const char *name, DWORD value);
BOOL reg_getstr(const char *name, char *buffer, DWORD buflen);
BOOL reg_setstr(const char *name, const char *buffer);
BOOL reg_setbin(const char *name, BYTE *buf, DWORD buflen);
BOOL reg_getbin(const char *name, BYTE *buf, DWORD buflen);
BOOL reg_delval(const char *name);

#endif //__REGISTRY_H__