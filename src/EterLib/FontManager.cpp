#include "StdAfx.h"
#include "FontManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include <algorithm>
#include <cctype>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

static std::string ToLower(const std::string &s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

static bool FileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

CFontManager::CFontManager() : m_ftLibrary(nullptr), m_bInitialized(false) {}

CFontManager::~CFontManager() { Destroy(); }

CFontManager &CFontManager::Instance() {
  static CFontManager instance;
  return instance;
}

bool CFontManager::Initialize() {
  if (m_bInitialized)
    return true;

  if (FT_Init_FreeType(&m_ftLibrary) != 0) {
    TraceError("CFontManager::Initialize - FT_Init_FreeType failed");
    return false;
  }

  m_bInitialized = true;

  // Enable LCD subpixel filter to reduce color fringing
  FT_Library_SetLcdFilter(m_ftLibrary, FT_LCD_FILTER_DEFAULT);

  // Register default font name -> file mappings
  // Korean system fonts
  m_fontPathMap["gulim"] = "gulim.ttc";
  m_fontPathMap["\xea\xb5\xb4\xeb\xa6\xbc"] = "gulim.ttc"; // 굴림 (UTF-8)
  m_fontPathMap["\xea\xb5\xb4\xeb\xa6\xbc\xec\xb2\xb4"] =
      "gulimche.ttc"; // 굴림체

  // Common Latin fonts
  m_fontPathMap["arial"] = "arial.ttf";
  m_fontPathMap["arial bold"] = "arialbd.ttf";
  m_fontPathMap["tahoma"] = "tahoma.ttf";
  m_fontPathMap["tahoma bold"] = "tahomabd.ttf";
  m_fontPathMap["verdana"] = "verdana.ttf";
  m_fontPathMap["verdana bold"] = "verdanab.ttf";
  m_fontPathMap["times new roman"] = "times.ttf";
  m_fontPathMap["courier new"] = "cour.ttf";
  m_fontPathMap["segoe ui"] = "segoeui.ttf";

  return true;
}

void CFontManager::Destroy() {
  m_resolvedPathCache.clear();
  m_fontPathMap.clear();

  if (m_ftLibrary) {
    FT_Done_FreeType(m_ftLibrary);
    m_ftLibrary = nullptr;
  }

  m_bInitialized = false;
}

std::string CFontManager::ResolveFontPath(const char *faceName) {
  if (!faceName || !faceName[0])
    return "";

  std::string lowerName = ToLower(faceName);

  // 1. Check explicit mappings
  auto it = m_fontPathMap.find(lowerName);
  std::string fileName;
  if (it != m_fontPathMap.end())
    fileName = it->second;
  else
    fileName = lowerName + ".ttf";

  // 2. Check local fonts/ directory first
  std::string localPath = "fonts/" + fileName;
  if (FileExists(localPath))
    return localPath;

  // 3. Fall back to C:\Windows\Fonts
#ifdef _WIN32
  static std::string s_fontsDir;
  if (s_fontsDir.empty()) {
    char winDir[MAX_PATH];
    if (GetWindowsDirectoryA(winDir, MAX_PATH))
      s_fontsDir = std::string(winDir) + "\\Fonts\\";
  }

  if (!s_fontsDir.empty()) {
    std::string systemPath = s_fontsDir + fileName;
    if (FileExists(systemPath))
      return systemPath;
  }

  // 4. Try .ttc variant if .ttf wasn't found
  if (fileName.size() > 4 && fileName.substr(fileName.size() - 4) == ".ttf") {
    std::string ttcName = fileName.substr(0, fileName.size() - 4) + ".ttc";

    localPath = "fonts/" + ttcName;
    if (FileExists(localPath))
      return localPath;

    if (!s_fontsDir.empty()) {
      std::string systemPath = s_fontsDir + ttcName;
      if (FileExists(systemPath))
        return systemPath;
    }
  }
#endif

  TraceError("CFontManager::ResolveFontPath - Could not find font: %s",
             faceName);
  return "";
}

FT_Face CFontManager::CreateFace(const char *faceName) {
  if (!m_bInitialized) {
    if (!Initialize())
      return nullptr;
  }

  if (!faceName || !faceName[0])
    return nullptr;

  std::string lowerName = ToLower(faceName);

  // Check resolved path cache (avoids repeated disk stat calls)
  std::string path;
  auto cacheIt = m_resolvedPathCache.find(lowerName);
  if (cacheIt != m_resolvedPathCache.end()) {
    path = cacheIt->second;
  } else {
    path = ResolveFontPath(faceName);
    if (path.empty())
      return nullptr;
    m_resolvedPathCache[lowerName] = path;
  }

  // Create a new FT_Face — caller owns it
  FT_Face face = nullptr;
  FT_Error err = FT_New_Face(m_ftLibrary, path.c_str(), 0, &face);
  if (err != 0 || !face) {
    TraceError(
        "CFontManager::CreateFace - FT_New_Face failed for '%s' (error %d)",
        path.c_str(), err);
    return nullptr;
  }

  return face;
}
