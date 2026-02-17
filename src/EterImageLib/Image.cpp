#include "StdAfx.h"

#include <assert.h>
#include "Image.h"

CImage::CImage(CImage &image) {
  Initialize();

  int w = image.GetWidth();
  int h = image.GetHeight();

  Create(w, h);

  DWORD *pdwDest = GetBasePointer();
  DWORD *pdwSrc = image.GetBasePointer();

  memcpy(pdwDest, pdwSrc, w * h * sizeof(DWORD));
}

void CImage::SetFileName(const char *c_szFileName) {
  m_stFileName = c_szFileName;
}

const std::string &CImage::GetFileNameString() { return m_stFileName; }

void CImage::PutImage(int x, int y, CImage *pImage) {
  assert(x >= 0 && x + pImage->GetWidth() <= GetWidth());
  assert(y >= 0 && y + pImage->GetHeight() <= GetHeight());

  int len = pImage->GetWidth() * sizeof(DWORD);

  for (int j = 0; j < pImage->GetHeight(); ++j) {
    DWORD *pdwDest = GetLinePointer(y + j) + x;
    memcpy(pdwDest, pImage->GetLinePointer(j), len);
  }
}

DWORD *CImage::GetBasePointer() {
  assert(m_pdwColors != NULL);
  return m_pdwColors;
}

DWORD *CImage::GetLinePointer(int line) {
  assert(m_pdwColors != NULL);
  return m_pdwColors + line * m_width;
}

int CImage::GetWidth() const {
  assert(m_pdwColors != NULL);
  return m_width;
}

int CImage::GetHeight() const {
  assert(m_pdwColors != NULL);
  return m_height;
}

void CImage::Clear(DWORD color) {
  assert(m_pdwColors != NULL);

  for (int y = 0; y < m_height; ++y) {
    DWORD *colorLine = &m_pdwColors[y * m_width];

    for (int x = 0; x < m_width; ++x)
      colorLine[x] = color;
  }
}

void CImage::Create(int width, int height) {
  Destroy();

  m_width = width;
  m_height = height;
  m_pdwColors = new DWORD[m_width * m_height];
}

void CImage::Destroy() {
  if (m_pdwColors) {
    delete[] m_pdwColors;
    m_pdwColors = NULL;
  }
}

void CImage::Initialize() {
  m_pdwColors = NULL;
  m_width = 0;
  m_height = 0;
}

bool CImage::IsEmpty() const { return (m_pdwColors == NULL) ? true : false; }

void CImage::FlipTopToBottom() {
  DWORD *swap = new DWORD[m_width * m_height];

  int row;
  UINT width = GetWidth();
  UINT height = GetHeight();
  DWORD *end_row;
  DWORD *start_row;

  for (row = 0; row < GetHeight() / 2; row++) {
    end_row = &(m_pdwColors[width * (height - row - 1)]);
    start_row = &(m_pdwColors[width * row]);

    memcpy(swap, end_row, width * sizeof(DWORD));
    memcpy(end_row, start_row, width * sizeof(DWORD));
    memcpy(start_row, swap, width * sizeof(DWORD));
  }

  delete[] swap;
}

CImage::CImage() { Initialize(); }

CImage::~CImage() { Destroy(); }
