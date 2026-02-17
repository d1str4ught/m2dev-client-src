#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <unordered_map>

class CFontManager {
public:
  static CFontManager &Instance();

  bool Initialize();
  void Destroy();

  // Create a NEW FT_Face for the given font name.
  // The caller OWNS the returned face and must call FT_Done_Face on it when
  // done.
  FT_Face CreateFace(const char *faceName);

  FT_Library GetLibrary() const { return m_ftLibrary; }

private:
  CFontManager();
  ~CFontManager();
  CFontManager(const CFontManager &) = delete;
  CFontManager &operator=(const CFontManager &) = delete;

  std::string ResolveFontPath(const char *faceName);

  FT_Library m_ftLibrary;
  bool m_bInitialized;

  // faceName (lowercase) -> file path
  std::unordered_map<std::string, std::string> m_fontPathMap;

  // faceName (lowercase) -> resolved file system path (caches disk lookups)
  std::unordered_map<std::string, std::string> m_resolvedPathCache;
};
