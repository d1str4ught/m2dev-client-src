#pragma once

#include "GrpDib.h"

class CBlockTexture;

class CDibBar
{
	public:
		CDibBar();
		virtual ~CDibBar();

		bool Create(DWORD dwWidth, DWORD dwHeight);
		void Invalidate();
		void SetClipRect(const RECT & c_rRect);
		void ClearBar();
		void Render(int ix, int iy);

	protected:
		DWORD __NearTextureSize(DWORD dwSize);
		void __DivideTextureSize(DWORD dwSize, DWORD dwMax, DWORD * pdwxStep, DWORD * pdwxCount, DWORD * pdwxRest);
		CBlockTexture * __BuildTextureBlock(DWORD dwxPos, DWORD dwyPos, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwTextureWidth, DWORD dwTextureHeight);
		void __BuildTextureBlockList(DWORD dwWidth, DWORD dwHeight, DWORD dwMax=256);

		virtual void OnCreate(){}

	protected:
		CGraphicDib m_dib;
		std::vector<CBlockTexture *> m_kVec_pkBlockTexture;

		DWORD m_dwWidth;
		DWORD m_dwHeight;
};
