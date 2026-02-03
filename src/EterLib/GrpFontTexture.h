#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <map>

class CGraphicFontTexture : public CGraphicTexture
{
	public:
		typedef wchar_t TCharacterKey;

		typedef struct SCharacterInfomation
		{
			short index;
			short width;
			short height;
			float left;
			float top;
			float right;
			float bottom;
			float advance;
		} TCharacterInfomation;

		typedef std::vector<TCharacterInfomation*> TPCharacterInfomationVector;

	public:
		CGraphicFontTexture();
		virtual ~CGraphicFontTexture();

		void Destroy();
		bool Create(const char* c_szFontName, int fontSize, bool bItalic);

		bool CreateDeviceObjects();
		void DestroyDeviceObjects();

		bool CheckTextureIndex(DWORD dwTexture);
		void SelectTexture(DWORD dwTexture);

		bool UpdateTexture();

		TCharacterInfomation* GetCharacterInfomation(wchar_t keyValue);
		TCharacterInfomation* UpdateCharacterInfomation(TCharacterKey keyValue);

		bool IsEmpty() const;

	protected:
		void Initialize();

		bool AppendTexture();

	protected:
		typedef std::vector<CGraphicImageTexture*> TGraphicImageTexturePointerVector;
		typedef std::map<TCharacterKey, TCharacterInfomation> TCharacterInfomationMap;

	protected:
		FT_Face m_ftFace;

		// CPU-side atlas buffer (replaces CGraphicDib)
		DWORD* m_pAtlasBuffer;
		int m_atlasWidth;
		int m_atlasHeight;

		TGraphicImageTexturePointerVector m_pFontTextureVector;

		TCharacterInfomationMap m_charInfoMap;

		int m_x;
		int m_y;
		int m_step;
		bool m_isDirty;

		TCHAR m_fontName[LF_FACESIZE];
		LONG m_fontSize;
		bool m_bItalic;

		// FreeType metrics cached per-font
		int m_ascender;
		int m_lineHeight;
};
