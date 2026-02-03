#pragma once

class CGraphicDib
{
	public:
		CGraphicDib();
		virtual ~CGraphicDib();

		void Destroy();
		bool Create(int width, int height);

		int GetWidth();
		int GetHeight();

		void* GetPointer();

	protected:
		void Initialize();

	protected:
		int			m_width;
		int			m_height;

		void *		m_pvBuf;
};
