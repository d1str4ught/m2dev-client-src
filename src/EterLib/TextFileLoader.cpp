#include "StdAfx.h"
#include "EterBase/CRC32.h"
#include <string>
#include "PackLib/PackManager.h"

#include "Pool.h"
#include "TextFileLoader.h"

std::map<DWORD, CTextFileLoader *>
    CTextFileLoader::ms_kMap_dwNameKey_pkTextFileLoader;
bool CTextFileLoader::ms_isCacheMode = false;

CDynamicPool<CTextFileLoader::SGroupNode> CTextFileLoader::SGroupNode::ms_kPool;

CTokenVector *
CTextFileLoader::SGroupNode::GetTokenVector(const std::string &c_rstGroupName) {
  DWORD dwGroupNameKey =
      GenNameKey(c_rstGroupName.c_str(), c_rstGroupName.length());

  std::map<DWORD, CTokenVector>::iterator f =
      m_kMap_dwKey_kVct_stToken.find(dwGroupNameKey);
  if (m_kMap_dwKey_kVct_stToken.end() == f)
    return NULL;

  return &f->second;
}

bool CTextFileLoader::SGroupNode::IsExistTokenVector(
    const std::string &c_rstGroupName) {
  DWORD dwGroupNameKey =
      GenNameKey(c_rstGroupName.c_str(), c_rstGroupName.length());

  if (m_kMap_dwKey_kVct_stToken.end() ==
      m_kMap_dwKey_kVct_stToken.find(dwGroupNameKey))
    return false;

  return true;
}

void CTextFileLoader::SGroupNode::InsertTokenVector(
    const std::string &c_rstGroupName, const CTokenVector &c_rkVct_stToken) {
  DWORD dwGroupNameKey =
      GenNameKey(c_rstGroupName.c_str(), c_rstGroupName.length());

  m_kMap_dwKey_kVct_stToken.insert(std::map<DWORD, CTokenVector>::value_type(
      dwGroupNameKey, c_rkVct_stToken));
}

DWORD CTextFileLoader::SGroupNode::GenNameKey(const char *c_szGroupName,
                                              UINT uGroupNameLen) {
  return GetCRC32(c_szGroupName, uGroupNameLen);
}

const std::string &CTextFileLoader::SGroupNode::GetGroupName() {
  return m_strGroupName;
}

bool CTextFileLoader::SGroupNode::IsGroupNameKey(DWORD dwGroupNameKey) {
  if (dwGroupNameKey == m_dwGroupNameKey)
    return true;

  return false;
}

void CTextFileLoader::SGroupNode::SetGroupName(
    const std::string &c_rstGroupName) {
  m_strGroupName = c_rstGroupName;
  stl_lowers(m_strGroupName);

  m_dwGroupNameKey =
      GenNameKey(m_strGroupName.c_str(), m_strGroupName.length());
}

CTextFileLoader::SGroupNode *CTextFileLoader::SGroupNode::New() {
  return ms_kPool.Alloc();
}

void CTextFileLoader::SGroupNode::Delete(SGroupNode *pkNode) {
  pkNode->m_kMap_dwKey_kVct_stToken.clear();
  pkNode->ChildNodeVector.clear();
  pkNode->m_strGroupName = "";
  pkNode->m_dwGroupNameKey = 0;
  ms_kPool.Free(pkNode);
}

void CTextFileLoader::SGroupNode::DestroySystem() { ms_kPool.Destroy(); }

CTextFileLoader *CTextFileLoader::Cache(const char *c_szFileName) {
  DWORD dwNameKey = GetCRC32(c_szFileName, strlen(c_szFileName));
  std::map<DWORD, CTextFileLoader *>::iterator f =
      ms_kMap_dwNameKey_pkTextFileLoader.find(dwNameKey);
  if (ms_kMap_dwNameKey_pkTextFileLoader.end() != f) {
    if (!ms_isCacheMode) {
      delete f->second;

      CTextFileLoader *pkNewTextFileLoader = new CTextFileLoader;
      pkNewTextFileLoader->Load(c_szFileName);
      f->second = pkNewTextFileLoader;
    }
    f->second->SetTop();
    return f->second;
  }

  CTextFileLoader *pkNewTextFileLoader = new CTextFileLoader;
  pkNewTextFileLoader->Load(c_szFileName);

  ms_kMap_dwNameKey_pkTextFileLoader.insert(
      std::map<DWORD, CTextFileLoader *>::value_type(dwNameKey,
                                                     pkNewTextFileLoader));
  return pkNewTextFileLoader;
}

void CTextFileLoader::SetCacheMode() { ms_isCacheMode = true; }

void CTextFileLoader::DestroySystem() {
  {
    std::map<DWORD, CTextFileLoader *>::iterator i;
    for (i = ms_kMap_dwNameKey_pkTextFileLoader.begin();
         i != ms_kMap_dwNameKey_pkTextFileLoader.end(); ++i)
      delete i->second;
    ms_kMap_dwNameKey_pkTextFileLoader.clear();
  }

  SGroupNode::DestroySystem();
}

void CTextFileLoader::Destroy() { __DestroyGroupNodeVector(); }

CTextFileLoader::CTextFileLoader() {
  SetTop();

  m_acBufData = NULL;
  m_dwBufSize = 0;
  m_dwBufCapacity = 0;

  m_GlobalNode.m_strGroupName = "global";
  m_GlobalNode.pParentNode = NULL;

  m_kVct_pkNode.reserve(128);
}

CTextFileLoader::~CTextFileLoader() {
  Destroy();

  if (m_acBufData)
    delete[] m_acBufData;
}

void CTextFileLoader::__DestroyGroupNodeVector() {
  std::vector<SGroupNode *>::iterator i;
  for (i = m_kVct_pkNode.begin(); i != m_kVct_pkNode.end(); ++i)
    SGroupNode::Delete(*i);
  m_kVct_pkNode.clear();
}

const char *CTextFileLoader::GetFileName() { return m_strFileName.c_str(); }

bool CTextFileLoader::IsEmpty() { return m_strFileName.empty(); }

bool CTextFileLoader::Load(const char *c_szFileName) {
  m_strFileName = "";

  TPackFile kFile;
  if (!CPackManager::Instance().GetFile(c_szFileName, kFile))
    return false;

  if (m_dwBufCapacity < kFile.size()) {
    m_dwBufCapacity = kFile.size();

    if (m_acBufData)
      delete[] m_acBufData;

    m_acBufData = new char[m_dwBufCapacity];
  }

  m_dwBufSize = kFile.size();
  memcpy(m_acBufData, kFile.data(), m_dwBufSize);

  m_strFileName = c_szFileName;
  m_dwcurLineIndex = 0;

  m_textFileLoader.Bind(m_dwBufSize, m_acBufData);
  return LoadGroup(&m_GlobalNode);
}

bool CTextFileLoader::LoadGroup(TGroupNode *pGroupNode) {
  CTokenVector stTokenVector;
  int nLocalGroupDepth = 0;

  for (; m_dwcurLineIndex < m_textFileLoader.GetLineCount();
       ++m_dwcurLineIndex) {
    int iRet;

    if ((iRet = m_textFileLoader.SplitLine2(m_dwcurLineIndex,
                                            &stTokenVector)) != 0) {
      if (iRet == -2)
        TraceError("cannot find \" in %s:%lu", m_strFileName.c_str(),
                   m_dwcurLineIndex);
      continue;
    }

    stl_lowers(stTokenVector[0]);

    if ('{' == stTokenVector[0][0]) {
      nLocalGroupDepth++;
      continue;
    }

    if ('}' == stTokenVector[0][0]) {
      nLocalGroupDepth--;
      break;
    }

    // Group
    if (0 == stTokenVector[0].compare("group")) {
      if (2 != stTokenVector.size()) {
        assert(!"There is no group name!");
        continue;
      }

      TGroupNode *pNewNode = TGroupNode::New();
      m_kVct_pkNode.push_back(pNewNode);

      pNewNode->pParentNode = pGroupNode;
      pNewNode->SetGroupName(stTokenVector[1]);
      pGroupNode->ChildNodeVector.push_back(pNewNode);

      ++m_dwcurLineIndex;

      if (false == LoadGroup(pNewNode))
        return false;
    }
    // List
    else if (0 == stTokenVector[0].compare("list")) {
      if (2 != stTokenVector.size()) {
        assert(!"There is no list name!");
        continue;
      }

      CTokenVector stSubTokenVector;

      stl_lowers(stTokenVector[1]);
      std::string key = stTokenVector[1];

      stTokenVector.clear();

      ++m_dwcurLineIndex;
      for (; m_dwcurLineIndex < m_textFileLoader.GetLineCount();
           ++m_dwcurLineIndex) {
        if (!m_textFileLoader.SplitLine(m_dwcurLineIndex, &stSubTokenVector))
          continue;

        if ('{' == stSubTokenVector[0][0])
          continue;

        if ('}' == stSubTokenVector[0][0])
          break;

        for (DWORD j = 0; j < stSubTokenVector.size(); ++j) {
          stTokenVector.push_back(stSubTokenVector[j]);
        }
      }

      pGroupNode->InsertTokenVector(key, stTokenVector);
      // pGroupNode->LocalTokenVectorMap.insert(std::make_pair(key,
      // stTokenVector));
    } else {
      std::string key = stTokenVector[0];

      if (1 == stTokenVector.size()) {
        TraceError("CTextFileLoader::LoadGroup : must have a value (filename: "
                   "%s line: %d key: %s)",
                   m_strFileName.c_str(), m_dwcurLineIndex, key.c_str());
        break;
      }

      stTokenVector.erase(stTokenVector.begin());
      pGroupNode->InsertTokenVector(key, stTokenVector);
      // pGroupNode->LocalTokenVectorMap.insert(std::make_pair(key,
      // stTokenVector));
    }
  }

  return (nLocalGroupDepth == 0);
}

void CTextFileLoader::SetTop() { m_pcurNode = &m_GlobalNode; }

DWORD CTextFileLoader::GetChildNodeCount() {
  if (!m_pcurNode) {
    assert(!"Node to access has not set!");
    return 0;
  }

  return m_pcurNode->ChildNodeVector.size();
}

BOOL CTextFileLoader::SetChildNode(const char *c_szKey) {
  if (!m_pcurNode) {
    assert(!"Node to access has not set!");
    return FALSE;
  }

  DWORD dwKey = SGroupNode::GenNameKey(c_szKey, strlen(c_szKey));

  for (DWORD i = 0; i < m_pcurNode->ChildNodeVector.size(); ++i) {
    TGroupNode *pGroupNode = m_pcurNode->ChildNodeVector[i];
    if (pGroupNode->IsGroupNameKey(dwKey)) {
      m_pcurNode = pGroupNode;
      return TRUE;
    }
  }

  return FALSE;
}
BOOL CTextFileLoader::SetChildNode(const std::string &c_rstrKeyHead,
                                   DWORD dwIndex) {
  char szKey[32 + 1];
  _snprintf(szKey, sizeof(szKey), "%s%02u", c_rstrKeyHead.c_str(), dwIndex);

  return SetChildNode(szKey);
}
BOOL CTextFileLoader::SetChildNode(DWORD dwIndex) {
  if (!m_pcurNode) {
    assert(!"Node to access has not set!");
    return FALSE;
  }

  if (dwIndex >= m_pcurNode->ChildNodeVector.size()) {
    assert(!"Node index to set is too large to access!");
    return FALSE;
  }

  m_pcurNode = m_pcurNode->ChildNodeVector[dwIndex];

  return TRUE;
}

BOOL CTextFileLoader::SetParentNode() {
  if (!m_pcurNode) {
    assert(!"Node to access has not set!");
    return FALSE;
  }

  if (NULL == m_pcurNode->pParentNode) {
    assert(!"Current group node is already top!");
    return FALSE;
  }

  m_pcurNode = m_pcurNode->pParentNode;

  return TRUE;
}

BOOL CTextFileLoader::GetCurrentNodeName(std::string *pstrName) {
  if (!m_pcurNode)
    return FALSE;
  if (NULL == m_pcurNode->pParentNode)
    return FALSE;

  *pstrName = m_pcurNode->GetGroupName();

  return TRUE;
}

BOOL CTextFileLoader::IsToken(const std::string &c_rstrKey) {
  if (!m_pcurNode) {
    assert(!"Node to access has not set!");
    return FALSE;
  }

  return m_pcurNode->IsExistTokenVector(c_rstrKey);
  // return m_pcurNode->LocalTokenVectorMap.end() !=
  // m_pcurNode->LocalTokenVectorMap.find(c_rstrKey);
}

BOOL CTextFileLoader::GetTokenVector(const std::string &c_rstrKey,
                                     CTokenVector **ppTokenVector) {
  if (!m_pcurNode) {
    assert(!"Node to access has not set!");
    return FALSE;
  }

  CTokenVector *pkRetTokenVector = m_pcurNode->GetTokenVector(c_rstrKey);
  if (!pkRetTokenVector)
    return FALSE;

  *ppTokenVector = pkRetTokenVector;

  // CTokenVectorMap::iterator itor =
  // m_pcurNode->LocalTokenVectorMap.find(c_rstrKey); if
  // (m_pcurNode->LocalTokenVectorMap.end() == itor)
  //{
  // Tracef(" CTextFileLoader::GetTokenVector - Failed to find the key %s [%s ::
  // %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
  // c_rstrKey.c_str());
  //	return FALSE;
  //}

  //*ppTokenVector = &itor->second;

  return TRUE;
}

BOOL CTextFileLoader::GetTokenBoolean(const std::string &c_rstrKey,
                                      BOOL *pData) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->empty()) {
    // Tracef(" CTextFileLoader::GetTokenBoolean - Failed to find the value %s
    // [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  *pData = BOOL(atoi(pTokenVector->at(0).c_str()));

  return TRUE;
}

BOOL CTextFileLoader::GetTokenByte(const std::string &c_rstrKey, BYTE *pData) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->empty()) {
    // Tracef(" CTextFileLoader::GetTokenByte - Failed to find the value %s [%s
    // : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  *pData = BYTE(atoi(pTokenVector->at(0).c_str()));

  return TRUE;
}

BOOL CTextFileLoader::GetTokenWord(const std::string &c_rstrKey, WORD *pData) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->empty()) {
    // Tracef(" CTextFileLoader::GetTokenWord - Failed to find the value %s [%s
    // : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  *pData = WORD(atoi(pTokenVector->at(0).c_str()));

  return TRUE;
}

BOOL CTextFileLoader::GetTokenInteger(const std::string &c_rstrKey,
                                      int *pData) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->empty()) {
    // Tracef(" CTextFileLoader::GetTokenInteger - Failed to find the value %s
    // [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  *pData = atoi(pTokenVector->at(0).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenDoubleWord(const std::string &c_rstrKey,
                                         DWORD *pData) {
  return GetTokenInteger(c_rstrKey, *(int **)(&pData));
}

BOOL CTextFileLoader::GetTokenFloat(const std::string &c_rstrKey,
                                    float *pData) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->empty()) {
    // Tracef(" CTextFileLoader::GetTokenFloat - Failed to find the value %s [%s
    // : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  *pData = atof(pTokenVector->at(0).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenVector2(const std::string &c_rstrKey,
                                      D3DXVECTOR2 *pVector2) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 2) {
    // Tracef(" CTextFileLoader::GetTokenVector2 - This key should have 2 values
    // %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  pVector2->x = atof(pTokenVector->at(0).c_str());
  pVector2->y = atof(pTokenVector->at(1).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenVector3(const std::string &c_rstrKey,
                                      D3DXVECTOR3 *pVector3) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 3) {
    // Tracef(" CTextFileLoader::GetTokenVector3 - This key should have 3 values
    // %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  pVector3->x = atof(pTokenVector->at(0).c_str());
  pVector3->y = atof(pTokenVector->at(1).c_str());
  pVector3->z = atof(pTokenVector->at(2).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenVector4(const std::string &c_rstrKey,
                                      D3DXVECTOR4 *pVector4) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 4) {
    // Tracef(" CTextFileLoader::GetTokenVector3 - This key should have 3 values
    // %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  pVector4->x = atof(pTokenVector->at(0).c_str());
  pVector4->y = atof(pTokenVector->at(1).c_str());
  pVector4->z = atof(pTokenVector->at(2).c_str());
  pVector4->w = atof(pTokenVector->at(3).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenPosition(const std::string &c_rstrKey,
                                       D3DXVECTOR3 *pVector) {
  return GetTokenVector3(c_rstrKey, pVector);
}

BOOL CTextFileLoader::GetTokenQuaternion(const std::string &c_rstrKey,
                                         D3DXQUATERNION *pQ) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 4) {
    // Tracef(" CTextFileLoader::GetTokenVector3 - This key should have 3 values
    // %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  pQ->x = atof(pTokenVector->at(0).c_str());
  pQ->y = atof(pTokenVector->at(1).c_str());
  pQ->z = atof(pTokenVector->at(2).c_str());
  pQ->w = atof(pTokenVector->at(3).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenDirection(const std::string &c_rstrKey,
                                        D3DVECTOR *pVector) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 3) {
    // Tracef(" CTextFileLoader::GetTokenDirection - This key should have 3
    // values %s [%s : %s]\n", m_File.GetFileName(),
    // m_pcurNode->strGroupName.c_str(), c_rstrKey.c_str());
    return FALSE;
  }

  pVector->x = atof(pTokenVector->at(0).c_str());
  pVector->y = atof(pTokenVector->at(1).c_str());
  pVector->z = atof(pTokenVector->at(2).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenColor(const std::string &c_rstrKey,
                                    D3DXCOLOR *pColor) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 4) {
    // Tracef(" CTextFileLoader::GetTokenColor - This key should have 4 values
    // %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  pColor->r = atof(pTokenVector->at(0).c_str());
  pColor->g = atof(pTokenVector->at(1).c_str());
  pColor->b = atof(pTokenVector->at(2).c_str());
  pColor->a = atof(pTokenVector->at(3).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenColor(const std::string &c_rstrKey,
                                    D3DCOLORVALUE *pColor) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->size() != 4) {
    // Tracef(" CTextFileLoader::GetTokenColor - This key should have 4 values
    // %s [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  pColor->r = atof(pTokenVector->at(0).c_str());
  pColor->g = atof(pTokenVector->at(1).c_str());
  pColor->b = atof(pTokenVector->at(2).c_str());
  pColor->a = atof(pTokenVector->at(3).c_str());

  return TRUE;
}

BOOL CTextFileLoader::GetTokenString(const std::string &c_rstrKey,
                                     std::string *pString) {
  CTokenVector *pTokenVector;
  if (!GetTokenVector(c_rstrKey, &pTokenVector))
    return FALSE;

  if (pTokenVector->empty()) {
    // Tracef(" CTextFileLoader::GetTokenString - Failed to find the value %s
    // [%s : %s]\n", m_File.GetFileName(), m_pcurNode->strGroupName.c_str(),
    // c_rstrKey.c_str());
    return FALSE;
  }

  *pString = pTokenVector->at(0);

  return TRUE;
}
