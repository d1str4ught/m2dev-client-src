#pragma once

#include <d3dx9.h>

#include "Eterlib/GrpScreen.h"
#include "Eterlib/Resource.h"
#include "Eterlib/GrpImageInstance.h"
#include "EterLib/TextFileLoader.h"

#include "Type.h"
#include "EffectElementBase.h"

class CEffectMesh : public CResource {
public:
  typedef struct SEffectFrameData {
    BYTE byChangedFrame;
    float fVisibility;
    DWORD dwVertexCount;
    DWORD dwTextureVertexCount;
    DWORD dwIndexCount;
    std::vector<TPTVertex> PDTVertexVector;
  } TEffectFrameData;

  typedef struct SEffectMeshData {
    char szObjectName[32];
    char szDiffuseMapFileName[128];

    std::vector<TEffectFrameData> EffectFrameDataVector;
    std::vector<CGraphicImage *> pImageVector;

    static SEffectMeshData *New();
    static void Delete(SEffectMeshData *pkData);

    static void DestroySystem();

    static CDynamicPool<SEffectMeshData> ms_kPool;
  } TEffectMeshData;

  // About Resource Code
public:
  typedef CRef<CEffectMesh> TRef;

public:
  static TType Type();

public:
  CEffectMesh(const char *c_szFileName);
  virtual ~CEffectMesh();

  DWORD GetFrameCount();
  DWORD GetMeshCount();
  TEffectMeshData *GetMeshDataPointer(DWORD dwMeshIndex);

  std::vector<CGraphicImage *> *GetTextureVectorPointer(DWORD dwMeshIndex);
  std::vector<CGraphicImage *> &GetTextureVectorReference(DWORD dwMeshIndex);

  // Exceptional function for tool
  BOOL GetMeshElementPointer(DWORD dwMeshIndex, TEffectMeshData **ppMeshData);

protected:
  bool OnLoad(int iSize, const void *c_pvBuf);

  void OnClear();
  bool OnIsEmpty() const;
  bool OnIsType(TType type);

  BOOL __LoadData_Ver001(int iSize, const BYTE *c_pbBuf);
  BOOL __LoadData_Ver002(int iSize, const BYTE *c_pbBuf);

protected:
  int m_iGeomCount;
  int m_iFrameCount;
  std::vector<TEffectMeshData *> m_pEffectMeshDataVector;

  bool m_isData;
};

class CEffectMeshScript : public CEffectElementBase {
public:
  typedef struct SMeshData {
    BYTE byBillboardType;

    BOOL bBlendingEnable;
    BYTE byBlendingSrcType;
    BYTE byBlendingDestType;
    BOOL bTextureAlphaEnable;

    BYTE byColorOperationType;
    D3DXCOLOR ColorFactor;

    BOOL bTextureAnimationLoopEnable;
    float fTextureAnimationFrameDelay;

    DWORD dwTextureAnimationStartFrame;

    TTimeEventTableFloat TimeEventAlpha;

    SMeshData() { TimeEventAlpha.clear(); }
  } TMeshData;
  typedef std::vector<TMeshData> TMeshDataVector;

public:
  CEffectMeshScript();
  virtual ~CEffectMeshScript();

  const char *GetMeshFileName();

  void ReserveMeshData(DWORD dwMeshCount);
  bool CheckMeshIndex(DWORD dwMeshIndex);
  bool GetMeshDataPointer(DWORD dwMeshIndex, TMeshData **ppMeshData);
  int GetMeshDataCount();

  int GetBillboardType(DWORD dwMeshIndex);
  BOOL isBlendingEnable(DWORD dwMeshIndex);
  BYTE GetBlendingSrcType(DWORD dwMeshIndex);
  BYTE GetBlendingDestType(DWORD dwMeshIndex);
  BOOL isTextureAlphaEnable(DWORD dwMeshIndex);
  BOOL GetColorOperationType(DWORD dwMeshIndex, BYTE *pbyType);
  BOOL GetColorFactor(DWORD dwMeshIndex, D3DXCOLOR *pColor);
  BOOL GetTimeTableAlphaPointer(DWORD dwMeshIndex,
                                TTimeEventTableFloat **pTimeEventAlpha);

  BOOL isMeshAnimationLoop();
  BOOL GetMeshAnimationLoopCount();
  float GetMeshAnimationFrameDelay();
  BOOL isTextureAnimationLoop(DWORD dwMeshIndex);
  float GetTextureAnimationFrameDelay(DWORD dwMeshIndex);
  DWORD GetTextureAnimationStartFrame(DWORD dwMeshIndex);

protected:
  void OnClear();
  bool OnIsData();
  BOOL OnLoadScript(CTextFileLoader &rTextFileLoader);

protected:
  BOOL m_isMeshAnimationLoop;
  int m_iMeshAnimationLoopCount;
  float m_fMeshAnimationFrameDelay;
  TMeshDataVector m_MeshDataVector;

  std::string m_strMeshFileName;

public:
  static void DestroySystem();

  static CEffectMeshScript *New();
  static void Delete(CEffectMeshScript *pkData);

  static CDynamicPool<CEffectMeshScript> ms_kPool;
};