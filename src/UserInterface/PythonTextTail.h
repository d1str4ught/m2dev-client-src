#pragma once

#include "EterBase/Singleton.h"

/*
 *	따라다니는 텍스트 처리
 */
class CPythonTextTail : public CSingleton<CPythonTextTail> {
public:
  typedef struct STextTail {
    CGraphicTextInstance *pTextInstance;
    CGraphicTextInstance *pOwnerTextInstance;

    CGraphicMarkInstance *pMarkInstance;
    CGraphicTextInstance *pGuildNameTextInstance;

    CGraphicTextInstance *pTitleTextInstance;
    CGraphicTextInstance *pLevelTextInstance;

    // Todo : 이건 전부 VID로 바꾸도록 한다.
    //        도중 캐릭터가 없어질 경우 튕길 가능성이 있음
    CGraphicObjectInstance *pOwner;

    DWORD dwVirtualID;

    float x, y, z;
    float fDistanceFromPlayer;
    D3DXCOLOR Color;
    BOOL bNameFlag; // 이름도 함께 켤것인지의 플래그

    float xStart, yStart;
    float xEnd, yEnd;

    DWORD LivingTime;

    float fHeight;

    STextTail() {}
    virtual ~STextTail() {}
  } TTextTail;

  typedef std::map<DWORD, TTextTail *> TTextTailMap;
  typedef std::list<TTextTail *> TTextTailList;
  typedef TTextTailMap TChatTailMap;

public:
  CPythonTextTail(void);
  virtual ~CPythonTextTail(void);

  void GetInfo(std::string *pstInfo);

  void Initialize();
  void Destroy();
  void Clear();

  void UpdateAllTextTail();
  void UpdateShowingTextTail();
  void Render();

  void ArrangeTextTail();
  void HideAllTextTail();
  void ShowAllTextTail();
  void ShowCharacterTextTail(DWORD VirtualID);
  void ShowItemTextTail(DWORD VirtualID);

  void RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID,
                                 const D3DXCOLOR &c_rColor,
                                 float fAddHeight = 10.0f);
  void RegisterItemTextTail(DWORD VirtualID, const char *c_szText,
                            CGraphicObjectInstance *pOwner);
  void RegisterChatTail(DWORD VirtualID, const char *c_szChat);
  void RegisterInfoTail(DWORD VirtualID, const char *c_szChat);
  void SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR &c_rColor);
  void SetItemTextTailOwner(DWORD dwVID, const char *c_szName);
  void DeleteCharacterTextTail(DWORD VirtualID);
  void DeleteItemTextTail(DWORD VirtualID);
  void RefreshAllGuildMark();

  int Pick(int ixMouse, int iyMouse);
  void SelectItemName(DWORD dwVirtualID);

  bool GetTextTailPosition(DWORD dwVID, float *px, float *py, float *pz);
  bool IsChatTextTail(DWORD dwVID);

  void EnablePKTitle(BOOL bFlag);
  void AttachTitle(DWORD dwVID, const char *c_szName,
                   const D3DXCOLOR &c_rColor);
  void DetachTitle(DWORD dwVID);

  void AttachLevel(DWORD dwVID, const char *c_szText,
                   const D3DXCOLOR &c_rColor);
  void DetachLevel(DWORD dwVID);

protected:
  TTextTail *RegisterTextTail(DWORD dwVirtualID, const char *c_szText,
                              CGraphicObjectInstance *pOwner, float fHeight,
                              const D3DXCOLOR &c_rColor);
  void DeleteTextTail(TTextTail *pTextTail);

  void UpdateTextTail(TTextTail *pTextTail);
  void RenderTextTailBox(TTextTail *pTextTail);
  void RenderTextTailName(TTextTail *pTextTail);
  void UpdateDistance(const TPixelPosition &c_rCenterPosition,
                      TTextTail *pTextTail);

  bool isIn(TTextTail *pSource, TTextTail *pTarget);

protected:
  TTextTailMap m_CharacterTextTailMap;
  TTextTailMap m_ItemTextTailMap;
  TChatTailMap m_ChatTailMap;

  TTextTailList m_CharacterTextTailList;
  TTextTailList m_ItemTextTailList;

private:
  CDynamicPool<STextTail> m_TextTailPool;
};
