#include "Stdafx.h"
#include "PackLib/PackManager.h"

#include "terrain.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
// Texture Set
//////////////////////////////////////////////////////////////////////////
CTextureSet *CTerrainImpl::ms_pTextureSet = NULL;

void CTerrainImpl::SetTextureSet(CTextureSet *pTextureSet) {
  static CTextureSet s_EmptyTextureSet;

  if (!pTextureSet)
    ms_pTextureSet = &s_EmptyTextureSet;
  else
    ms_pTextureSet = pTextureSet;
}

CTextureSet *CTerrainImpl::GetTextureSet() {
  if (!ms_pTextureSet)
    SetTextureSet(NULL);

  return ms_pTextureSet;
}

CTerrainImpl::CTerrainImpl() { Initialize(); }

CTerrainImpl::~CTerrainImpl() { Clear(); }

void CTerrainImpl::Initialize() {
  memset(m_lWaterHeight, -1, sizeof(m_lWaterHeight));

  m_byNumWater = 0;
  memset(&m_awShadowMap, 0xFFFF, sizeof(m_awShadowMap));
  memset(&m_lpAlphaTexture, NULL, sizeof(m_lpAlphaTexture));

  m_lViewRadius = 0;

  m_wTileMapVersion = 8976;

  m_fHeightScale = 0.0f;

  m_lpShadowTexture = NULL;

  m_lSplatTilesX = 0;
  m_lSplatTilesY = 0;
}

void CTerrainImpl::Clear() {
  for (DWORD i = 0; i < GetTextureSet()->GetTextureCount(); ++i) {
    if (m_lpAlphaTexture[i]) {
      m_lpAlphaTexture[i]->Release();
      m_lpAlphaTexture[i] = NULL;
    }
  }

  Initialize();
}

bool CTerrainImpl::LoadHeightMap(const char *c_szFileName) {
  Tracef("LoadRawHeightMapFile %s ", c_szFileName);

  TPackFile kMappedFile;

  if (!CPackManager::Instance().GetFile(c_szFileName, kMappedFile)) {
    Tracen("Error");
    TraceError("CTerrainImpl::LoadHeightMap - %s OPEN ERROR", c_szFileName);
    return false;
  }

  memcpy(m_awRawHeightMap, kMappedFile.data(),
         sizeof(WORD) * HEIGHTMAP_RAW_XSIZE * HEIGHTMAP_RAW_YSIZE);

  return true;
}

bool CTerrainImpl::LoadAttrMap(const char *c_szFileName) {
  DWORD dwStart = ELTimer_GetMSec();
  Tracef("LoadAttrMapFile %s ", c_szFileName);

  TPackFile kMappedFile;

  if (!CPackManager::Instance().GetFile(c_szFileName, kMappedFile)) {
    TraceError("CTerrainImpl::LoadAttrMap - %s OPEN ERROR", c_szFileName);
    return false;
  }

  DWORD dwFileSize = kMappedFile.size();
  BYTE *abFileData = kMappedFile.data();

  // LoadAttrMap
  {
#pragma pack(push)
#pragma pack(1)
    struct SAttrMapHeader {
      WORD m_wMagic;
      WORD m_wWidth;
      WORD m_wHeight;
    };
#pragma pack(pop)

    if (dwFileSize < sizeof(SAttrMapHeader)) {
      TraceError(" CTerrainImpl::LoadAttrMap - %s FILE SIZE ERROR",
                 c_szFileName);
      return false;
    }

    SAttrMapHeader kAttrMapHeader;
    memcpy(&kAttrMapHeader, abFileData, sizeof(kAttrMapHeader));

    const WORD c_wAttrMapMagic = 2634;
    if (c_wAttrMapMagic != kAttrMapHeader.m_wMagic) {
      TraceError(
          "CTerrainImpl::LoadAttrMap - %s MAGIC NUMBER(%d!=MAGIC[%d]) ERROR",
          c_szFileName, kAttrMapHeader.m_wMagic, kAttrMapHeader.m_wMagic);
      return false;
    }

    if (ATTRMAP_XSIZE != kAttrMapHeader.m_wWidth) {
      TraceError("CTerrainImpl::LoadAttrMap - "
                 "kAttrMapHeader(%s).m_width(%d)!=ATTRMAP_XSIZE(%d)",
                 c_szFileName, kAttrMapHeader.m_wWidth, ATTRMAP_XSIZE);
      return false;
    }

    if (ATTRMAP_YSIZE != kAttrMapHeader.m_wHeight) {
      TraceError("CTerrainImpl::LoadAttrMap - "
                 "kAttrMapHeader(%s).m_height(%d)!=ATTRMAP_YSIZE(%d)",
                 c_szFileName, kAttrMapHeader.m_wHeight, ATTRMAP_YSIZE);
      return false;
    }

    DWORD dwFileRestSize = dwFileSize - sizeof(kAttrMapHeader);
    DWORD dwFileNeedSize = sizeof(m_abyAttrMap);
    if (dwFileRestSize != dwFileNeedSize) {
      TraceError("CTerrainImpl::LoadAttrMap - %s FILE DATA SIZE(rest %d != "
                 "need %d) ERROR",
                 c_szFileName, dwFileRestSize, dwFileNeedSize);
      return false;
    }

    BYTE *abSrcAttrData = abFileData + sizeof(kAttrMapHeader);
    memcpy(m_abyAttrMap, abSrcAttrData, sizeof(m_abyAttrMap));
  }

  Tracef("%d\n", ELTimer_GetMSec() - dwStart);
  return true;
}

bool CTerrainImpl::RAW_LoadTileMap(const char *c_szFileName) {
  Tracef("LoadSplatFile %s ", c_szFileName);

  TPackFile kMappedFile;

  if (!CPackManager::Instance().GetFile(c_szFileName, kMappedFile)) {
    Tracen("Error");
    TraceError("CTerrainImpl::RAW_LoadTileMap - %s OPEN ERROR", c_szFileName);
    return false;
  }

  memcpy(m_abyTileMap, kMappedFile.data(),
         sizeof(BYTE) * (TILEMAP_RAW_XSIZE) * (TILEMAP_RAW_YSIZE));

  return true;
}

bool CTerrainImpl::LoadWaterMap(const char *c_szFileName) {
  DWORD dwStart = ELTimer_GetMSec();

  if (!LoadWaterMapFile(c_szFileName)) {
    memset(m_abyWaterMap, 0xFF, sizeof(m_abyWaterMap));

    m_byNumWater = 0;
    memset(m_lWaterHeight, -1, sizeof(m_lWaterHeight));

    TraceError("CMapOutdoor::LoadWaterMap LoadWaterMapFile(%s) Failed",
               c_szFileName);

    return false;
  }

  Tracef("LoadWaterMapFile %s %d\n", c_szFileName, ELTimer_GetMSec() - dwStart);
  return true;
}

bool CTerrainImpl::LoadWaterMapFile(const char *c_szFileName) {
  TPackFile kMappedFile;

  if (!CPackManager::Instance().GetFile(c_szFileName, kMappedFile)) {
    Tracen("Error");
    TraceError("CTerrainImpl::LoadWaterMap - %s OPEN ERROR", c_szFileName);
    return false;
  }

  DWORD dwFileSize = kMappedFile.size();
  BYTE *abFileData = kMappedFile.data();

  {
#pragma pack(push)
#pragma pack(1)
    struct SWaterMapHeader {
      WORD m_wMagic;
      WORD m_wWidth;
      WORD m_wHeight;
      BYTE m_byLayerCount;
    };
#pragma pack(pop)

    if (dwFileSize < sizeof(SWaterMapHeader)) {
      TraceError("CTerrainImpl::LoadWaterMap - %s FILE SIZE ERROR",
                 c_szFileName);
      return false;
    }

    SWaterMapHeader kWaterMapHeader;
    memcpy(&kWaterMapHeader, abFileData, sizeof(kWaterMapHeader));

    const WORD c_wWaterMapMagic = 5426;

    if (c_wWaterMapMagic != kWaterMapHeader.m_wMagic) {
      TraceError(
          "CTerrainImpl::LoadWaterMap - %s MAGIC NUMBER(%d!=MAGIC[%d]) ERROR",
          c_szFileName, kWaterMapHeader.m_wMagic, c_wWaterMapMagic);
      return false;
    }

    if (WATERMAP_XSIZE != kWaterMapHeader.m_wWidth) {
      TraceError("CTerrainImpl::LoadWaterMap - "
                 "kWaterMapHeader(%s).m_width(%d)!=WATERMAP_XSIZE(%d)",
                 c_szFileName, kWaterMapHeader.m_wWidth, WATERMAP_XSIZE);
      return false;
    }

    if (WATERMAP_YSIZE != kWaterMapHeader.m_wHeight) {
      TraceError("CTerrainImpl::LoadWaterMap - "
                 "kWaterMapHeader(%s).m_height(%d)!=WATERMAP_YSIZE(%d)",
                 c_szFileName, kWaterMapHeader.m_wHeight, WATERMAP_YSIZE);
      return false;
    }

    m_byNumWater = kWaterMapHeader.m_byLayerCount;

    DWORD dwFileRestSize = dwFileSize - sizeof(kWaterMapHeader);
    DWORD dwFileNeedSize = sizeof(m_abyWaterMap) + sizeof(long) * m_byNumWater;
    DWORD dwFileNeedSize2 = sizeof(m_abyWaterMap) + sizeof(WORD) * m_byNumWater;
    if (dwFileRestSize == dwFileNeedSize2) {
      WORD wWaterHeight[MAX_WATER_NUM + 1];

      BYTE *abSrcWaterData = abFileData + sizeof(kWaterMapHeader);
      memcpy(m_abyWaterMap, abSrcWaterData, sizeof(m_abyWaterMap));

      BYTE *abSrcWaterHeight = abSrcWaterData + sizeof(m_abyWaterMap);

      m_byNumWater = MIN(MAX_WATER_NUM, m_byNumWater);
      if (m_byNumWater) {
        memcpy(wWaterHeight, abSrcWaterHeight, sizeof(WORD) * m_byNumWater);

        for (int i = 0; i < m_byNumWater; ++i)
          m_lWaterHeight[i] = wWaterHeight[i];
      }
    } else if (dwFileRestSize != dwFileNeedSize) {
      TraceError("CTerrainImpl::LoadWaterMap - %s FILE DATA SIZE(rest %d != "
                 "need %d) ERROR",
                 c_szFileName, dwFileRestSize, dwFileNeedSize);
      return false;
    }

    BYTE *abSrcWaterData = abFileData + sizeof(kWaterMapHeader);
    memcpy(m_abyWaterMap, abSrcWaterData, sizeof(m_abyWaterMap));

    BYTE *abSrcWaterHeight = abSrcWaterData + sizeof(m_abyWaterMap);

    if (m_byNumWater)
      memcpy(m_lWaterHeight, abSrcWaterHeight, sizeof(long) * m_byNumWater);
  }

  return true;
}

DWORD CTerrainImpl::GetShadowMapColor(float fx, float fy) {
  float fMapSize = (float)(TERRAIN_XSIZE);
  float fooMapSize = 1.0f / fMapSize;
  if (fx < 0 || fy < 0 || fx >= fMapSize || fy >= fMapSize)
    return 0xFFFFFFFF;

  fx = fx * fooMapSize * (float)(SHADOWMAP_XSIZE - 1);
  fy = fy * fooMapSize * (float)(SHADOWMAP_YSIZE - 1);
  int ix, iy;
  PR_FLOAT_TO_INT(fx, ix);
  PR_FLOAT_TO_INT(fy, iy);

  WORD w = *(m_awShadowMap + (iy * SHADOWMAP_XSIZE) + ix);

  int b = w & 0x1f;
  w >>= 5;
  b <<= 3;
  int g = w & 0x1f;
  w >>= 5;
  g <<= 3;
  int r = w & 0x1f;
  r <<= 3;

  return (DWORD)(0xff << 24) | (g << 16) | (g << 8) | r;
}
