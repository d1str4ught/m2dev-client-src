#include "StdAfx.h"
#include "AffectFlagContainer.h"

CAffectFlagContainer::CAffectFlagContainer() { Clear(); }

CAffectFlagContainer::~CAffectFlagContainer() {}

void CAffectFlagContainer::Clear() {
  memset(m_aElement, 0, sizeof(m_aElement));
}

void CAffectFlagContainer::CopyInstance(
    const CAffectFlagContainer &c_rkAffectContainer) {
  memcpy(m_aElement, c_rkAffectContainer.m_aElement, sizeof(m_aElement));
}

void CAffectFlagContainer::CopyData(UINT uPos, UINT uByteSize,
                                    const void *c_pvData) {
  const uint8_t *c_pbData = (const uint8_t *)c_pvData;
  Element bMask = 0x01;

  UINT uBitEnd = uPos + uByteSize * 8;
  for (UINT i = uPos; i < uBitEnd; ++i) {
    Set(i, (*c_pbData & bMask) ? true : false);
    bMask <<= 1;

    if (bMask == 0) {
      ++c_pbData;
      bMask = 0x01;
    }
  }
}

void CAffectFlagContainer::ConvertToPosition(unsigned *uRetX,
                                             unsigned *uRetY) const {
  DWORD *pos = (DWORD *)m_aElement;
  *uRetX = pos[0];
  *uRetY = pos[1];
}
/*
const void * CAffectFlagContainer::GetDataPtr(UINT uPos) const
{
        if (uPos/8>=BYTE_SIZE)
        {
                return NULL;
        }

        return (const void *)&m_aElement[uPos];
}
*/

void CAffectFlagContainer::Set(UINT uPos, bool isSet) {
  if (uPos / 8 >= BYTE_SIZE) {
    TraceError("CAffectFlagContainer::Set(uPos=%d>%d, isSet=%d", uPos,
               BYTE_SIZE * 8, isSet);
    return;
  }

  uint8_t &rElement = m_aElement[uPos / 8];

  uint8_t bMask = uint8_t(1 << (uPos & 7));
  if (isSet)
    rElement |= bMask;
  else
    rElement &= ~bMask;
}

bool CAffectFlagContainer::IsSet(UINT uPos) const {
  if (uPos / 8 >= BYTE_SIZE) {
    TraceError("CAffectFlagContainer::IsSet(uPos=%d>%d", uPos, BYTE_SIZE * 8);
    return false;
  }

  const uint8_t &c_rElement = m_aElement[uPos / 8];

  uint8_t bMask = uint8_t(1 << (uPos & 7));
  if (c_rElement & bMask)
    return true;

  return false;
}
