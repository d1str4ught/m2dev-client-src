#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonSafeBox.h"
#include "PythonCharacterManager.h"

#include "AbstractPlayer.h"
#include "GameLib/ItemManager.h"

//////////////////////////////////////////////////////////////////////////
// SafeBox

bool CPythonNetworkStream::SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney) {
  assert(!"CPythonNetworkStream::SendSafeBoxMoneyPacket");
  return false;

  //	TPacketCGSafeboxMoney kSafeboxMoney;
  //	kSafeboxMoney.bHeader = CG::SAFEBOX_MONEY;
  //	kSafeboxMoney.bState = byState;
  //	kSafeboxMoney.dwMoney = dwMoney;
  //	if (!Send(sizeof(kSafeboxMoney), &kSafeboxMoney))
  //		return false;
  //
}

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos,
                                                    BYTE bySafeBoxPos) {
  __PlayInventoryItemDropSound(InventoryPos);

  TPacketCGSafeboxCheckin kSafeboxCheckin;
  kSafeboxCheckin.header = CG::SAFEBOX_CHECKIN;
  kSafeboxCheckin.length = sizeof(kSafeboxCheckin);
  kSafeboxCheckin.ItemPos = InventoryPos;
  kSafeboxCheckin.bSafePos = bySafeBoxPos;
  if (!Send(sizeof(kSafeboxCheckin), &kSafeboxCheckin))
    return false;

  return true;
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos,
                                                     TItemPos InventoryPos) {
  __PlaySafeBoxItemDropSound(bySafeBoxPos);

  TPacketCGSafeboxCheckout kSafeboxCheckout;
  kSafeboxCheckout.header = CG::SAFEBOX_CHECKOUT;
  kSafeboxCheckout.length = sizeof(kSafeboxCheckout);
  kSafeboxCheckout.bSafePos = bySafeBoxPos;
  kSafeboxCheckout.ItemPos = InventoryPos;
  if (!Send(sizeof(kSafeboxCheckout), &kSafeboxCheckout))
    return false;

  return true;
}

bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos,
                                                     BYTE byTargetPos,
                                                     BYTE byCount) {
  __PlaySafeBoxItemDropSound(bySourcePos);

  TPacketCGItemMove kItemMove;
  kItemMove.header = CG::SAFEBOX_ITEM_MOVE;
  kItemMove.length = sizeof(kItemMove);
  kItemMove.pos = TItemPos(INVENTORY, bySourcePos);
  kItemMove.num = byCount;
  kItemMove.change_pos = TItemPos(INVENTORY, byTargetPos);
  if (!Send(sizeof(kItemMove), &kItemMove))
    return false;

  return true;
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket() {
  TPacketGCItemSet kItemSet;
  if (!Recv(sizeof(kItemSet), &kItemSet))
    return false;

  TItemData kItemData;
  kItemData.vnum = kItemSet.vnum;
  kItemData.count = kItemSet.count;
  kItemData.flags = kItemSet.flags;
  kItemData.anti_flags = kItemSet.anti_flags;
  for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
    kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
  for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
    kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];

  CPythonSafeBox::Instance().SetItemData(kItemSet.pos.cell, kItemData);

  __RefreshSafeboxWindow();

  return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket() {
  TPacketGCItemDel kItemDel;
  if (!Recv(sizeof(kItemDel), &kItemDel))
    return false;

  CPythonSafeBox::Instance().DelItemData(kItemDel.pos.cell);

  __RefreshSafeboxWindow();

  return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket() {
  TPacketGCSafeboxWrongPassword kSafeboxWrongPassword;

  if (!Recv(sizeof(kSafeboxWrongPassword), &kSafeboxWrongPassword))
    return false;

  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError",
                        Py_BuildValue("()"));

  return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket() {
  TPacketGCSafeboxSize kSafeBoxSize;
  if (!Recv(sizeof(kSafeBoxSize), &kSafeBoxSize))
    return false;

  CPythonSafeBox::Instance().OpenSafeBox(kSafeBoxSize.bSize);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow",
                        Py_BuildValue("(i)", kSafeBoxSize.bSize));

  return true;
}

bool CPythonNetworkStream::RecvSafeBoxMoneyChangePacket() {
  TPacketGCSafeboxMoneyChange kMoneyChange;
  if (!Recv(sizeof(kMoneyChange), &kMoneyChange))
    return false;

  CPythonSafeBox::Instance().SetMoney(kMoneyChange.lMoney);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafeboxMoney",
                        Py_BuildValue("()"));

  return true;
}

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(BYTE byMallPos,
                                                  TItemPos InventoryPos) {
  __PlayMallItemDropSound(byMallPos);

  TPacketCGMallCheckout kMallCheckoutPacket;
  kMallCheckoutPacket.header = CG::MALL_CHECKOUT;
  kMallCheckoutPacket.length = sizeof(kMallCheckoutPacket);
  kMallCheckoutPacket.bMallPos = byMallPos;
  kMallCheckoutPacket.ItemPos = InventoryPos;
  if (!Send(sizeof(kMallCheckoutPacket), &kMallCheckoutPacket))
    return false;

  return true;
}

bool CPythonNetworkStream::RecvMallOpenPacket() {
  TPacketGCMallOpen kMallOpen;
  if (!Recv(sizeof(kMallOpen), &kMallOpen))
    return false;

  CPythonSafeBox::Instance().OpenMall(kMallOpen.bSize);
  PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow",
                        Py_BuildValue("(i)", kMallOpen.bSize));

  return true;
}
bool CPythonNetworkStream::RecvMallItemSetPacket() {
  TPacketGCItemSet kItemSet;
  if (!Recv(sizeof(kItemSet), &kItemSet))
    return false;

  TItemData kItemData;
  kItemData.vnum = kItemSet.vnum;
  kItemData.count = kItemSet.count;
  kItemData.flags = kItemSet.flags;
  kItemData.anti_flags = kItemSet.anti_flags;
  for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
    kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
  for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
    kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];

  CPythonSafeBox::Instance().SetMallItemData(kItemSet.pos.cell, kItemData);

  __RefreshMallWindow();

  return true;
}
bool CPythonNetworkStream::RecvMallItemDelPacket() {
  TPacketGCItemDel kItemDel;
  if (!Recv(sizeof(kItemDel), &kItemDel))
    return false;

  CPythonSafeBox::Instance().DelMallItemData(kItemDel.pos.cell);

  __RefreshMallWindow();
  Tracef(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

  return true;
}
// Mall
//////////////////////////////////////////////////////////////////////////

// Item
// Recieve
bool CPythonNetworkStream::RecvItemDelPacket() {
  TPacketGCItemDel packet_item_set;
  if (!Recv(sizeof(TPacketGCItemDel), &packet_item_set))
    return false;

  TItemData kItemData;
  memset(&kItemData, 0, sizeof(TItemData));

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.SetItemData(packet_item_set.pos, kItemData);

  __RefreshInventoryWindow();
  return true;
}

bool CPythonNetworkStream::RecvItemSetPacket() {
  TPacketGCItemSet packet_item_set;
  if (!Recv(sizeof(TPacketGCItemSet), &packet_item_set))
    return false;

  TItemData kItemData;
  kItemData.vnum = packet_item_set.vnum;
  kItemData.count = packet_item_set.count;
  kItemData.flags = packet_item_set.flags;
  kItemData.anti_flags = packet_item_set.anti_flags;

  for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
    kItemData.alSockets[i] = packet_item_set.alSockets[i];
  for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
    kItemData.aAttr[j] = packet_item_set.aAttr[j];

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.SetItemData(packet_item_set.pos, kItemData);

  if (packet_item_set.highlight)
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_Highlight_Item",
                          Py_BuildValue("(ii)", packet_item_set.pos.window_type,
                                        packet_item_set.pos.cell));

  __RefreshInventoryWindow();
  return true;
}

bool CPythonNetworkStream::RecvItemGetPacket() {
  TPacketGCItemGet packet;
  if (!Recv(sizeof(TPacketGCItemGet), &packet))
    return false;

  CItemData *pItemData;
  if (!CItemManager::Instance().GetItemDataPointer(packet.dwItemVnum,
                                                   &pItemData)) {
    TraceError("CPythonNetworkStream::RecvItemGetPacket - Unknown item vnum %u",
               packet.dwItemVnum);
    return true;
  }

  if (packet.bArg == 0) {
    // Normal pickup
    PyCallClassMemberFunc(
        m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ItemGet",
        Py_BuildValue("(si)", pItemData->GetName(), packet.bCount));
  } else if (packet.bArg == 1) {
    // Received from party member
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_ItemGetFromParty",
                          Py_BuildValue("(ssi)", pItemData->GetName(),
                                        packet.szFromName, packet.bCount));
  } else if (packet.bArg == 2) {
    // Delivered to party member
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_ItemDeliverToParty",
                          Py_BuildValue("(ssi)", pItemData->GetName(),
                                        packet.szFromName, packet.bCount));
  }

  return true;
}

bool CPythonNetworkStream::RecvItemUsePacket() {
  TPacketGCItemUse packet_item_use;

  if (!Recv(sizeof(TPacketGCItemUse), &packet_item_use))
    return false;

  __RefreshInventoryWindow();
  return true;
}

bool CPythonNetworkStream::RecvItemUpdatePacket() {
  TPacketGCItemUpdate packet_item_update;

  if (!Recv(sizeof(TPacketGCItemUpdate), &packet_item_update))
    return false;

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count);
  for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
    rkPlayer.SetItemMetinSocket(packet_item_update.Cell, i,
                                packet_item_update.alSockets[i]);
  for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
    rkPlayer.SetItemAttribute(packet_item_update.Cell, j,
                              packet_item_update.aAttr[j].bType,
                              packet_item_update.aAttr[j].sValue);

  __RefreshInventoryWindow();
  return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket() {
  TPacketGCItemGroundAdd packet_item_ground_add;

  if (!Recv(sizeof(TPacketGCItemGroundAdd), &packet_item_ground_add))
    return false;

  __GlobalPositionToLocalPosition(packet_item_ground_add.lX,
                                  packet_item_ground_add.lY);

  CPythonItem::Instance().CreateItem(
      packet_item_ground_add.dwVID, packet_item_ground_add.dwVnum,
      packet_item_ground_add.lX, packet_item_ground_add.lY,
      packet_item_ground_add.lZ);
  return true;
}

bool CPythonNetworkStream::RecvItemOwnership() {
  TPacketGCItemOwnership p;

  if (!Recv(sizeof(TPacketGCItemOwnership), &p))
    return false;

  CPythonItem::Instance().SetOwnership(p.dwVID, p.szName);
  return true;
}

bool CPythonNetworkStream::RecvItemGroundDelPacket() {
  TPacketGCItemGroundDel packet_item_ground_del;

  if (!Recv(sizeof(TPacketGCItemGroundDel), &packet_item_ground_del))
    return false;

  CPythonItem::Instance().DeleteItem(packet_item_ground_del.vid);
  return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket() {
  TPacketGCQuickSlotAdd packet_quick_slot_add;

  if (!Recv(sizeof(TPacketGCQuickSlotAdd), &packet_quick_slot_add))
    return false;

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.AddQuickSlot(packet_quick_slot_add.pos,
                        packet_quick_slot_add.slot.Type,
                        packet_quick_slot_add.slot.Position);

  __RefreshInventoryWindow();

  return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket() {
  TPacketGCQuickSlotDel packet_quick_slot_del;

  if (!Recv(sizeof(TPacketGCQuickSlotDel), &packet_quick_slot_del))
    return false;

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.DeleteQuickSlot(packet_quick_slot_del.pos);

  __RefreshInventoryWindow();

  return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket() {
  TPacketGCQuickSlotSwap packet_quick_slot_swap;

  if (!Recv(sizeof(TPacketGCQuickSlotSwap), &packet_quick_slot_swap))
    return false;

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.MoveQuickSlot(packet_quick_slot_swap.pos,
                         packet_quick_slot_swap.change_pos);

  __RefreshInventoryWindow();

  return true;
}

bool CPythonNetworkStream::SendShopEndPacket() {
  if (!__CanActMainInstance())
    return true;

  TPacketCGShop packet_shop;
  packet_shop.header = CG::SHOP;
  packet_shop.length = sizeof(packet_shop);
  packet_shop.subheader = ShopSub::CG::END;

  if (!Send(sizeof(packet_shop), &packet_shop)) {
    Tracef("SendShopEndPacket Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendShopBuyPacket(BYTE bPos) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGShop PacketShop;
  PacketShop.header = CG::SHOP;
  PacketShop.length = sizeof(PacketShop) + sizeof(BYTE) + sizeof(BYTE);
  PacketShop.subheader = ShopSub::CG::BUY;

  if (!Send(sizeof(TPacketCGShop), &PacketShop)) {
    Tracef("SendShopBuyPacket Error\n");
    return false;
  }

  BYTE bCount = 1;
  if (!Send(sizeof(BYTE), &bCount)) {
    Tracef("SendShopBuyPacket Error\n");
    return false;
  }

  if (!Send(sizeof(BYTE), &bPos)) {
    Tracef("SendShopBuyPacket Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendShopSellPacket(BYTE bySlot) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGShop PacketShop;
  PacketShop.header = CG::SHOP;
  PacketShop.length = sizeof(PacketShop) + sizeof(BYTE);
  PacketShop.subheader = ShopSub::CG::SELL;

  if (!Send(sizeof(TPacketCGShop), &PacketShop)) {
    Tracef("SendShopSellPacket Error\n");
    return false;
  }
  if (!Send(sizeof(BYTE), &bySlot)) {
    Tracef("SendShopAddSellPacket Error\n");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, BYTE byCount) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGShop PacketShop;
  PacketShop.header = CG::SHOP;
  PacketShop.length = sizeof(PacketShop) + sizeof(BYTE) + sizeof(BYTE);
  PacketShop.subheader = ShopSub::CG::SELL2;

  if (!Send(sizeof(TPacketCGShop), &PacketShop)) {
    Tracef("SendShopSellPacket Error\n");
    return false;
  }
  if (!Send(sizeof(BYTE), &bySlot)) {
    Tracef("SendShopAddSellPacket Error\n");
    return false;
  }
  if (!Send(sizeof(BYTE), &byCount)) {
    Tracef("SendShopAddSellPacket Error\n");
    return false;
  }

  Tracef(" SendShopSellPacketNew(bySlot=%d, byCount=%d)\n", bySlot, byCount);

  return true;
}

// Send
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos) {
  if (!__CanActMainInstance())
    return true;

  if (__IsEquipItemInSlot(pos)) {
    if (CPythonExchange::Instance().isTrading()) {
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "BINARY_AppendNotifyMessage",
                            Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
      return true;
    }

    if (CPythonShop::Instance().IsOpen()) {
      PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                            "BINARY_AppendNotifyMessage",
                            Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
      return true;
    }

    if (__IsPlayerAttacking())
      return true;
  }

  __PlayInventoryItemUseSound(pos);

  TPacketCGItemUse itemUsePacket;
  itemUsePacket.header = CG::ITEM_USE;
  itemUsePacket.length = sizeof(itemUsePacket);
  itemUsePacket.pos = pos;

  if (!Send(sizeof(TPacketCGItemUse), &itemUsePacket)) {
    Tracen("SendItemUsePacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos,
                                                   TItemPos target_pos) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGItemUseToItem itemUseToItemPacket;
  itemUseToItemPacket.header = CG::ITEM_USE_TO_ITEM;
  itemUseToItemPacket.length = sizeof(itemUseToItemPacket);
  itemUseToItemPacket.source_pos = source_pos;
  itemUseToItemPacket.target_pos = target_pos;

  if (!Send(sizeof(TPacketCGItemUseToItem), &itemUseToItemPacket)) {
    Tracen("SendItemUseToItemPacket Error");
    return false;
  }

#ifdef _DEBUG
  Tracef(" << SendItemUseToItemPacket(src=%d, dst=%d)\n", source_pos,
         target_pos);
#endif

  return true;
}

bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, DWORD elk) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGItemDrop itemDropPacket;
  itemDropPacket.header = CG::ITEM_DROP;
  itemDropPacket.length = sizeof(itemDropPacket);
  itemDropPacket.pos = pos;
  itemDropPacket.elk = elk;

  if (!Send(sizeof(TPacketCGItemDrop), &itemDropPacket)) {
    Tracen("SendItemDropPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, DWORD elk,
                                                 DWORD count) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGItemDrop2 itemDropPacket;
  itemDropPacket.header = CG::ITEM_DROP2;
  itemDropPacket.length = sizeof(itemDropPacket);
  itemDropPacket.pos = pos;
  itemDropPacket.gold = elk;
  itemDropPacket.count = count;

  if (!Send(sizeof(itemDropPacket), &itemDropPacket)) {
    Tracen("SendItemDropPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos) {
  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos) {
  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  DWORD dwItemID = rkPlayer.GetItemIndex(uSlotPos);

  CPythonItem &rkItem = CPythonItem::Instance();
  rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos) {
  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  DWORD dwItemID = rkPlayer.GetItemIndex(uSlotPos);

  CPythonItem &rkItem = CPythonItem::Instance();
  rkItem.PlayDropSound(dwItemID);
}

// void CPythonNetworkStream::__PlayShopItemDropSound(UINT uSlotPos)
//{
//	DWORD dwItemID;
//	CPythonShop& rkShop=CPythonShop::Instance();
//	if (!rkShop.GetSlotItemID(uSlotPos, &dwItemID))
//		return;
//
//	CPythonItem& rkItem=CPythonItem::Instance();
//	rkItem.PlayDropSound(dwItemID);
// }

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(UINT uSlotPos) {
  DWORD dwItemID;
  CPythonSafeBox &rkSafeBox = CPythonSafeBox::Instance();
  if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
    return;

  CPythonItem &rkItem = CPythonItem::Instance();
  rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(UINT uSlotPos) {
  DWORD dwItemID;
  CPythonSafeBox &rkSafeBox = CPythonSafeBox::Instance();
  if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
    return;

  CPythonItem &rkItem = CPythonItem::Instance();
  rkItem.PlayDropSound(dwItemID);
}

bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos,
                                              BYTE num) {
  if (!__CanActMainInstance())
    return true;

  if (__IsEquipItemInSlot(pos)) {
    if (CPythonExchange::Instance().isTrading()) {
      if (pos.IsEquipCell() || change_pos.IsEquipCell()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendNotifyMessage",
                              Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
        return true;
      }
    }

    if (CPythonShop::Instance().IsOpen()) {
      if (pos.IsEquipCell() || change_pos.IsEquipCell()) {
        PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                              "BINARY_AppendNotifyMessage",
                              Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
        return true;
      }
    }

    if (__IsPlayerAttacking())
      return true;
  }

  __PlayInventoryItemDropSound(pos);

  TPacketCGItemMove itemMovePacket;
  itemMovePacket.header = CG::ITEM_MOVE;
  itemMovePacket.length = sizeof(itemMovePacket);
  itemMovePacket.pos = pos;
  itemMovePacket.change_pos = change_pos;
  itemMovePacket.num = num;

  if (!Send(sizeof(TPacketCGItemMove), &itemMovePacket)) {
    Tracen("SendItemMovePacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendItemPickUpPacket(DWORD vid) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGItemPickUp itemPickUpPacket;
  itemPickUpPacket.header = CG::ITEM_PICKUP;
  itemPickUpPacket.length = sizeof(itemPickUpPacket);
  itemPickUpPacket.vid = vid;

  if (!Send(sizeof(TPacketCGItemPickUp), &itemPickUpPacket)) {
    Tracen("SendItemPickUpPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendQuickSlotAddPacket(BYTE wpos, BYTE type,
                                                  BYTE pos) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGQuickSlotAdd quickSlotAddPacket;

  quickSlotAddPacket.header = CG::QUICKSLOT_ADD;
  quickSlotAddPacket.length = sizeof(quickSlotAddPacket);
  quickSlotAddPacket.pos = wpos;
  quickSlotAddPacket.slot.Type = type;
  quickSlotAddPacket.slot.Position = pos;

  if (!Send(sizeof(TPacketCGQuickSlotAdd), &quickSlotAddPacket)) {
    Tracen("SendQuickSlotAddPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(BYTE pos) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGQuickSlotDel quickSlotDelPacket;

  quickSlotDelPacket.header = CG::QUICKSLOT_DEL;
  quickSlotDelPacket.length = sizeof(quickSlotDelPacket);
  quickSlotDelPacket.pos = pos;

  if (!Send(sizeof(TPacketCGQuickSlotDel), &quickSlotDelPacket)) {
    Tracen("SendQuickSlotDelPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(BYTE pos, BYTE change_pos) {
  if (!__CanActMainInstance())
    return true;

  TPacketCGQuickSlotSwap quickSlotSwapPacket;

  quickSlotSwapPacket.header = CG::QUICKSLOT_SWAP;
  quickSlotSwapPacket.length = sizeof(quickSlotSwapPacket);
  quickSlotSwapPacket.pos = pos;
  quickSlotSwapPacket.change_pos = change_pos;

  if (!Send(sizeof(TPacketCGQuickSlotSwap), &quickSlotSwapPacket)) {
    Tracen("SendQuickSlotSwapPacket Error");
    return false;
  }

  return true;
}

bool CPythonNetworkStream::RecvSpecialEffect() {
  TPacketGCSpecialEffect kSpecialEffect;
  if (!Recv(sizeof(kSpecialEffect), &kSpecialEffect))
    return false;

  DWORD effect = -1;
  bool bPlayPotionSound = false; // 포션을 먹을 경우는 포션 사운드를 출력하자.!!
  bool bAttachEffect = true;     // 캐리터에 붙는 어태치 이펙트와 일반 이펙트
                                 // 구분.!!
  switch (kSpecialEffect.type) {
  case SE_HPUP_RED:
    effect = CInstanceBase::EFFECT_HPUP_RED;
    bPlayPotionSound = true;
    break;
  case SE_SPUP_BLUE:
    effect = CInstanceBase::EFFECT_SPUP_BLUE;
    bPlayPotionSound = true;
    break;
  case SE_SPEEDUP_GREEN:
    effect = CInstanceBase::EFFECT_SPEEDUP_GREEN;
    bPlayPotionSound = true;
    break;
  case SE_DXUP_PURPLE:
    effect = CInstanceBase::EFFECT_DXUP_PURPLE;
    bPlayPotionSound = true;
    break;
  case SE_CRITICAL:
    effect = CInstanceBase::EFFECT_CRITICAL;
    break;
  case SE_PENETRATE:
    effect = CInstanceBase::EFFECT_PENETRATE;
    break;
  case SE_BLOCK:
    effect = CInstanceBase::EFFECT_BLOCK;
    break;
  case SE_DODGE:
    effect = CInstanceBase::EFFECT_DODGE;
    break;
  case SE_CHINA_FIREWORK:
    effect = CInstanceBase::EFFECT_FIRECRACKER;
    bAttachEffect = false;
    break;
  case SE_SPIN_TOP:
    effect = CInstanceBase::EFFECT_SPIN_TOP;
    bAttachEffect = false;
    break;
  case SE_SUCCESS:
    effect = CInstanceBase::EFFECT_SUCCESS;
    bAttachEffect = false;
    break;
  case SE_FAIL:
    effect = CInstanceBase::EFFECT_FAIL;
    break;
  case SE_FR_SUCCESS:
    effect = CInstanceBase::EFFECT_FR_SUCCESS;
    bAttachEffect = false;
    break;
  case SE_LEVELUP_ON_14_FOR_GERMANY: // 레벨업 14일때 ( 독일전용 )
    effect = CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY;
    bAttachEffect = false;
    break;
  case SE_LEVELUP_UNDER_15_FOR_GERMANY: // 레벨업 15일때 ( 독일전용 )
    effect = CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY;
    bAttachEffect = false;
    break;
  case SE_PERCENT_DAMAGE1:
    effect = CInstanceBase::EFFECT_PERCENT_DAMAGE1;
    break;
  case SE_PERCENT_DAMAGE2:
    effect = CInstanceBase::EFFECT_PERCENT_DAMAGE2;
    break;
  case SE_PERCENT_DAMAGE3:
    effect = CInstanceBase::EFFECT_PERCENT_DAMAGE3;
    break;
  case SE_AUTO_HPUP:
    effect = CInstanceBase::EFFECT_AUTO_HPUP;
    break;
  case SE_AUTO_SPUP:
    effect = CInstanceBase::EFFECT_AUTO_SPUP;
    break;
  case SE_EQUIP_RAMADAN_RING:
    effect = CInstanceBase::EFFECT_RAMADAN_RING_EQUIP;
    break;
  case SE_EQUIP_HALLOWEEN_CANDY:
    effect = CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP;
    break;
  case SE_EQUIP_HAPPINESS_RING:
    effect = CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP;
    break;
  case SE_EQUIP_LOVE_PENDANT:
    effect = CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP;
    break;
  case SE_AGGREGATE_MONSTER:
    effect = CInstanceBase::EFFECT_AGGREGATE_MONSTER;
    break;

  default:
    TraceError("%d 는 없는 스페셜 이펙트 번호입니다.TPacketGCSpecialEffect",
               kSpecialEffect.type);
    break;
  }

  if (bPlayPotionSound) {
    IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
    if (rkPlayer.IsMainCharacterIndex(kSpecialEffect.vid)) {
      CPythonItem &rkItem = CPythonItem::Instance();
      rkItem.PlayUsePotionSound();
    }
  }

  if (-1 != effect) {
    CInstanceBase *pInstance =
        CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
    if (pInstance) {
      if (bAttachEffect)
        pInstance->AttachSpecialEffect(effect);
      else
        pInstance->CreateSpecialEffect(effect);
    }
  }

  return true;
}

bool CPythonNetworkStream::RecvSpecificEffect() {
  TPacketGCSpecificEffect kSpecificEffect;
  if (!Recv(sizeof(kSpecificEffect), &kSpecificEffect))
    return false;

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(kSpecificEffect.vid);
  // EFFECT_TEMP
  if (pInstance) {
    CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "",
                                  kSpecificEffect.effect_file, false);
    pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
  }

  return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine() {
  TPacketGCDragonSoulRefine kDragonSoul;

  if (!Recv(sizeof(kDragonSoul), &kDragonSoul))
    return false;

  switch (kDragonSoul.bSubType) {
  case DragonSoulSub::OPEN:
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_DragonSoulRefineWindow_Open",
                          Py_BuildValue("()"));
    break;
  case DragonSoulSub::REFINE_FAIL:
  case DragonSoulSub::REFINE_FAIL_MAX_REFINE:
  case DragonSoulSub::REFINE_FAIL_INVALID_MATERIAL:
  case DragonSoulSub::REFINE_FAIL_NOT_ENOUGH_MONEY:
  case DragonSoulSub::REFINE_FAIL_NOT_ENOUGH_MATERIAL:
  case DragonSoulSub::REFINE_FAIL_TOO_MUCH_MATERIAL:
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_DragonSoulRefineWindow_RefineFail",
                          Py_BuildValue("(iii)", kDragonSoul.bSubType,
                                        kDragonSoul.Pos.window_type,
                                        kDragonSoul.Pos.cell));
    break;
  case DragonSoulSub::REFINE_SUCCEED:
    PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
                          "BINARY_DragonSoulRefineWindow_RefineSucceed",
                          Py_BuildValue("(ii)", kDragonSoul.Pos.window_type,
                                        kDragonSoul.Pos.cell));
    break;
  }

  return true;
}
