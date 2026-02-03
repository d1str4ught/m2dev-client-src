#pragma once

#include "DibBar.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class CTextBar : public CDibBar
{
	public:
		CTextBar(int fontSize, bool isBold);
		virtual ~CTextBar();

		void TextOut(int ix, int iy, const char * c_szText);
		void SetTextColor(int r, int g, int b);
		void GetTextExtent(const char * c_szText, SIZE* p_size);

	protected:
		void __SetFont(int fontSize, bool isBold);
		void __ApplyFaceState();

		void OnCreate();

	protected:
		FT_Face m_ftFace;
		DWORD m_textColor;

		int		m_fontSize;
		bool	m_isBold;

		int		m_ascender;
		int		m_lineHeight;
};
