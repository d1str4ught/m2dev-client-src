//
// 캐릭터를 따라다니는 텍스트 관련 소스 (이름, 길드이름, 길드마크 등)
//
#include "stdafx.h"
#include "InstanceBase.h"
#include "resource.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "Locale.h"
#include "MarkManager.h"
#include "PackLib/PackManager.h"

#include <utf8.h>
// EPlaceDir and TextTailBiDi() template are defined in utf8.h

const D3DXCOLOR c_TextTail_Player_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Monster_Color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR c_TextTail_Item_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Chat_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Info_Color = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
const D3DXCOLOR c_TextTail_Guild_Name_Color = 0xFFEFD3FF;
const float c_TextTail_Name_Position = -10.0f;
const float c_fxMarkPosition = 1.5f;
const float c_fyGuildNamePosition = 15.0f;
const float c_fyMarkPosition = 15.0f + 11.0f;
BOOL bPKTitleEnable = TRUE;

// TEXTTAIL_LIVINGTIME_CONTROL
long gs_TextTail_LivingTime = 5000;

long TextTail_GetLivingTime() {
  assert(gs_TextTail_LivingTime > 1000);
  return gs_TextTail_LivingTime;
}

void TextTail_SetLivingTime(long livingTime) {
  gs_TextTail_LivingTime = livingTime;
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

CGraphicText *ms_pFont = NULL;

void CPythonTextTail::GetInfo(std::string *pstInfo) {
  char szInfo[256];
  sprintf(szInfo,
          "TextTail: ChatTail %zd, ChrTail (Map %zd, List %zd), ItemTail (Map "
          "%zd, List %zd), Pool %zd",
          m_ChatTailMap.size(), m_CharacterTextTailMap.size(),
          m_CharacterTextTailList.size(), m_ItemTextTailMap.size(),
          m_ItemTextTailList.size(), m_TextTailPool.GetCapacity());

  pstInfo->append(szInfo);
}

void CPythonTextTail::UpdateAllTextTail() {
  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetMainInstancePtr();
  if (pInstance) {
    TPixelPosition pixelPos;
    pInstance->NEW_GetPixelPosition(&pixelPos);

    TTextTailMap::iterator itorMap;

    for (itorMap = m_CharacterTextTailMap.begin();
         itorMap != m_CharacterTextTailMap.end(); ++itorMap) {
      UpdateDistance(pixelPos, itorMap->second);
    }

    for (itorMap = m_ItemTextTailMap.begin();
         itorMap != m_ItemTextTailMap.end(); ++itorMap) {
      UpdateDistance(pixelPos, itorMap->second);
    }

    for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin();
         itorChat != m_ChatTailMap.end(); ++itorChat) {
      UpdateDistance(pixelPos, itorChat->second);

      // NOTE : Chat TextTail이 있으면 캐릭터 이름도 출력한다.
      if (itorChat->second->bNameFlag) {
        DWORD dwVID = itorChat->first;
        ShowCharacterTextTail(dwVID);
      }
    }
  }
}

void CPythonTextTail::UpdateShowingTextTail() {
  TTextTailList::iterator itor;

  for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end();
       ++itor) {
    UpdateTextTail(*itor);
  }

  for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin();
       itorChat != m_ChatTailMap.end(); ++itorChat) {
    UpdateTextTail(itorChat->second);
  }

  for (itor = m_CharacterTextTailList.begin();
       itor != m_CharacterTextTailList.end(); ++itor) {
    TTextTail *pTextTail = *itor;
    UpdateTextTail(pTextTail);

    // NOTE : Chat TextTail이 있을 경우 위치를 바꾼다.
    TChatTailMap::iterator itor = m_ChatTailMap.find(pTextTail->dwVirtualID);
    if (m_ChatTailMap.end() != itor) {
      TTextTail *pChatTail = itor->second;
      if (pChatTail->bNameFlag) {
        pTextTail->y = pChatTail->y - 17.0f;
      }
    }
  }
}

void CPythonTextTail::UpdateTextTail(TTextTail *pTextTail) {
  if (!pTextTail->pOwner)
    return;

  /////

  CPythonGraphic &rpyGraphic = CPythonGraphic::Instance();
  rpyGraphic.Identity();

  const D3DXVECTOR3 &c_rv3Position = pTextTail->pOwner->GetPosition();
  rpyGraphic.ProjectPosition(c_rv3Position.x, c_rv3Position.y,
                             c_rv3Position.z + pTextTail->fHeight,
                             &pTextTail->x, &pTextTail->y, &pTextTail->z);

  pTextTail->x = floorf(pTextTail->x);
  pTextTail->y = floorf(pTextTail->y);

  // NOTE : 13m 밖에 있을때만 깊이를 넣습니다 - [levites]
  if (pTextTail->fDistanceFromPlayer < 1300.0f) {
    pTextTail->z = 0.0f;
  } else {
    pTextTail->z =
        pTextTail->z * CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
    pTextTail->z += 10.0f;
  }
}

void CPythonTextTail::ArrangeTextTail() {
  TTextTailList::iterator itor;
  TTextTailList::iterator itorCompare;

  DWORD dwTime = CTimer::Instance().GetCurrentMillisecond();

  for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end();
       ++itor) {
    TTextTail *pInsertTextTail = *itor;

    int yTemp = 5;
    int LimitCount = 0;

    for (itorCompare = m_ItemTextTailList.begin();
         itorCompare != m_ItemTextTailList.end();) {
      TTextTail *pCompareTextTail = *itorCompare;

      if (*itorCompare == *itor) {
        ++itorCompare;
        continue;
      }

      if (LimitCount >= 20)
        break;

      if (isIn(pInsertTextTail, pCompareTextTail)) {
        pInsertTextTail->y =
            (pCompareTextTail->y + pCompareTextTail->yEnd + yTemp);

        itorCompare = m_ItemTextTailList.begin();
        ++LimitCount;
        continue;
      }

      ++itorCompare;
    }

    if (pInsertTextTail->pOwnerTextInstance) {
      pInsertTextTail->pOwnerTextInstance->SetPosition(
          pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
      pInsertTextTail->pOwnerTextInstance->Update();

      pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r,
                                               pInsertTextTail->Color.g,
                                               pInsertTextTail->Color.b);
      pInsertTextTail->pTextInstance->SetPosition(
          pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
      pInsertTextTail->pTextInstance->Update();

    } else {
      pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r,
                                               pInsertTextTail->Color.g,
                                               pInsertTextTail->Color.b);
      pInsertTextTail->pTextInstance->SetPosition(
          pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
      pInsertTextTail->pTextInstance->Update();
    }
  }

  for (itor = m_CharacterTextTailList.begin();
       itor != m_CharacterTextTailList.end(); ++itor) {
    TTextTail *pTextTail = *itor;

    CGraphicMarkInstance *pMarkInstance = pTextTail->pMarkInstance;
    CGraphicTextInstance *pGuildNameInstance =
        pTextTail->pGuildNameTextInstance;
    if (pMarkInstance && pGuildNameInstance) {
      int iWidth, iHeight;
      int iImageHalfSize = pMarkInstance->GetWidth() / 2 + c_fxMarkPosition;
      pGuildNameInstance->GetTextSize(&iWidth, &iHeight);

      pMarkInstance->SetPosition(pTextTail->x - iWidth / 2 - iImageHalfSize,
                                 pTextTail->y - c_fyMarkPosition);
      pGuildNameInstance->SetPosition(pTextTail->x + iImageHalfSize,
                                      pTextTail->y - c_fyGuildNamePosition,
                                      pTextTail->z);
      pGuildNameInstance->Update();
    }

    // Title & Name refactor
    int iNameWidth = 0, iNameHeight = 0;
    pTextTail->pTextInstance->GetTextSize(&iNameWidth, &iNameHeight);

    const float fxAdd = 4.0f;

    float nameShift = 0.0f;
    if (pTextTail->pTitleTextInstance)
      nameShift = 1.0f;
    else if (pTextTail->pLevelTextInstance)
      nameShift = 5.0f;

    float nameX = floorf((pTextTail->x + nameShift) + 0.5f);
    float nameY = floorf(pTextTail->y + 0.5f);

    const float nameLeftEdge = pTextTail->x - (iNameWidth / 2.0f);
    float cursor = nameLeftEdge - fxAdd;

    // [Level] [Title] Name - LTR.
    // It can be set for RTL - Name [Title] [Level] by adding the required
    // check.
    cursor = TextTailBiDi(pTextTail->pTitleTextInstance, cursor, nameY,
                          pTextTail->z, fxAdd, EPlaceDir::Left);
    cursor = TextTailBiDi(pTextTail->pLevelTextInstance, cursor, nameY,
                          pTextTail->z, fxAdd, EPlaceDir::Left);

    pTextTail->pTextInstance->SetColor(pTextTail->Color.r, pTextTail->Color.g,
                                       pTextTail->Color.b);
    pTextTail->pTextInstance->SetPosition(nameX, nameY, pTextTail->z);
    pTextTail->pTextInstance->Update();
  }

  for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin();
       itorChat != m_ChatTailMap.end();) {
    TTextTail *pTextTail = itorChat->second;

    if (pTextTail->LivingTime < dwTime) {
      DeleteTextTail(pTextTail);
      itorChat = m_ChatTailMap.erase(itorChat);
      continue;
    } else
      ++itorChat;

    pTextTail->pTextInstance->SetColor(pTextTail->Color);
    pTextTail->pTextInstance->SetPosition(pTextTail->x, pTextTail->y,
                                          pTextTail->z);
    pTextTail->pTextInstance->Update();
  }
}

void CPythonTextTail::Render() {
  TTextTailList::iterator itor;

  for (itor = m_CharacterTextTailList.begin();
       itor != m_CharacterTextTailList.end(); ++itor) {
    TTextTail *pTextTail = *itor;
    pTextTail->pTextInstance->Render();
    if (pTextTail->pMarkInstance && pTextTail->pGuildNameTextInstance) {
      pTextTail->pMarkInstance->Render();
      pTextTail->pGuildNameTextInstance->Render();
    }
    if (pTextTail->pTitleTextInstance) {
      pTextTail->pTitleTextInstance->Render();
    }
    if (pTextTail->pLevelTextInstance) {
      pTextTail->pLevelTextInstance->Render();
    }
  }

  for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end();
       ++itor) {
    TTextTail *pTextTail = *itor;

    RenderTextTailBox(pTextTail);
    pTextTail->pTextInstance->Render();
    if (pTextTail->pOwnerTextInstance)
      pTextTail->pOwnerTextInstance->Render();
  }

  for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin();
       itorChat != m_ChatTailMap.end(); ++itorChat) {
    TTextTail *pTextTail = itorChat->second;
    if (pTextTail->pOwner->isShow())
      RenderTextTailName(pTextTail);
  }
}

void CPythonTextTail::RenderTextTailBox(TTextTail *pTextTail) {
  // 검은색 테두리
  CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
  CPythonGraphic::Instance().RenderBox2d(
      pTextTail->x + pTextTail->xStart, pTextTail->y + pTextTail->yStart,
      pTextTail->x + pTextTail->xEnd, pTextTail->y + pTextTail->yEnd,
      pTextTail->z);

  // 검은색 투명박스
  CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
  CPythonGraphic::Instance().RenderBar2d(
      pTextTail->x + pTextTail->xStart, pTextTail->y + pTextTail->yStart,
      pTextTail->x + pTextTail->xEnd, pTextTail->y + pTextTail->yEnd,
      pTextTail->z);
}

void CPythonTextTail::RenderTextTailName(TTextTail *pTextTail) {
  pTextTail->pTextInstance->Render();
}

void CPythonTextTail::HideAllTextTail() {
  // NOTE : Show All을 해준뒤 Hide All을 해주지 않으면 문제 발생 가능성 있음
  //        디자인 자체가 그렇게 깔끔하게 되지 않았음 - [levites]
  m_CharacterTextTailList.clear();
  m_ItemTextTailList.clear();
}

void CPythonTextTail::UpdateDistance(const TPixelPosition &c_rCenterPosition,
                                     TTextTail *pTextTail) {
  const D3DXVECTOR3 &c_rv3Position = pTextTail->pOwner->GetPosition();
  D3DXVECTOR2 v2Distance(c_rv3Position.x - c_rCenterPosition.x,
                         -c_rv3Position.y - c_rCenterPosition.y);
  pTextTail->fDistanceFromPlayer = D3DXVec2Length(&v2Distance);
}

void CPythonTextTail::ShowAllTextTail() {
  TTextTailMap::iterator itor;
  for (itor = m_CharacterTextTailMap.begin();
       itor != m_CharacterTextTailMap.end(); ++itor) {
    TTextTail *pTextTail = itor->second;
    if (pTextTail->fDistanceFromPlayer < 3500.0f)
      ShowCharacterTextTail(itor->first);
  }
  for (itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end();
       ++itor) {
    TTextTail *pTextTail = itor->second;
    if (pTextTail->fDistanceFromPlayer < 3500.0f)
      ShowItemTextTail(itor->first);
  }
}

void CPythonTextTail::ShowCharacterTextTail(DWORD VirtualID) {
  TTextTailMap::iterator itor = m_CharacterTextTailMap.find(VirtualID);

  if (m_CharacterTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  if (m_CharacterTextTailList.end() !=
      std::find(m_CharacterTextTailList.begin(), m_CharacterTextTailList.end(),
                pTextTail)) {
    // Tracef("이미 리스트에 있음 : %d\n", VirtualID);
    return;
  }

  // NOTE : ShowAll 시에는 모든 Instance 의 Pointer 를 찾아서 체크하므로 부하가
  // 걸릴 가능성도 있다.
  //        CInstanceBase 가 TextTail 을 직접 가지고 있는 것이 가장 좋은 형태일
  //        듯..
  if (!pTextTail->pOwner->isShow())
    return;

  CInstanceBase *pInstance = CPythonCharacterManager::Instance().GetInstancePtr(
      pTextTail->dwVirtualID);
  if (!pInstance)
    return;

  if (pInstance->IsGuildWall())
    return;

  if (pInstance->CanPickInstance())
    m_CharacterTextTailList.push_back(pTextTail);
}

void CPythonTextTail::ShowItemTextTail(DWORD VirtualID) {
  TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

  if (m_ItemTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  if (m_ItemTextTailList.end() != std::find(m_ItemTextTailList.begin(),
                                            m_ItemTextTailList.end(),
                                            pTextTail)) {
    // Tracef("이미 리스트에 있음 : %d\n", VirtualID);
    return;
  }

  m_ItemTextTailList.push_back(pTextTail);
}

bool CPythonTextTail::isIn(CPythonTextTail::TTextTail *pSource,
                           CPythonTextTail::TTextTail *pTarget) {
  float x1Source = pSource->x + pSource->xStart;
  float y1Source = pSource->y + pSource->yStart;
  float x2Source = pSource->x + pSource->xEnd;
  float y2Source = pSource->y + pSource->yEnd;
  float x1Target = pTarget->x + pTarget->xStart;
  float y1Target = pTarget->y + pTarget->yStart;
  float x2Target = pTarget->x + pTarget->xEnd;
  float y2Target = pTarget->y + pTarget->yEnd;

  if (x1Source <= x2Target && x2Source >= x1Target && y1Source <= y2Target &&
      y2Source >= y1Target) {
    return true;
  }

  return false;
}

void CPythonTextTail::RegisterCharacterTextTail(DWORD dwGuildID,
                                                DWORD dwVirtualID,
                                                const D3DXCOLOR &c_rColor,
                                                float fAddHeight) {
  CInstanceBase *pCharacterInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);

  if (!pCharacterInstance)
    return;

  TTextTail *pTextTail = RegisterTextTail(
      dwVirtualID, pCharacterInstance->GetNameString(),
      pCharacterInstance->GetGraphicThingInstancePtr(),
      pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + fAddHeight,
      c_rColor);

  CGraphicTextInstance *pTextInstance = pTextTail->pTextInstance;
  pTextInstance->SetOutline(true);
  pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);

  pTextTail->pMarkInstance = NULL;
  pTextTail->pGuildNameTextInstance = NULL;
  pTextTail->pTitleTextInstance = NULL;
  pTextTail->pLevelTextInstance = NULL;

  if (0 != dwGuildID) {
    pTextTail->pMarkInstance = CGraphicMarkInstance::New();

    DWORD dwMarkID = CGuildMarkManager::Instance().GetMarkID(dwGuildID);

    if (dwMarkID != CGuildMarkManager::INVALID_MARK_ID) {
      std::string markImagePath;

      if (CGuildMarkManager::Instance().GetMarkImageFilename(
              dwMarkID / CGuildMarkImage::MARK_TOTAL_COUNT, markImagePath)) {
        CPackManager::instance().SetFileLoadMode();
        pTextTail->pMarkInstance->SetImageFileName(markImagePath.c_str());
        pTextTail->pMarkInstance->Load();
        pTextTail->pMarkInstance->SetIndex(dwMarkID %
                                           CGuildMarkImage::MARK_TOTAL_COUNT);
        CPackManager::instance().SetPackLoadMode();
      }
    }

    std::string strGuildName;
    if (!CPythonGuild::Instance().GetGuildName(dwGuildID, &strGuildName))
      strGuildName = "Noname";

    CGraphicTextInstance *&prGuildNameInstance =
        pTextTail->pGuildNameTextInstance;
    prGuildNameInstance = CGraphicTextInstance::New();
    prGuildNameInstance->SetTextPointer(ms_pFont);
    prGuildNameInstance->SetOutline(true);
    prGuildNameInstance->SetHorizonalAlign(
        CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
    prGuildNameInstance->SetVerticalAlign(
        CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
    prGuildNameInstance->SetValue(strGuildName.c_str());
    prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color.r,
                                  c_TextTail_Guild_Name_Color.g,
                                  c_TextTail_Guild_Name_Color.b);
    prGuildNameInstance->Update();
  }

  m_CharacterTextTailMap.insert(
      TTextTailMap::value_type(dwVirtualID, pTextTail));
}

void CPythonTextTail::RegisterItemTextTail(DWORD VirtualID,
                                           const char *c_szText,
                                           CGraphicObjectInstance *pOwner) {
#ifdef __DEBUG
  char szName[256];
  spritnf(szName, "%s[%d]", c_szText, VirtualID);

  TTextTail *pTextTail =
      RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position,
                       c_TextTail_Item_Color);
  m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
#else
  TTextTail *pTextTail =
      RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position,
                       c_TextTail_Item_Color);
  m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
#endif
}

void CPythonTextTail::RegisterChatTail(DWORD VirtualID, const char *c_szChat) {
  CInstanceBase *pCharacterInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

  if (!pCharacterInstance)
    return;

  TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

  if (m_ChatTailMap.end() != itor) {
    TTextTail *pTextTail = itor->second;

    pTextTail->pTextInstance->SetValue(c_szChat);
    pTextTail->pTextInstance->Update();
    pTextTail->Color = c_TextTail_Chat_Color;
    pTextTail->pTextInstance->SetColor(c_TextTail_Chat_Color);

    // TEXTTAIL_LIVINGTIME_CONTROL
    pTextTail->LivingTime =
        CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
    // END_OF_TEXTTAIL_LIVINGTIME_CONTROL

    pTextTail->bNameFlag = TRUE;

    return;
  }

  TTextTail *pTextTail = RegisterTextTail(
      VirtualID, c_szChat, pCharacterInstance->GetGraphicThingInstancePtr(),
      pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f,
      c_TextTail_Chat_Color);

  // TEXTTAIL_LIVINGTIME_CONTROL
  pTextTail->LivingTime =
      CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
  // END_OF_TEXTTAIL_LIVINGTIME_CONTROL

  pTextTail->bNameFlag = TRUE;
  pTextTail->pTextInstance->SetOutline(true);
  pTextTail->pTextInstance->SetVerticalAlign(
      CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
  m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

void CPythonTextTail::RegisterInfoTail(DWORD VirtualID, const char *c_szChat) {
  CInstanceBase *pCharacterInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

  if (!pCharacterInstance)
    return;

  TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

  if (m_ChatTailMap.end() != itor) {
    TTextTail *pTextTail = itor->second;

    pTextTail->pTextInstance->SetValue(c_szChat);
    pTextTail->pTextInstance->Update();
    pTextTail->Color = c_TextTail_Info_Color;
    pTextTail->pTextInstance->SetColor(c_TextTail_Info_Color);

    // TEXTTAIL_LIVINGTIME_CONTROL
    pTextTail->LivingTime =
        CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
    // END_OF_TEXTTAIL_LIVINGTIME_CONTROL

    pTextTail->bNameFlag = FALSE;

    return;
  }

  TTextTail *pTextTail = RegisterTextTail(
      VirtualID, c_szChat, pCharacterInstance->GetGraphicThingInstancePtr(),
      pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f,
      c_TextTail_Info_Color);

  // TEXTTAIL_LIVINGTIME_CONTROL
  pTextTail->LivingTime =
      CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
  // END_OF_TEXTTAIL_LIVINGTIME_CONTROL

  pTextTail->bNameFlag = FALSE;
  pTextTail->pTextInstance->SetOutline(true);
  pTextTail->pTextInstance->SetVerticalAlign(
      CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
  m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

bool CPythonTextTail::GetTextTailPosition(DWORD dwVID, float *px, float *py,
                                          float *pz) {
  TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(dwVID);

  if (m_CharacterTextTailMap.end() == itorCharacter) {
    return false;
  }

  TTextTail *pTextTail = itorCharacter->second;
  *px = pTextTail->x;
  *py = pTextTail->y;
  *pz = pTextTail->z;

  return true;
}

bool CPythonTextTail::IsChatTextTail(DWORD dwVID) {
  TChatTailMap::iterator itorChat = m_ChatTailMap.find(dwVID);

  if (m_ChatTailMap.end() == itorChat)
    return false;

  return true;
}

void CPythonTextTail::SetCharacterTextTailColor(DWORD VirtualID,
                                                const D3DXCOLOR &c_rColor) {
  TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);

  if (m_CharacterTextTailMap.end() == itorCharacter)
    return;

  TTextTail *pTextTail = itorCharacter->second;
  pTextTail->pTextInstance->SetColor(c_rColor);
  pTextTail->Color = c_rColor;
}

void CPythonTextTail::SetItemTextTailOwner(DWORD dwVID, const char *c_szName) {
  TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVID);
  if (m_ItemTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  if (strlen(c_szName) > 0) {
    if (!pTextTail->pOwnerTextInstance) {
      pTextTail->pOwnerTextInstance = CGraphicTextInstance::New();
    }

    std::string strName = c_szName;
    pTextTail->pOwnerTextInstance->SetTextPointer(ms_pFont);
    pTextTail->pOwnerTextInstance->SetHorizonalAlign(
        CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
    pTextTail->pOwnerTextInstance->SetValue(strName.c_str());
    pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
    pTextTail->pOwnerTextInstance->Update();

    int xOwnerSize, yOwnerSize;
    pTextTail->pOwnerTextInstance->GetTextSize(&xOwnerSize, &yOwnerSize);
    pTextTail->yStart = -2.0f;
    pTextTail->yEnd += float(yOwnerSize + 4);
    pTextTail->xStart = fMIN(pTextTail->xStart, float(-xOwnerSize / 2 - 1));
    pTextTail->xEnd = fMAX(pTextTail->xEnd, float(xOwnerSize / 2 + 1));
  } else {
    if (pTextTail->pOwnerTextInstance) {
      CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
      pTextTail->pOwnerTextInstance = NULL;
    }

    int xSize, ySize;
    pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
    pTextTail->xStart = (float)(-xSize / 2 - 2);
    pTextTail->yStart = -2.0f;
    pTextTail->xEnd = (float)(xSize / 2 + 2);
    pTextTail->yEnd = (float)ySize;
  }
}

void CPythonTextTail::DeleteCharacterTextTail(DWORD VirtualID) {
  TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);
  TTextTailMap::iterator itorChat = m_ChatTailMap.find(VirtualID);

  if (m_CharacterTextTailMap.end() != itorCharacter) {
    DeleteTextTail(itorCharacter->second);
    m_CharacterTextTailMap.erase(itorCharacter);
  } else {
    Tracenf("CPythonTextTail::DeleteCharacterTextTail - Find VID[%d] Error",
            VirtualID);
  }

  if (m_ChatTailMap.end() != itorChat) {
    DeleteTextTail(itorChat->second);
    m_ChatTailMap.erase(itorChat);
  }
}

void CPythonTextTail::DeleteItemTextTail(DWORD VirtualID) {
  TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

  if (m_ItemTextTailMap.end() == itor) {
    Tracef(" CPythonTextTail::DeleteItemTextTail - None Item Text Tail\n");
    return;
  }

  DeleteTextTail(itor->second);
  m_ItemTextTailMap.erase(itor);
}

CPythonTextTail::TTextTail *
CPythonTextTail::RegisterTextTail(DWORD dwVirtualID, const char *c_szText,
                                  CGraphicObjectInstance *pOwner, float fHeight,
                                  const D3DXCOLOR &c_rColor) {
  TTextTail *pTextTail = m_TextTailPool.Alloc();

  pTextTail->dwVirtualID = dwVirtualID;
  pTextTail->pOwner = pOwner;
  pTextTail->pTextInstance = CGraphicTextInstance::New();
  pTextTail->pOwnerTextInstance = NULL;
  pTextTail->fHeight = fHeight;

  pTextTail->pTextInstance->SetTextPointer(ms_pFont);
  pTextTail->pTextInstance->SetHorizonalAlign(
      CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
  pTextTail->pTextInstance->SetValue(c_szText);
  pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
  pTextTail->pTextInstance->Update();

  int xSize, ySize;
  pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
  pTextTail->xStart = (float)(-xSize / 2 - 2);
  pTextTail->yStart = -2.0f;
  pTextTail->xEnd = (float)(xSize / 2 + 2);
  pTextTail->yEnd = (float)ySize;
  pTextTail->Color = c_rColor;
  pTextTail->fDistanceFromPlayer = 0.0f;
  pTextTail->x = -100.0f;
  pTextTail->y = -100.0f;
  pTextTail->z = 0.0f;
  pTextTail->pMarkInstance = NULL;
  pTextTail->pGuildNameTextInstance = NULL;
  pTextTail->pTitleTextInstance = NULL;
  pTextTail->pLevelTextInstance = NULL;
  return pTextTail;
}

void CPythonTextTail::DeleteTextTail(TTextTail *pTextTail) {
  if (pTextTail->pTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pTextInstance);
    pTextTail->pTextInstance = NULL;
  }
  if (pTextTail->pOwnerTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
    pTextTail->pOwnerTextInstance = NULL;
  }
  if (pTextTail->pMarkInstance) {
    CGraphicMarkInstance::Delete(pTextTail->pMarkInstance);
    pTextTail->pMarkInstance = NULL;
  }
  if (pTextTail->pGuildNameTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pGuildNameTextInstance);
    pTextTail->pGuildNameTextInstance = NULL;
  }
  if (pTextTail->pTitleTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
    pTextTail->pTitleTextInstance = NULL;
  }
  if (pTextTail->pLevelTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
    pTextTail->pLevelTextInstance = NULL;
  }

  m_TextTailPool.Free(pTextTail);
}

int CPythonTextTail::Pick(int ixMouse, int iyMouse) {
  for (TTextTailMap::iterator itor = m_ItemTextTailMap.begin();
       itor != m_ItemTextTailMap.end(); ++itor) {
    TTextTail *pTextTail = itor->second;

    if (ixMouse >= pTextTail->x + pTextTail->xStart &&
        ixMouse <= pTextTail->x + pTextTail->xEnd &&
        iyMouse >= pTextTail->y + pTextTail->yStart &&
        iyMouse <= pTextTail->y + pTextTail->yEnd) {
      SelectItemName(itor->first);
      return (itor->first);
    }
  }

  return -1;
}

void CPythonTextTail::SelectItemName(DWORD dwVirtualID) {
  TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVirtualID);

  if (m_ItemTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;
  pTextTail->pTextInstance->SetColor(0.1f, 0.9f, 0.1f);
}

void CPythonTextTail::AttachTitle(DWORD dwVID, const char *c_szName,
                                  const D3DXCOLOR &c_rColor) {
  if (!bPKTitleEnable)
    return;

  TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
  if (m_CharacterTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  CGraphicTextInstance *&prTitle = pTextTail->pTitleTextInstance;
  if (!prTitle) {
    prTitle = CGraphicTextInstance::New();
    prTitle->SetTextPointer(ms_pFont);
    prTitle->SetOutline(true);
    prTitle->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
    prTitle->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
  }

  prTitle->SetValue(c_szName);
  prTitle->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
  prTitle->Update();
}

void CPythonTextTail::DetachTitle(DWORD dwVID) {
  if (!bPKTitleEnable)
    return;

  TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
  if (m_CharacterTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  if (pTextTail->pTitleTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
    pTextTail->pTitleTextInstance = NULL;
  }
}

void CPythonTextTail::EnablePKTitle(BOOL bFlag) { bPKTitleEnable = bFlag; }

void CPythonTextTail::AttachLevel(DWORD dwVID, const char *c_szText,
                                  const D3DXCOLOR &c_rColor) {
  if (!bPKTitleEnable)
    return;

  TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
  if (m_CharacterTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  CGraphicTextInstance *&prLevel = pTextTail->pLevelTextInstance;
  if (!prLevel) {
    prLevel = CGraphicTextInstance::New();
    prLevel->SetTextPointer(ms_pFont);
    prLevel->SetOutline(true);

    prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
    prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
  }

  prLevel->SetValue(c_szText);
  prLevel->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
  prLevel->Update();
}

void CPythonTextTail::DetachLevel(DWORD dwVID) {
  if (!bPKTitleEnable)
    return;

  TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
  if (m_CharacterTextTailMap.end() == itor)
    return;

  TTextTail *pTextTail = itor->second;

  if (pTextTail->pLevelTextInstance) {
    CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
    pTextTail->pLevelTextInstance = NULL;
  }
}

void CPythonTextTail::RefreshAllGuildMark() {
  CPackManager::instance().SetFileLoadMode();

  for (TTextTailMap::iterator itor = m_CharacterTextTailMap.begin();
       itor != m_CharacterTextTailMap.end(); ++itor) {
    TTextTail *pTextTail = itor->second;

    if (!pTextTail->pMarkInstance)
      continue;

    // Re-load the mark instance to get the updated texture
    pTextTail->pMarkInstance->Load();
  }

  CPackManager::instance().SetPackLoadMode();
}

void CPythonTextTail::Initialize() {
  // DEFAULT_FONT
  // ms_pFont = (CGraphicText
  // *)CResourceManager::Instance().GetTypeResourcePointer(g_strDefaultFontName.c_str());

  CGraphicText *pkDefaultFont =
      static_cast<CGraphicText *>(DefaultFont_GetResource());
  if (!pkDefaultFont) {
    TraceError("CPythonTextTail::Initialize - CANNOT_FIND_DEFAULT_FONT");
    return;
  }

  ms_pFont = pkDefaultFont;
  // END_OF_DEFAULT_FONT
}

void CPythonTextTail::Destroy() { m_TextTailPool.Clear(); }

void CPythonTextTail::Clear() {
  m_CharacterTextTailMap.clear();
  m_CharacterTextTailList.clear();
  m_ItemTextTailMap.clear();
  m_ItemTextTailList.clear();
  m_ChatTailMap.clear();

  m_TextTailPool.Clear();
}

CPythonTextTail::CPythonTextTail() { Clear(); }

CPythonTextTail::~CPythonTextTail() { Destroy(); }
