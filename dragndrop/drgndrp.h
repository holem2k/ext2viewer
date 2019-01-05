#ifndef __DRGNDRP_H__
#define __DRGNDRP_H__

#include "oleidl.h"

// definition of:
class CDropSource;
class CDataObject;
class CEnumFORMATETC;
class CFileDragManager;
class CDataCache;

/////////////////////////////////////////////////////////////////
//						CDropSource
/////////////////////////////////////////////////////////////////

class CDropSource:  public IDropSource
{
public:
	CDropSource();
	~CDropSource();

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID, LPVOID *);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	
	// IDropSource
	STDMETHOD(QueryContinueDrag)(BOOL, DWORD);
	STDMETHOD(GiveFeedback)(DWORD);

private:
    DWORD m_cRef;
};

/////////////////////////////////////////////////////////////////
//						CDataCache
/////////////////////////////////////////////////////////////////

const DWORD CACHE_SIZE = 4;
typedef struct tag_CacheData
{
    FORMATETC format;
    HGLOBAL hMem;
} CacheData;

class CDataCache
{
public:
    CDataCache();
    ~CDataCache();
    BOOL CacheGlobalData(CLIPFORMAT cfFormat, HGLOBAL hGlobal);
    CacheData *CDataCache::Lookup(LPFORMATETC lpFormatEtc);
    void Flush();

private:
    CacheData m_fCache[CACHE_SIZE];
    int m_iNext;
};

/////////////////////////////////////////////////////////////////
//						CDataObject
/////////////////////////////////////////////////////////////////

class CDataObject:  public IDataObject
{
public:
	CDataObject();
	~CDataObject();

	//  IUnknown
	STDMETHOD(QueryInterface)(REFIID, LPVOID *);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IDataObject
    STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
    STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
	STDMETHOD(QueryGetData)(LPFORMATETC);
	STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
	STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
	STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC *);
	STDMETHOD(DAdvise)(LPFORMATETC, DWORD,  LPADVISESINK, DWORD *);
	STDMETHOD(DUnadvise)(DWORD);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA *);

    // 
    DWORD DropEffect() { return m_dwDropEffect; };
private:
    DWORD m_cRef;
    DWORD m_dwCfPerformedDropEffect;
    DWORD m_dwDropEffect;
    /*
    ^^^^^
    Если DoDragDrop вызывается с параметром DROPEFFECT_MOVE
    shell  может оптимизировать move operation и удалит
    источник данных с диска. В этом случае ::DoDragDrop и
    CDataObject::DropEffect вернут DROPEFFECT_NONE.
    Программа должна удалить данные, если только оба
    значения = DROPEFFECT_MOVE.
    P.S В NT в данном случае DoDragDrop  ВСЕГДА возвр. DROPEFFECT_NONE.
    */
    CDataCache m_DataCache;
};

/////////////////////////////////////////////////////////////////
//						CEnumFORMATETC
/////////////////////////////////////////////////////////////////

class CEnumFORMATETC: public IEnumFORMATETC
{
public:
	CEnumFORMATETC(FORMATETC *pData, DWORD cNum	);
	~CEnumFORMATETC();
	
	// IUnknown 
	STDMETHOD(QueryInterface)(REFIID, LPVOID *);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	
	// IEnumFORMATETC 
	STDMETHOD(Next)(ULONG, FORMATETC *, ULONG *);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumFORMATETC **);

private:
    DWORD m_cRef;

	FORMATETC *m_pData;
	DWORD m_cNum;
	DWORD m_iCur;
};


    

/////////////////////////////////////////////////////////////////
//						CDragManager
/////////////////////////////////////////////////////////////////

class  CFileDragManager
{
public:
    CFileDragManager();
    ~CFileDragManager();
    BOOL DoDragFiles(BOOL bMove, LPCTSTR ppFileName[], DWORD cNumOfFiles);

private:
    BOOL m_bOnNT;
};

#endif //__DRGNDRP_H__