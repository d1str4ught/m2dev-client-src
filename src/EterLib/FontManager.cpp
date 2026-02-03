#include "StdAfx.h"
#include "FontManager.h"

#include <algorithm>
#include <cctype>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

static std::string ToLower(const std::string& s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return result;
}

static bool FileExists(const std::string& path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0;
}

CFontManager::CFontManager()
	: m_ftLibrary(nullptr)
	, m_bInitialized(false)
{
}

CFontManager::~CFontManager()
{
	Destroy();
}

CFontManager& CFontManager::Instance()
{
	static CFontManager instance;
	return instance;
}

bool CFontManager::Initialize()
{
	if (m_bInitialized)
		return true;

	if (FT_Init_FreeType(&m_ftLibrary) != 0)
	{
		TraceError("CFontManager::Initialize - FT_Init_FreeType failed");
		return false;
	}

	m_bInitialized = true;

	// Register default font name -> file mappings
	// Korean system fonts
	m_fontPathMap["gulim"] = "gulim.ttc";
	m_fontPathMap["\xea\xb5\xb4\xeb\xa6\xbc"] = "gulim.ttc";             // 굴림 (UTF-8)
	m_fontPathMap["\xea\xb5\xb4\xeb\xa6\xbc\xec\xb2\xb4"] = "gulimche.ttc"; // 굴림체

	// Common Latin fonts
	m_fontPathMap["arial"] = "arial.ttf";
	m_fontPathMap["tahoma"] = "tahoma.ttf";
	m_fontPathMap["verdana"] = "verdana.ttf";
	m_fontPathMap["times new roman"] = "times.ttf";
	m_fontPathMap["courier new"] = "cour.ttf";
	m_fontPathMap["segoe ui"] = "segoeui.ttf";

	return true;
}

void CFontManager::Destroy()
{
	for (auto& pair : m_faceCache)
	{
		if (pair.second)
			FT_Done_Face(pair.second);
	}
	m_faceCache.clear();
	m_fontPathMap.clear();

	if (m_ftLibrary)
	{
		FT_Done_FreeType(m_ftLibrary);
		m_ftLibrary = nullptr;
	}

	m_bInitialized = false;
}

std::string CFontManager::ResolveFontPath(const char* faceName)
{
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
	char winDir[MAX_PATH];
	if (GetWindowsDirectoryA(winDir, MAX_PATH))
	{
		std::string systemPath = std::string(winDir) + "\\Fonts\\" + fileName;
		if (FileExists(systemPath))
			return systemPath;
	}

	// 4. Try .ttc variant if .ttf wasn't found
	if (fileName.size() > 4 && fileName.substr(fileName.size() - 4) == ".ttf")
	{
		std::string ttcName = fileName.substr(0, fileName.size() - 4) + ".ttc";

		localPath = "fonts/" + ttcName;
		if (FileExists(localPath))
			return localPath;

		if (GetWindowsDirectoryA(winDir, MAX_PATH))
		{
			std::string systemPath = std::string(winDir) + "\\Fonts\\" + ttcName;
			if (FileExists(systemPath))
				return systemPath;
		}
	}
#endif

	TraceError("CFontManager::ResolveFontPath - Could not find font: %s", faceName);
	return "";
}

FT_Face CFontManager::GetFace(const char* faceName)
{
	if (!m_bInitialized)
	{
		if (!Initialize())
			return nullptr;
	}

	std::string path = ResolveFontPath(faceName);
	if (path.empty())
		return nullptr;

	// Check cache
	auto it = m_faceCache.find(path);
	if (it != m_faceCache.end())
		return it->second;

	// Load new face
	FT_Face face = nullptr;
	FT_Error err = FT_New_Face(m_ftLibrary, path.c_str(), 0, &face);
	if (err != 0 || !face)
	{
		TraceError("CFontManager::GetFace - FT_New_Face failed for '%s' (error %d)", path.c_str(), err);
		return nullptr;
	}

	m_faceCache[path] = face;
	return face;
}
