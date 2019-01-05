#include "stdafx.h"
#include "drgndrp.h"
#include <shlobj.h>
#include "debug.h"

/////////////////////////////////////////////////////////////////
//                        CDropSource
/////////////////////////////////////////////////////////////////

CDropSource::CDropSource() : m_cRef(0)
{
    
}

/////////////////////////////////////////////////////////////////

CDropSource::~CDropSource()
{
    TR("CDropSource::~CDropSource()");
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDropSource::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;
    
    HRESULT hr = E_NOINTERFACE;
    if (riid == IID_IUnknown || riid == IID_IDropSource)
    {
        *ppv = this;
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    
    return hr;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CDropSource::AddRef()
{
    TR("CDropSource::AddRef()");
    
    return ++m_cRef;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CDropSource::Release()
{
    TR("CDropSource::Release()");
    
    DWORD cRef = --m_cRef;
    if (cRef == 0L) 
		delete this;
    
    return cRef;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEsc, DWORD grfKeyState)
{
    if (fEsc) 
		return DRAGDROP_S_CANCEL;
    if (!(grfKeyState & MK_LBUTTON))  
		return DRAGDROP_S_DROP;
    return S_OK;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
    //TR("CDropSource::GiveFeedback");
    
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


/////////////////////////////////////////////////////////////////
//                        CDataObject
/////////////////////////////////////////////////////////////////

CDataObject::CDataObject() : m_cRef(0)
{
    m_dwCfPerformedDropEffect = RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);    
}

/////////////////////////////////////////////////////////////////

CDataObject::~CDataObject()
{
    TR("CDataObject::~CDataObject");
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;
    
    HRESULT hr = E_NOINTERFACE;
    if (riid == IID_IUnknown || riid == IID_IDataObject)
    {
        *ppv = this;
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    
    return hr;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CDataObject::AddRef()
{
    TR("CDataObject:AddRef()");
    
    return ++m_cRef;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CDataObject::Release()
{
    TR("CDataObject:Release()");
    
    DWORD cRef = --m_cRef;
    if (cRef == 0L)  
		delete this;
    
    return cRef;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    TR("CDataObject::GetData");
    
    if (pFE->cfFormat != CF_HDROP) 
		return S_FALSE;
    
    static char s[] = "c:\\config.sys\0";
    
    HGLOBAL hMem = GlobalAlloc(GHND, sizeof(DROPFILES) + 150);
    
    DROPFILES *DropFiles;
    DropFiles = (DROPFILES *)GlobalLock(hMem);
    
    MoveMemory((char *)DropFiles + sizeof(DROPFILES), s, lstrlen(s) + 2);
    DropFiles->pFiles = sizeof(DROPFILES);
    DropFiles->pt.x = 0; DropFiles->pt.x = 0;
    DropFiles->fWide = FALSE;
    
    GlobalUnlock(hMem);
    
    pSTM->tymed = TYMED_HGLOBAL;
    pSTM->hGlobal = hMem;
    pSTM->pUnkForRelease = NULL;
    
    return S_OK;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    TR("CDataObject::GetDataHere");
    
    return DV_E_FORMATETC; // really not implemented
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC pFE)
{
    TR("CDataObject::QueryGetData");
    
    TR(pFE->cfFormat);
    TR(pFE->dwAspect);
    TR(pFE->lindex);
    TR((DWORD)pFE->ptd);
    TR(pFE->tymed);
    
    HRESULT hr = DATA_E_FORMATETC;
    if (pFE->cfFormat == CF_HDROP)
		hr = S_OK;
    
    return hr;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn,
                                                LPFORMATETC pFEOut)
{
    TR("CDataObject::GetCanonicalFormatEtc");
    
    HRESULT hr;
    if (pFEOut == NULL)
		hr = E_INVALIDARG;
    else 
    {
        pFEOut->ptd = NULL;
        hr = DATA_S_SAMEFORMATETC;
    }
    
    return hr; 
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::SetData(LPFORMATETC pFE,
                                  LPSTGMEDIUM pSTM, BOOL fRelease)
{
    TR("CDataObject::SetData");

    HRESULT hr = E_NOTIMPL; 
    if (pFE->cfFormat == m_dwCfPerformedDropEffect)
    {
        DWORD *dwEffect = (DWORD *)GlobalLock(pSTM->hGlobal);
        m_dwDropEffect = *dwEffect;
        TR("SetDataEffect");
        TR(*dwEffect);
        GlobalUnlock(pSTM->hGlobal);
        hr = S_OK;
    }

    return hr;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDir,
                                        LPENUMFORMATETC *ppEnum)
{
    TR("CDataObject::EnumFormatEtc");
    
    static FORMATETC format;
    CEnumFORMATETC *pEnumFE = NULL;
    
    if (dwDir == DATADIR_GET)
    {
        format.cfFormat = CF_HDROP;
        format.dwAspect = DVASPECT_CONTENT;
        format.lindex   = -1;
        format.ptd      = 0; // not used
        format.tymed    = TYMED_HGLOBAL;
        
        pEnumFE = new CEnumFORMATETC(&format, 1);
    }
    
    if (pEnumFE)
    {
        *ppEnum = pEnumFE;
       ((IUnknown*)*ppEnum)->AddRef();
    }
    
    return pEnumFE  ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::DAdvise(LPFORMATETC pFE, DWORD dwFlags,
                                  LPADVISESINK pIAdviseSink, LPDWORD pdwConn)
{
    TR("CDataObject::DAdvise");
    
    return OLE_E_ADVISENOTSUPPORTED;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::DUnadvise(DWORD dwConn)
{
    TR("CDataObject::DUnadvise");
    
    return OLE_E_ADVISENOTSUPPORTED;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CDataObject::EnumDAdvise(LPENUMSTATDATA *ppEnum)
{
    TR("CDataObject::EnumDAdvise");
    
    return OLE_E_ADVISENOTSUPPORTED;
}

/////////////////////////////////////////////////////////////////
//                       CEnumFORMATETC
/////////////////////////////////////////////////////////////////

CEnumFORMATETC::CEnumFORMATETC(FORMATETC *pData, DWORD cNum) : m_cRef(0)
{
    m_iCur = 0;
    m_cNum = cNum;
    m_pData = new FORMATETC[cNum];
    for (DWORD i = 0; i < cNum; i++)
    {
        ASSERT(!pData[i].ptd);
        m_pData[i] = pData[i];
    }
}

/////////////////////////////////////////////////////////////////

CEnumFORMATETC::~CEnumFORMATETC()
{
    delete [] m_pData;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumFORMATETC::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;
    
    HRESULT hr = E_NOINTERFACE;
    if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC)
    {
        *ppv = this;
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    
    return hr;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CEnumFORMATETC::AddRef()
{
    TR("CEnumFORMATETC::AddRef");
    
    return ++m_cRef;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG) CEnumFORMATETC::Release()
{
    TR("CEnumFORMATETC::Release");
    
    DWORD cRef = --m_cRef;
    if (cRef == 0L) 
		delete this;
    
    return cRef;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumFORMATETC::Next(ULONG cElt, FORMATETC *rgElt, ULONG *pceltFetched)
{
    TR("CEnumFORMATETC::Next");
    
    DWORD nFetched = 0;
    if (m_iCur < m_cNum)
    {
        while (m_iCur < m_cNum)
            rgElt[nFetched++] = m_pData[m_iCur++];
    }
    
    if (pceltFetched) 
		*pceltFetched = nFetched;
    
    return nFetched == cElt ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumFORMATETC::Skip(ULONG cElt)
{
    HRESULT hr = S_FALSE;
    if (m_iCur + cElt < m_cNum)
    {
        m_iCur += cElt;
        hr = S_OK;
    }
    return hr;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumFORMATETC::Reset()
{
    TR("CEnumFORMATETC::Reset");
    
    m_iCur = 0;
    return S_OK;
}

/////////////////////////////////////////////////////////////////

STDMETHODIMP CEnumFORMATETC::Clone(IEnumFORMATETC **ppIEnum)
{
    TR("CEnumFORMATETC::Clone");
    
    CEnumFORMATETC *pEnumFE = new CEnumFORMATETC(m_pData, m_cNum);
    
    HRESULT hr = E_OUTOFMEMORY;
    if (pEnumFE)
    {
        pEnumFE->m_iCur = m_iCur;
        *ppIEnum = pEnumFE;
        (*ppIEnum)->AddRef();
        hr = S_OK;
    }
    
    return hr;
}

/////////////////////////////////////////////////////////////////
//						CDataCache
/////////////////////////////////////////////////////////////////

CDataCache::CDataCache()
{
    m_iNext = 0;
}

/////////////////////////////////////////////////////////////////

CDataCache::~CDataCache()
{

}

/////////////////////////////////////////////////////////////////

BOOL CDataCache::CacheGlobalData(CLIPFORMAT cfFormat, HGLOBAL hMem)
{
    BOOL bResult = FALSE;

    FORMATETC format;
    switch (cfFormat)
    {
    case CF_HDROP:
        format.ptd = NULL;
        format.cfFormat = cfFormat;
        format.tymed = TYMED_HGLOBAL;
        format.lindex = -1;
        format.dwAspect = DVASPECT_CONTENT;;
        bResult = TRUE;

        break;
    }
    
    if (bResult)
    {
        if (m_iNext < CACHE_SIZE)
        {
            m_fCache[m_iNext++].format = format;
            m_fCache[m_iNext++].hMem = hMem;
        } 
		else 
			bResult = FALSE;
    }
    
    return bResult;
}

/////////////////////////////////////////////////////////////////

CacheData *CDataCache::Lookup(LPFORMATETC lpFormatEtc)
{
    CacheData *pCacheData = NULL;
    for (int i = 0; i < m_iNext; i++)
    {
        FORMATETC *format = &m_fCache[i].format;
        if (
            lpFormatEtc->cfFormat == format->cfFormat &&
            lpFormatEtc->dwAspect == format->dwAspect &&
            lpFormatEtc->lindex   == format->lindex &&
            lpFormatEtc->tymed == format->tymed
            ) 
        {
            pCacheData = m_fCache + i;
            break;
        }
    }
    return pCacheData;
}

/////////////////////////////////////////////////////////////////

void CDataCache::Flush()
{
    m_iNext = 0;
}

/////////////////////////////////////////////////////////////////
//						CDragManager
/////////////////////////////////////////////////////////////////

CFileDragManager::CFileDragManager()
{
    // check OS version
    OSVERSIONINFO info;
    GetVersionEx(&info);
    m_bOnNT = info.dwPlatformId == VER_PLATFORM_WIN32_NT;

    OleInitialize(NULL);
}

/////////////////////////////////////////////////////////////////

CFileDragManager::~CFileDragManager()
{
    OleUninitialize();
}

/////////////////////////////////////////////////////////////////

BOOL CFileDragManager::DoDragFiles(BOOL bMove, LPCTSTR ppFileName[],
                                   DWORD cNumOfFiles)
{
    UINT uSize = 0;
    for (DWORD i = 0; i < cNumOfFiles; i++)
        uSize += lstrlen(ppFileName[i]) + 1;

    uSize = sizeof(DROPFILES) + (uSize + 1)*sizeof(TCHAR);

    HGLOBAL hMem = GlobalAlloc(GHND,uSize);
    DROPFILES *pDrop = (DROPFILES *)GlobalLock(hMem);
    pDrop->pFiles = sizeof(DROPFILES);

#ifdef _UNICODE
    pDrop->fWide = TRUE;
#endif

    TCHAR *pszBuff = (TCHAR *)((LPBYTE)pDrop + sizeof(DROPFILES));
    for (i = 0; i < cNumOfFiles; i++)
    {
        lstrcpy(pszBuff, ppFileName[i]);
        pszBuff = _tcschr(pszBuff, 0) + 1;
    }
    GlobalLock(hMem);

    return FALSE;
}

/////////////////////////////////////////////////////////////////

