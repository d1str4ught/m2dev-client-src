#include "StdAfx.h"
#include "GameType.h"

std::string g_strResourcePath = "d:/ymir work/";
std::string g_strImagePath = "d:/ymir work/ui/";

std::string g_strGuildSymbolPathName = "mark/10/";

// DEFAULT_FONT
static std::string gs_strDefaultFontName = "굴림체:12.fnt";
static std::string gs_strDefaultItalicFontName = "굴림체:12i.fnt";
static CResource *gs_pkDefaultFont = NULL;
static CResource *gs_pkDefaultItalicFont = NULL;

static bool gs_isReloadDefaultFont = false;

void DefaultFont_Startup() { gs_pkDefaultFont = NULL; }

void DefaultFont_Cleanup() {
  if (gs_pkDefaultFont)
    gs_pkDefaultFont->Release();
}

void DefaultFont_SetName(const char *c_szFontName) {
  gs_strDefaultFontName = c_szFontName;
  gs_strDefaultFontName += ".fnt";

  gs_strDefaultItalicFontName = c_szFontName;
  if (strchr(c_szFontName, ':'))
    gs_strDefaultItalicFontName += "i";
  gs_strDefaultItalicFontName += ".fnt";

  gs_isReloadDefaultFont = true;
}

bool ReloadDefaultFonts() {
  CResourceManager &rkResMgr = CResourceManager::Instance();

  gs_isReloadDefaultFont = false;

  CResource *pkNewFont =
      rkResMgr.GetResourcePointer(gs_strDefaultFontName.c_str());
  pkNewFont->AddReference();
  if (gs_pkDefaultFont)
    gs_pkDefaultFont->Release();
  gs_pkDefaultFont = pkNewFont;

  CResource *pkNewItalicFont =
      rkResMgr.GetResourcePointer(gs_strDefaultItalicFontName.c_str());
  pkNewItalicFont->AddReference();
  if (gs_pkDefaultItalicFont)
    gs_pkDefaultItalicFont->Release();
  gs_pkDefaultItalicFont = pkNewItalicFont;

  return true;
}

CResource *DefaultFont_GetResource() {
  if (!gs_pkDefaultFont || gs_isReloadDefaultFont)
    ReloadDefaultFonts();
  return gs_pkDefaultFont;
}

CResource *DefaultItalicFont_GetResource() {
  if (!gs_pkDefaultItalicFont || gs_isReloadDefaultFont)
    ReloadDefaultFonts();
  return gs_pkDefaultItalicFont;
}

// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char *c_szPathName) {
  g_strGuildSymbolPathName = "mark/";
  g_strGuildSymbolPathName += c_szPathName;
  g_strGuildSymbolPathName += "/";
}

const char *GetGuildSymbolFileName(DWORD dwGuildID) {
  return _getf("%s%03d.jpg", g_strGuildSymbolPathName.c_str(), dwGuildID);
}

BYTE c_aSlotTypeToInvenType[SLOT_TYPE_MAX] = {
    RESERVED_WINDOW, // SLOT_TYPE_NONE
    INVENTORY,       // SLOT_TYPE_INVENTORY
    RESERVED_WINDOW, // SLOT_TYPE_SKILL
    RESERVED_WINDOW, // SLOT_TYPE_EMOTION
    RESERVED_WINDOW, // SLOT_TYPE_SHOP
    RESERVED_WINDOW, // SLOT_TYPE_EXCHANGE_OWNER
    RESERVED_WINDOW, // SLOT_TYPE_EXCHANGE_TARGET
    RESERVED_WINDOW, // SLOT_TYPE_QUICK_SLOT
    RESERVED_WINDOW, // SLOT_TYPE_SAFEBOX	<- SAFEBOX, MALL의 경우 하드
                     // 코딩되어있는 LEGACY 코드를 유지함.
    RESERVED_WINDOW, // SLOT_TYPE_PRIVATE_SHOP
    RESERVED_WINDOW, // SLOT_TYPE_MALL		<- SAFEBOX, MALL의 경우 하드
                     // 코딩되어있는 LEGACY 코드를 유지함.
    DRAGON_SOUL_INVENTORY, // SLOT_TYPE_DRAGON_SOUL_INVENTORY
};

BYTE SlotTypeToInvenType(BYTE bSlotType) {
  if (bSlotType >= SLOT_TYPE_MAX)
    return RESERVED_WINDOW;
  else
    return c_aSlotTypeToInvenType[bSlotType];
}
