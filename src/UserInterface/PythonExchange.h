#pragma once

#include "Packet.h"

/*
 *	교환 창 관련
 */
class CPythonExchange : public CSingleton<CPythonExchange> {
public:
  enum {
    EXCHANGE_ITEM_MAX_NUM = 12,
  };

  typedef struct trade {
    char name[CHARACTER_NAME_MAX_LEN + 1];

    DWORD item_vnum[EXCHANGE_ITEM_MAX_NUM];
    BYTE item_count[EXCHANGE_ITEM_MAX_NUM];
    DWORD item_metin[EXCHANGE_ITEM_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute item_attr[EXCHANGE_ITEM_MAX_NUM]
                                  [ITEM_ATTRIBUTE_SLOT_MAX_NUM];

    BYTE accept;
    DWORD elk;
  } TExchangeData;

public:
  CPythonExchange();
  virtual ~CPythonExchange();

  void Clear();

  void Start();
  void End();
  bool isTrading();

  // Interface

  void SetSelfName(const char *name);
  void SetTargetName(const char *name);

  char *GetNameFromSelf();
  char *GetNameFromTarget();

  void SetElkToTarget(DWORD elk);
  void SetElkToSelf(DWORD elk);

  DWORD GetElkFromTarget();
  DWORD GetElkFromSelf();

  void SetItemToTarget(DWORD pos, DWORD vnum, BYTE count);
  void SetItemToSelf(DWORD pos, DWORD vnum, BYTE count);

  void SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum);
  void SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum);

  void SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType,
                                short sValue);
  void SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue);

  void DelItemOfTarget(BYTE pos);
  void DelItemOfSelf(BYTE pos);

  DWORD GetItemVnumFromTarget(BYTE pos);
  DWORD GetItemVnumFromSelf(BYTE pos);

  BYTE GetItemCountFromTarget(BYTE pos);
  BYTE GetItemCountFromSelf(BYTE pos);

  DWORD GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos);
  DWORD GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos);

  void GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE *pbyType,
                                  short *psValue);
  void GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE *pbyType,
                                short *psValue);

  void SetAcceptToTarget(BYTE Accept);
  void SetAcceptToSelf(BYTE Accept);

  bool GetAcceptFromTarget();
  bool GetAcceptFromSelf();

  bool GetElkMode();
  void SetElkMode(bool value);

protected:
  bool m_isTrading;

  bool m_elk_mode; // 엘크를 클릭해서 교환했을때를 위한 변종임.
  TExchangeData m_self;
  TExchangeData m_victim;
};
