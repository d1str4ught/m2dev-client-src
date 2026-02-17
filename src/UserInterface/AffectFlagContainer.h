#pragma once

class CAffectFlagContainer {
public:
  enum {
    BIT_SIZE = 64,
    BYTE_SIZE = BIT_SIZE / 8 + (1 * ((BIT_SIZE & 7) ? 1 : 0)),
  };

public:
  CAffectFlagContainer();
  ~CAffectFlagContainer();

  void Clear();
  void CopyInstance(const CAffectFlagContainer &c_rkAffectContainer);
  void Set(UINT uPos, bool isSet);
  bool IsSet(UINT uPos) const;

  void CopyData(UINT uPos, UINT uByteSize, const void *c_pvData);

  void ConvertToPosition(unsigned *uRetX, unsigned *uRetY) const;

private:
  typedef unsigned char Element;

  Element m_aElement[BYTE_SIZE];
};