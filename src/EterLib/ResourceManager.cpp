#include "StdAfx.h"
#include <io.h>
#include "EterBase/CRC32.h"
#include "EterBase/Timer.h"
#include "EterBase/Stl.h"
#include "PackLib/PackManager.h"

#include "ResourceManager.h"
#include "GrpImage.h"
#include "TextureCache.h"
#include "DecodedImageData.h"

int g_iLoadingDelayTime =
    1; // Reduced from 20ms to 1ms for faster async loading

const long c_Deleting_Wait_Time = 30000; // 삭제 대기 시간 (30초)
const long c_DeletingCountPerFrame = 30; // 프레임당 체크 리소스 갯수
const long c_Reference_Decrease_Wait_Time =
    30000; // 선로딩 리소스의 해제 대기 시간 (30초)

CFileLoaderThread CResourceManager::ms_loadingThread;

void CResourceManager::BeginThreadLoading() {
  // Already started in constructor, nothing to do
}

void CResourceManager::EndThreadLoading() {
  // Wait for all pending requests to complete
  while (!m_RequestMap.empty() || !m_WaitingMap.empty()) {
    ProcessBackgroundLoading();
    Sleep(10);
  }
}

void CResourceManager::LoadStaticCache(const char *c_szFileName) {
  CResource *pkRes = GetResourcePointer(c_szFileName);
  if (!pkRes) {
    Lognf(1, "CResourceManager::LoadStaticCache %s - FAILED", c_szFileName);
    return;
  }

  DWORD dwCacheKey = GetCRC32(c_szFileName, strlen(c_szFileName));
  TResourcePointerMap::iterator f = m_pCacheMap.find(dwCacheKey);
  if (m_pCacheMap.end() != f)
    return;

  pkRes->AddReference();
  m_pCacheMap.insert(TResourcePointerMap::value_type(dwCacheKey, pkRes));
}

void CResourceManager::ProcessBackgroundLoading() {
  TResourceRequestMap::iterator itor = m_RequestMap.begin();

  while (itor != m_RequestMap.end()) {
    DWORD dwFileCRC = itor->first;
    std::string &stFileName = itor->second;

    if (isResourcePointerData(dwFileCRC) ||
        (m_WaitingMap.end() != m_WaitingMap.find(dwFileCRC))) {
      // printf("SKP %s\n", stFileName.c_str());
      itor = m_RequestMap.erase(itor);
      continue;
    }

    // printf("REQ %s\n", stFileName.c_str());

    ms_loadingThread.Request(stFileName);

    m_WaitingMap.insert(TResourceRequestMap::value_type(dwFileCRC, stFileName));
    itor = m_RequestMap.erase(itor);
    // break; // NOTE: 여기서 break 하면 천천히 로딩 된다.
  }

  DWORD dwCurrentTime = ELTimer_GetMSec();

  // Process thread results
  CFileLoaderThread::TData *pData;
  while (ms_loadingThread.Fetch(&pData)) {
    // printf("LOD %s\n", pData->stFileName.c_str());
    CResource *pResource = GetResourcePointer(pData->stFileName.c_str());

    if (pResource) {
      if (pResource->IsEmpty()) {
        pResource->OnLoad(pData->File.size(), pData->File.data());
        pResource->AddReferenceOnly();

        // 여기서 올라간 레퍼런스 카운트를 일정 시간이 지난 뒤에 풀어주기 위하여
        m_pResRefDecreaseWaitingMap.insert(
            TResourceRefDecreaseWaitingMap::value_type(dwCurrentTime,
                                                       pResource));
      }
    }

    m_WaitingMap.erase(
        GetCRC32(pData->stFileName.c_str(), pData->stFileName.size()));

    delete pData;
  }

  // DO : 일정 시간이 지나고 난뒤 미리 로딩해 두었던 리소스의 레퍼런스 카운트를
  // 감소 시킨다 - [levites]
  long lCurrentTime = ELTimer_GetMSec();

  TResourceRefDecreaseWaitingMap::iterator itorRef =
      m_pResRefDecreaseWaitingMap.begin();

  while (itorRef != m_pResRefDecreaseWaitingMap.end()) {
    const long &rCreatingTime = itorRef->first;

    if (lCurrentTime - rCreatingTime > c_Reference_Decrease_Wait_Time) {
      CResource *pResource = itorRef->second;

      // Decrease Reference Count
      pResource->Release();
      itorRef = m_pResRefDecreaseWaitingMap.erase(itorRef);
      // Tracef("Decrease Pre Loading Resource\n", rCreatingTime);
    } else
      ++itorRef;
  }
}

void CResourceManager::PushBackgroundLoadingSet(
    std::set<std::string> &LoadingSet) {
  std::set<std::string>::iterator itor = LoadingSet.begin();

  while (itor != LoadingSet.end()) {
    DWORD dwFileCRC = __GetFileCRC(itor->c_str());

    if (NULL != isResourcePointerData(dwFileCRC)) {
      ++itor;
      continue;
    }

    m_RequestMap.insert(
        TResourceRequestMap::value_type(dwFileCRC, itor->c_str()));
    ++itor;
  }
}

void CResourceManager::__DestroyCacheMap() {
  TResourcePointerMap::iterator i;
  for (i = m_pCacheMap.begin(); i != m_pCacheMap.end(); ++i) {
    CResource *pResource = i->second;
    pResource->Release();
  }

  m_pCacheMap.clear();
}

void CResourceManager::__DestroyDeletingResourceMap() {
  Tracenf("CResourceManager::__DestroyDeletingResourceMap %d",
          m_ResourceDeletingMap.size());
  for (TResourceDeletingMap::iterator i = m_ResourceDeletingMap.begin();
       i != m_ResourceDeletingMap.end(); ++i)
    (i->first)->Clear();

  m_ResourceDeletingMap.clear();
}

void CResourceManager::__DestroyResourceMap() {
  Tracenf("CResourceManager::__DestroyResourceMap %d", m_pResMap.size());

  TResourcePointerMap::iterator i;
  for (i = m_pResMap.begin(); i != m_pResMap.end(); ++i) {
    CResource *pResource = i->second;
    pResource->Clear();
  }

  stl_wipe_second(m_pResMap);
}

void CResourceManager::DestroyDeletingList() {
  CResource::SetDeleteImmediately(true);

  __DestroyCacheMap();
  __DestroyDeletingResourceMap();
}

void CResourceManager::Destroy() {
  assert(m_ResourceDeletingMap.empty() &&
         "CResourceManager::Destroy - YOU MUST CALL DestroyDeletingList");
  __DestroyResourceMap();
}

void CResourceManager::RegisterResourceNewFunctionPointer(
    const char *c_szFileExt, CResource *(*pNewFunc)(const char *c_szFileName)) {
  m_pResNewFuncMap[c_szFileExt] = pNewFunc;
}

void CResourceManager::RegisterResourceNewFunctionByTypePointer(
    int iType, CResource *(*pNewFunc)(const char *c_szFileName)) {
  assert(iType >= 0);
  m_pResNewFuncByTypeMap[iType] = pNewFunc;
}

CResource *CResourceManager::InsertResourcePointer(DWORD dwFileCRC,
                                                   CResource *pResource) {
  // Thread-safe check and insert
  std::lock_guard<std::mutex> lock(m_ResourceMapMutex);

  TResourcePointerMap::iterator itor = m_pResMap.find(dwFileCRC);

  if (m_pResMap.end() != itor) {
    TraceError("CResource::InsertResourcePointer: %s is already registered\n",
               pResource->GetFileName());
    assert(!"CResource::InsertResourcePointer: Resource already resistered");
    delete pResource;
    return itor->second;
  }

  m_pResMap.insert(TResourcePointerMap::value_type(dwFileCRC, pResource));
  return pResource;
}

int __ConvertPathName(const char *c_szPathName, char *pszRetPathName,
                      int retLen) {
  const char *pc;
  int len = 0;

  for (pc = c_szPathName; *pc && len < retLen; ++pc, ++len) {
    if (*pc == '/')
      *(pszRetPathName++) = '\\';
    else
      *(pszRetPathName++) = (char)ascii_tolower(*pc);
  }

  *pszRetPathName = '\0';
  return len;
}

CResource *CResourceManager::GetTypeResourcePointer(const char *c_szFileName,
                                                    int iType) {
  if (!c_szFileName || !*c_szFileName) {
    assert(c_szFileName != NULL && *c_szFileName != '\0');
    return NULL;
  }

  const char *c_pszFile;
  DWORD dwFileCRC = __GetFileCRC(c_szFileName, &c_pszFile);
  CResource *pResource = FindResourcePointer(dwFileCRC);

  if (pResource) // 이미 리소스가 있으면 리턴 한다.
    return pResource;

  CResource *(*newFunc)(const char *) = NULL;

  if (iType != -1) {
    TResourceNewFunctionByTypePointerMap::iterator f =
        m_pResNewFuncByTypeMap.find(iType);

    if (m_pResNewFuncByTypeMap.end() != f)
      newFunc = f->second;
  } else {
    const char *pcFileExt = strrchr(c_pszFile, '.');

    if (pcFileExt) {
      static char s_szFileExt[8 + 1];
      strncpy(s_szFileExt, pcFileExt + 1, 8);

      TResourceNewFunctionPointerMap::iterator f =
          m_pResNewFuncMap.find(s_szFileExt);

      if (m_pResNewFuncMap.end() != f)
        newFunc = f->second;
    }
  }

  if (!newFunc) {
    TraceError("ResourceManager::GetResourcePointer: NOT SUPPORT FILE %s",
               c_pszFile);
    return NULL;
  }

  pResource = InsertResourcePointer(dwFileCRC, newFunc(c_pszFile));
  return pResource;
}

CResource *CResourceManager::GetResourcePointer(const char *c_szFileName) {
  if (!c_szFileName || !*c_szFileName) {
    TraceError("CResourceManager::GetResourcePointer: filename error!");
    return NULL;
  }

  const char *c_pszFile;
  DWORD dwFileCRC = __GetFileCRC(c_szFileName, &c_pszFile);
  CResource *pResource = FindResourcePointer(dwFileCRC);

  if (pResource) // 이미 리소스가 있으면 리턴 한다.
    return pResource;

  const char *pcFileExt = strrchr(c_pszFile, '.');

#ifdef _DEBUG
  if (!IsFileExist(c_szFileName)) {
    if (pcFileExt == NULL || (stricmp(pcFileExt, ".fnt") != 0)) {
      TraceError("CResourceManager::GetResourcePointer: File not exist %s",
                 c_szFileName);
    }
  }
#endif

  CResource *(*newFunc)(const char *) = NULL;

  if (pcFileExt) {
    static char s_szFileExt[8 + 1];
    strncpy(s_szFileExt, pcFileExt + 1, 8);

    TResourceNewFunctionPointerMap::iterator f =
        m_pResNewFuncMap.find(s_szFileExt);

    if (m_pResNewFuncMap.end() != f)
      newFunc = f->second;
  }

  if (!newFunc) {
    TraceError("ResourceManager::GetResourcePointer: NOT SUPPORT FILE %s",
               c_pszFile);
    return NULL;
  }

  pResource = InsertResourcePointer(dwFileCRC, newFunc(c_pszFile));
  return pResource;
}

CResource *CResourceManager::FindResourcePointer(DWORD dwFileCRC) {
  TResourcePointerMap::iterator itor = m_pResMap.find(dwFileCRC);

  if (m_pResMap.end() == itor)
    return NULL;

  return itor->second;
}

bool CResourceManager::isResourcePointerData(DWORD dwFileCRC) {
  TResourcePointerMap::iterator itor = m_pResMap.find(dwFileCRC);

  if (m_pResMap.end() == itor)
    return NULL;

  return (itor->second)->IsData();
}

DWORD CResourceManager::__GetFileCRC(const char *c_szFileName,
                                     const char **c_ppszLowerFileName) {
  static char s_szFullPathFileName[MAX_PATH];
  const char *src = c_szFileName;
  char *dst = s_szFullPathFileName;
  int len = 0;

  while (src[len]) {
    if (src[len] == '/')
      dst[len] = '\\';
    else
      dst[len] = (char)ascii_tolower(src[len]);

    ++len;
  }

  dst[len] = '\0';

  if (c_ppszLowerFileName)
    *c_ppszLowerFileName = &s_szFullPathFileName[0];

  return (GetCRC32(s_szFullPathFileName, len));
}

typedef struct SDumpData {
  const char *filename;
  float KB;
  DWORD cost;
} TDumpData;

bool DumpKBCompare(const TDumpData &lhs, const TDumpData &rhs) {
  return (lhs.KB > rhs.KB) ? true : false;
}

bool DumpCostCompare(const TDumpData &lhs, const TDumpData &rhs) {
  return (lhs.cost > rhs.cost) ? true : false;
}

struct FDumpPrint {
  FILE *m_fp;
  static float m_totalKB;

  void operator()(TDumpData &data) {
    m_totalKB += data.KB;
    fprintf(m_fp, "%6.1f %s\n", data.KB, data.filename);
  }
};

float FDumpPrint::m_totalKB;

struct FDumpCostPrint {
  FILE *m_fp;

  void operator()(TDumpData &data) {
    fprintf(m_fp, "%-4d %s\n", data.cost, data.filename);
  }
};

void CResourceManager::DumpFileListToTextFile(const char *c_szFileName) {
  std::vector<TDumpData> dumpVector;

  for (TResourcePointerMap::iterator i = m_pResMap.begin();
       i != m_pResMap.end(); ++i) {
    CResource *pResource = i->second;
    TDumpData data;

    if (pResource->IsEmpty())
      continue;

    data.filename = pResource->GetFileName();

    int filesize;

    const char *ext = strrchr(data.filename, '.');

    if (pResource->IsType(CGraphicImage::Type()) && strnicmp(ext, ".sub", 4))
      filesize = ((CGraphicImage *)pResource)->GetWidth() *
                 ((CGraphicImage *)pResource)->GetHeight() * 4;
    else {
      FILE *fp2 = fopen(data.filename, "rb");

      if (fp2) {
        fseek(fp2, 0L, SEEK_END);
        filesize = ftell(fp2);
        fclose(fp2);
      } else
        filesize = 0;
    }

    data.KB = (float)filesize / (float)1024;
    data.cost = pResource->GetLoadCostMilliSecond();

    dumpVector.push_back(data);
  }

  FILE *fp = fopen(c_szFileName, "w");

  if (fp) {
    std::sort(dumpVector.begin(), dumpVector.end(), DumpKBCompare);

    FDumpPrint DumpPrint;
    DumpPrint.m_fp = fp;
    DumpPrint.m_totalKB = 0;

    std::for_each(dumpVector.begin(), dumpVector.end(), DumpPrint);
    fprintf(fp, "total: %.2fmb", DumpPrint.m_totalKB / 1024.0f);

    FDumpCostPrint DumpCostPrint;
    DumpCostPrint.m_fp = fp;

    std::sort(dumpVector.begin(), dumpVector.end(), DumpCostCompare);
    std::for_each(dumpVector.begin(), dumpVector.end(), DumpCostPrint);
    fprintf(fp, "total: %.2fmb", DumpPrint.m_totalKB / 1024.0f);

    fclose(fp);
  }
}

bool CResourceManager::IsFileExist(const char *c_szFileName) {
  return CPackManager::Instance().IsExist(c_szFileName);
}

void CResourceManager::Update() {
  DWORD CurrentTime = ELTimer_GetMSec();
  CResource *pResource;
  int Count = 0;

  TResourceDeletingMap::iterator itor = m_ResourceDeletingMap.begin();

  while (itor != m_ResourceDeletingMap.end()) {
    pResource = itor->first;

    if (CurrentTime >= itor->second) {
      if (pResource->canDestroy()) {
        // Tracef("Resource Clear %s\n", pResource->GetFileName());
        pResource->Clear();
      }

      itor = m_ResourceDeletingMap.erase(itor);

      if (++Count >= c_DeletingCountPerFrame)
        break;
    } else
      ++itor;
  }

  ProcessBackgroundLoading();
}

void CResourceManager::ReserveDeletingResource(CResource *pResource) {
  DWORD dwCurrentTime = ELTimer_GetMSec();
  m_ResourceDeletingMap.insert(TResourceDeletingMap::value_type(
      pResource, dwCurrentTime + c_Deleting_Wait_Time));
}

CResourceManager::CResourceManager() : m_pTextureCache(nullptr) {
  ms_loadingThread.Create(0);
  m_pTextureCache = new CTextureCache(512);
}

CResourceManager::~CResourceManager() {
  Destroy();
  ms_loadingThread.Shutdown();

  if (m_pTextureCache) {
    delete m_pTextureCache;
    m_pTextureCache = nullptr;
  }
}
