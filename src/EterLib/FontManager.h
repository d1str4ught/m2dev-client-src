#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <unordered_map>

class CFontManager
{
public:
	static CFontManager& Instance();

	bool Initialize();
	void Destroy();

	// Get an FT_Face for a given face name. The face is owned by CFontManager.
	// Callers must NOT call FT_Done_Face on it.
	FT_Face GetFace(const char* faceName);

	FT_Library GetLibrary() const { return m_ftLibrary; }

private:
	CFontManager();
	~CFontManager();
	CFontManager(const CFontManager&) = delete;
	CFontManager& operator=(const CFontManager&) = delete;

	std::string ResolveFontPath(const char* faceName);

	FT_Library m_ftLibrary;
	bool m_bInitialized;

	// faceName (lowercase) -> file path
	std::unordered_map<std::string, std::string> m_fontPathMap;

	// filePath -> FT_Face (cached, shared across sizes)
	std::unordered_map<std::string, FT_Face> m_faceCache;
};
