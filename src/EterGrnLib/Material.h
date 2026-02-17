#pragma once

#include <granny.h>
#include <windows.h>
#include <d3d9.h>

#include "Eterlib/ReferenceObject.h"
#include "Eterlib/Ref.h"
#include "Eterlib/GrpImageInstance.h"
#include "Util.h"

class CGrannyMaterial : public CReferenceObject {
public:
  typedef CRef<CGrannyMaterial> TRef;

  static void CreateSphereMap(UINT uMapIndex,
                              const char *c_szSphereMapImageFileName);
  static void DestroySphereMap();

public:
  enum EType { TYPE_DIFFUSE_PNT, TYPE_BLEND_PNT, TYPE_MAX_NUM };

public:
  static void TranslateSpecularMatrix(float fAddX, float fAddY, float fAddZ);

private:
  static D3DXMATRIX ms_matSpecular;
  static D3DXVECTOR3 ms_v3SpecularTrans;

public:
  CGrannyMaterial();
  virtual ~CGrannyMaterial();

  void Destroy();
  void Copy(CGrannyMaterial &rkMtrl);
  bool IsEqual(granny_material *pgrnMaterial) const;
  bool IsIn(const char *c_szImageName, int *iStage);
  void SetSpecularInfo(BOOL bFlag, float fPower, BYTE uSphereMapIndex);

  void ApplyRenderState();
  void RestoreRenderState();

protected:
  void Initialize();

public:
  bool CreateFromGrannyMaterialPointer(granny_material *pgrnMaterial);
  void SetImagePointer(int iStage, CGraphicImage *pImage);

  CGrannyMaterial::EType GetType() const;
  CGraphicImage *GetImagePointer(int iStage) const;

  const CGraphicTexture *GetDiffuseTexture() const;
  const CGraphicTexture *GetOpacityTexture() const;

  LPDIRECT3DTEXTURE9 GetD3DTexture(int iStage) const;

  // MR-12: Fix specular isolation issue
  float GetSpecularPower() const;
  bool IsSpecularEnabled() const { return m_bSpecularEnable; }
  BYTE GetSphereMapIndex() const { return m_bSphereMapIndex; }
  // MR-12: -- END OF -- Fix specular isolation issue

  bool IsTwoSided() const { return m_bTwoSideRender; }

protected:
  CGraphicImage *__GetImagePointer(const char *c_szFileName);

  BOOL __IsSpecularEnable() const;

  void __ApplyDiffuseRenderState();
  void __RestoreDiffuseRenderState();
  void __ApplySpecularRenderState();
  void __RestoreSpecularRenderState();

protected:
  granny_material *m_pgrnMaterial;
  CGraphicImage::TRef m_roImage[2];
  EType m_eType;

  float m_fSpecularPower;
  BOOL m_bSpecularEnable;
  bool m_bTwoSideRender;
  DWORD m_dwLastCullRenderStateForTwoSideRendering;
  BYTE m_bSphereMapIndex;

  void (CGrannyMaterial::*m_pfnApplyRenderState)();
  void (CGrannyMaterial::*m_pfnRestoreRenderState)();

private:
  enum {
    SPHEREMAP_NUM = 10,
  };
  static CGraphicImageInstance ms_akSphereMapInstance[SPHEREMAP_NUM];
};

class CGrannyMaterialPalette {
public:
  CGrannyMaterialPalette();
  virtual ~CGrannyMaterialPalette();

  void Clear();
  void Copy(const CGrannyMaterialPalette &rkMtrlPalSrc);

  DWORD RegisterMaterial(granny_material *pgrnMaterial);
  void SetMaterialImagePointer(const char *c_szMtrlName, CGraphicImage *pImage);
  void SetMaterialData(const char *c_szMtrlName,
                       const SMaterialData &c_rkMaterialData);
  void SetSpecularInfo(const char *c_szMtrlName, BOOL bEnable, float fPower);

  CGrannyMaterial &GetMaterialRef(DWORD mtrlIndex);

  DWORD GetMaterialCount() const;

protected:
  std::vector<CGrannyMaterial::TRef> m_mtrlVector;
};