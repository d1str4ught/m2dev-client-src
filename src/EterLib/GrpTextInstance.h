#ifndef __INC_ETERLIB_GRPTEXTINSTANCE_H__
#define __INC_ETERLIB_GRPTEXTINSTANCE_H__

#include "Pool.h"
#include "GrpText.h"

class CGraphicTextInstance {
public:
  typedef CDynamicPool<CGraphicTextInstance> TPool;

public:
  enum EHorizontalAlign {
    HORIZONTAL_ALIGN_LEFT = 0x01,
    HORIZONTAL_ALIGN_CENTER = 0x02,
    HORIZONTAL_ALIGN_RIGHT = 0x03,
  };

  enum EVerticalAlign {
    VERTICAL_ALIGN_TOP = 0x10,
    VERTICAL_ALIGN_CENTER = 0x20,
    VERTICAL_ALIGN_BOTTOM = 0x30
  };

  enum class ETextDirection : unsigned char {
    Auto = 0,
    LTR = 1,
    RTL = 2,
  };

public:
  static void Hyperlink_UpdateMousePos(int x, int y);
  static int Hyperlink_GetText(char *buf, int len);

public:
  CGraphicTextInstance();
  virtual ~CGraphicTextInstance();

  void Destroy();

  void Update();
  void Render(RECT *pClipRect = NULL);

  void ShowCursor();
  void HideCursor();

  void ShowOutLine();
  void HideOutLine();

  void SetColor(DWORD color);
  void SetColor(float r, float g, float b, float a = 1.0f);

  void SetOutLineColor(DWORD color);
  void SetOutLineColor(float r, float g, float b, float a = 1.0f);

  void SetHorizonalAlign(int hAlign);
  void SetVerticalAlign(int vAlign);
  void SetMax(int iMax);
  void SetTextPointer(CGraphicText *pText);
  void SetValueString(const std::string &c_stValue);
  void SetValue(const char *c_szValue, size_t len = -1);
  void SetChatValue(const char *c_szName,
                    const char *c_szMessage); // Chat-specific setter with
                                              // name/message separation
  void SetPosition(float fx, float fy, float fz = 0.0f);
  void SetSecret(bool Value);
  void SetOutline(bool Value);
  void SetFeather(bool Value);
  void SetMultiLine(bool Value);
  void SetLimitWidth(float fWidth);

  void GetTextSize(int *pRetWidth, int *pRetHeight);
  const std::string &GetValueStringReference();
  WORD GetTextLineCount();

  void SetTextDirection(ETextDirection dir);
  ETextDirection GetTextDirection() const { return m_direction; }
  bool IsRTL() const {
    // For AUTO mode, use computed result from BiDi analysis
    if (m_direction == ETextDirection::Auto)
      return m_computedRTL;
    // For explicit direction, derive from m_direction
    return (m_direction == ETextDirection::RTL);
  }

  int PixelPositionToCharacterPosition(int iPixelPosition);
  int GetHorizontalAlign();

protected:
  void __Initialize();
  int __DrawCharacter(CGraphicFontTexture *pFontTexture, wchar_t text,
                      DWORD dwColor, wchar_t prevChar = 0);
  void __GetTextPos(DWORD index, float *x, float *y);

protected:
  struct SHyperlink {
    short sx;
    short ex;
    std::wstring text;

    SHyperlink() : sx(0), ex(0) {}
  };

protected:
  DWORD m_dwTextColor;
  DWORD m_dwOutLineColor;

  WORD m_textWidth;
  WORD m_textHeight;

  BYTE m_hAlign;
  BYTE m_vAlign;

  WORD m_iMax;
  float m_fLimitWidth;

  bool m_isCursor;
  bool m_isSecret;
  bool m_isMultiLine;

  bool m_isOutline;
  float m_fFontFeather;

  /////

  std::string m_stText;
  D3DXVECTOR3 m_v3Position;

private:
  bool m_isUpdate;
  bool m_computedRTL; // Result of BiDi analysis (used when m_direction == Auto)
  bool m_isChatMessage; // True if this text was set via SetChatValue (has
                        // separated name/message)
  std::string
      m_chatName; // Chat sender name (only used when m_isChatMessage is true)
  std::string m_chatMessage; // Chat message text (only used when
                             // m_isChatMessage is true)

  CGraphicText::TRef m_roText;
  CGraphicFontTexture::TPCharacterInfomationVector m_pCharInfoVector;
  std::vector<float> m_kernVector;
  std::vector<DWORD> m_dwColorInfoVector;
  std::vector<SHyperlink> m_hyperlinkVector;
  std::vector<int> m_logicalToVisualPos; // Maps logical cursor pos (UTF-16 with
                                         // tags) to visual pos (rendered chars)
  std::vector<int>
      m_visualToLogicalPos; // Reverse mapping: visual pos -> logical pos
  ETextDirection m_direction =
      ETextDirection::Auto; // Will be overwritten by __Initialize()

public:
  static void CreateSystem(UINT uCapacity);
  static void DestroySystem();

  static CGraphicTextInstance *New();
  static void Delete(CGraphicTextInstance *pkInst);

  static CDynamicPool<CGraphicTextInstance> ms_kPool;
};

#endif