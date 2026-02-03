#include "StdAfx.h"
#include "GrpDIB.h"

CGraphicDib::CGraphicDib()
{
	Initialize();
}

CGraphicDib::~CGraphicDib()
{
	Destroy();
}

void CGraphicDib::Initialize()
{
	m_pvBuf = NULL;
	m_width = 0;
	m_height = 0;
}

void CGraphicDib::Destroy()
{
	delete[] (DWORD*)m_pvBuf;
	Initialize();
}

bool CGraphicDib::Create(int width, int height)
{
	Destroy();

	m_width = width;
	m_height = height;

	m_pvBuf = new DWORD[width * height];
	memset(m_pvBuf, 0, width * height * sizeof(DWORD));

	return true;
}

void* CGraphicDib::GetPointer()
{
	return m_pvBuf;
}

int CGraphicDib::GetWidth()
{
	return m_width;
}

int CGraphicDib::GetHeight()
{
	return m_height;
}
