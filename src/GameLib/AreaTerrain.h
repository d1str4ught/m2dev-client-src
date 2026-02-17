#pragma once

class CMapOutdoor;

#include "PRTerrainLib/Terrain.h"
#include "TerrainPatch.h"

class CTerrain : public CTerrainImpl, public CGraphicBase {
public:
  enum EBoundaryLoadPart {
    LOAD_INVALID,
    LOAD_NOBOUNDARY,
    LOAD_TOPLEFT,
    LOAD_TOP,
    LOAD_TOPRIGHT,
    LOAD_LEFT,
    LOAD_RIGHT,
    LOAD_BOTTOMLEFT,
    LOAD_BOTTOM,
    LOAD_BOTTOMRIGHT,
    LOAD_ALLBOUNDARY,
  };

  CTerrain();
  virtual ~CTerrain();

  virtual void Clear();

  void SetMapOutDoor(CMapOutdoor *pOwnerOutdoorMap);

  //////////////////////////////////////////////////////////////////////////
  // Loading
  bool RAW_LoadTileMap(const char *c_pszFileName, bool bBGLoading = false);

  bool LoadHeightMap(const char *c_pszFileName);

  void CalculateTerrainPatch();
  //////////////////////////////////////////////////////////////////////////

  void CopySettingFromGlobalSetting();

  WORD WE_GetHeightMapValue(short sX, short sY);

  bool IsReady() { return m_bReady; }
  void SetReady(bool bReady = true) { m_bReady = bReady; }

  // Height Map
  WORD *GetHeightMap() { return m_awRawHeightMap; }
  float GetHeight(int x, int y);

  // Normal Map
  bool GetNormal(int ix, int iy, D3DXVECTOR3 *pv3Normal);

  // TileMap
  BYTE *RAW_GetTileMap() { return m_abyTileMap; }
  char *GetNormalMap() { return m_acNormalMap; }

  // Attr
  bool LoadAttrMap(const char *c_pszFileName);
  BYTE *GetAttrMap() { return m_abyAttrMap; }
  BYTE GetAttr(WORD wCoordX, WORD wCoordY);
  bool isAttrOn(WORD wCoordX, WORD wCoordY, BYTE byAttrFlag);

  //////////////////////////////////////////////////////////////////////////
  // Water
  BYTE *GetWaterMap() { return m_abyWaterMap; }
  void GetWaterHeight(BYTE byWaterNum, long *plWaterHeight);
  bool GetWaterHeight(WORD wCoordX, WORD wCoordY, long *plWaterHeight);

  // Shadow Texture
  void LoadShadowTexture(const char *c_pszFileName);

  // Shadow Map
  bool LoadShadowMap(const char *c_pszFileName);

  // MiniMap
  void LoadMiniMapTexture(const char *c_pszFileName);
  inline LPDIRECT3DTEXTURE9 GetMiniMapTexture() { return m_lpMiniMapTexture; }

  // Marked Area
  BOOL IsMarked() { return m_bMarked; }
  void AllocateMarkedSplats(BYTE *pbyAlphaMap);
  void DeallocateMarkedSplats();
  TTerrainSplatPatch &GetMarkedSplatPatch() { return m_MarkedSplatPatch; }

  // Coordinate
  void GetCoordinate(WORD *usCoordX, WORD *usCoordY) {
    *usCoordX = m_wX;
    *usCoordY = m_wY;
  }

  void SetCoordinate(WORD wCoordX, WORD wCoordY);

  std::string &GetName() { return m_strName; }
  void SetName(const std::string c_strName) { m_strName = c_strName; }

  CMapOutdoor *GetOwner() { return m_pOwnerOutdoorMap; }
  void RAW_GenerateSplat(bool bBGLoading = false);

protected:
  bool Initialize();
  void RAW_AllocateSplats(bool bBGLoading = false);
  void RAW_DeallocateSplats(bool bBGLoading = false);
  virtual void RAW_CountTiles();

  LPDIRECT3DTEXTURE9 AddTexture32(BYTE byImageNum, BYTE *pbyImage,
                                  long lTextureWidth, long lTextureHeight);
  void PutImage32(BYTE *pbySrc, BYTE *pbyDst, long src_pitch, long dst_pitch,
                  long lTextureWidth, long lTextureHeight,
                  bool bResize = false);
  void PutImage16(BYTE *pbySrc, BYTE *pbyDst, long src_pitch, long dst_pitch,
                  long lTextureWidth, long lTextureHeight,
                  bool bResize = false);

protected:
  void CalculateNormal(long x, long y);

protected:
  std::string m_strName;
  WORD m_wX;
  WORD m_wY;

protected:
  bool m_bReady;

  CGraphicImageInstance m_ShadowGraphicImageInstance;

  // MiniMap
  CGraphicImageInstance m_MiniMapGraphicImageInstance;
  LPDIRECT3DTEXTURE9 m_lpMiniMapTexture;

  // Owner COutdoorMap poineter
  CMapOutdoor *m_pOwnerOutdoorMap;

  // Picking
  D3DXVECTOR3 m_v3Pick;

  DWORD m_dwNumTexturesShow;
  std::vector<DWORD> m_VectorNumShowTexture;

  CTerrainPatch m_TerrainPatchList[PATCH_XCOUNT * PATCH_YCOUNT];

  BOOL m_bMarked;
  TTerrainSplatPatch m_MarkedSplatPatch;
  LPDIRECT3DTEXTURE9 m_lpMarkedTexture;

public:
  CTerrainPatch *GetTerrainPatchPtr(BYTE byPatchNumX, BYTE byPatchNumY);

protected:
  void _CalculateTerrainPatch(BYTE byPatchNumX, BYTE byPatchNumY);

public:
  static void DestroySystem();

  static CTerrain *New();
  static void Delete(CTerrain *pkTerrain);

  static CDynamicPool<CTerrain> ms_kPool;
};
