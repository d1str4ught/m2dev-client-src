// SkyBox.h: interface for the CSkyBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKYBOX_H__AB5049E1_8F1C_4C35_9406_45EC7EF4AD1B__INCLUDED_)
#define AFX_SKYBOX_H__AB5049E1_8F1C_4C35_9406_45EC7EF4AD1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GrpBase.h"
#include "GrpScreen.h"
#include "GrpImageInstance.h"
#include "ColorTransitionHelper.h"

#include <map>
#include <string>

typedef struct SColor {
  SColor(float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 0.0f)
      : r(_r), g(_g), b(_b), a(_a) {}
  float r, g, b, a;
} TColor;

typedef struct {
  TColor m_FirstColor;
  TColor m_SecondColor;
} TGradientColor;

typedef std::vector<TGradientColor> TVectorGradientColor;
typedef TVectorGradientColor::iterator TVectorGradientIterator;

class CSkyObjectQuad {
public:
  CSkyObjectQuad();
  virtual ~CSkyObjectQuad();

  void Clear(const unsigned char &c_rucNumVertex, const float &c_rfRed,
             const float &c_rfGreen, const float &c_rfBlue,
             const float &c_rfAlpha);
  void SetSrcColor(const unsigned char &c_rucNumVertex, const float &c_rfRed,
                   const float &c_rfGreen, const float &c_rfBlue,
                   const float &c_rfAlpha);
  void SetTransition(const unsigned char &c_rucNumVertex, const float &c_rfRed,
                     const float &c_rfGreen, const float &c_rfBlue,
                     const float &c_rfAlpha, DWORD dwDuration);

  void SetVertex(const unsigned char &c_rucNumVertex,
                 const TPDTVertex &c_rPDTVertex);

  void StartTransition();
  bool Update();
  void Render();

private:
  TPDTVertex m_Vertex[4];
  TIndex m_Indices[4]; // 인덱스 버퍼...
  CColorTransitionHelper m_Helper[4];
};

class CSkyObject : public CScreen {
public:
  enum {
    SKY_RENDER_MODE_DEFAULT, // = SKY_RENDER_MODE_TEXTURE
    SKY_RENDER_MODE_DIFFUSE,
    SKY_RENDER_MODE_TEXTURE,
    SKY_RENDER_MODE_MODULATE,
    SKY_RENDER_MODE_MODULATE2X,
    SKY_RENDER_MODE_MODULATE4X,
  };

  CSkyObject();
  virtual ~CSkyObject();

  virtual void Destroy() = 0;

  virtual void Render() = 0;
  virtual void Update() = 0;
  virtual void StartTransition();

  void SetRenderMode(unsigned char ucRenderMode) {
    m_ucRenderMode = ucRenderMode;
  }

  const bool &isTransitionStarted() { return m_bTransitionStarted; }

protected:
  CGraphicImageInstance *GenerateTexture(const char *szfilename);
  void DeleteTexture(CGraphicImageInstance *pGraphicImageInstance);

protected:
  //////////////////////////////////////////////////////////////////////////
  // 타입 정의

  typedef std::vector<CSkyObjectQuad> TSkyObjectQuadVector;
  typedef TSkyObjectQuadVector::iterator TSkyObjectQuadIterator;

  typedef struct CSkyBox {
    void StartTransition();
    bool Update();
    void Render();
    std::string m_strfacename;
    std::string m_strFaceTextureFileName;
    TSkyObjectQuadVector m_SkyObjectQuadVector;
  } TSkyObjectFace;

  typedef std::map<std::string, CGraphicImageInstance *>
      TGraphicImageInstanceMap;

  //////////////////////////////////////////////////////////////////////////

  // 구름...
  TSkyObjectFace m_FaceCloud; // 구름 일단 한장...
  D3DXMATRIX m_matWorldCloud, m_matTranslationCloud, m_matTextureCloud;
  D3DXVECTOR3 m_v3PositionCloud;
  float m_fCloudScaleX, m_fCloudScaleY, m_fCloudHeight;
  float m_fCloudTextureScaleX, m_fCloudTextureScaleY;
  float m_fCloudScrollSpeedU, m_fCloudScrollSpeedV;
  float m_fCloudPositionU, m_fCloudPositionV;

  DWORD m_dwlastTime;

  // 스카이 박스 이미지...
  TGraphicImageInstanceMap m_GraphicImageInstanceMap;

  // Transform...
  D3DXMATRIX m_matWorld, m_matTranslation;
  D3DXVECTOR3 m_v3Position;
  float m_fScaleX, m_fScaleY, m_fScaleZ;

  // 랜더링 관련... 임시 변수..
  unsigned char m_ucRenderMode;

  std::string m_strCurTime;
  bool m_bTransitionStarted;
  bool m_bSkyMatrixUpdated;

  CGraphicImageInstance m_CloudAlphaImageInstance;
};

class CSkyBox : public CSkyObject {
public:
  CSkyBox();
  virtual ~CSkyBox();

  void Update();
  void Render();
  void RenderCloud();

  void Destroy();
  void Unload();

  void SetSkyBoxScale(const D3DXVECTOR3 &c_rv3Scale);
  void SetGradientLevel(BYTE byUpper, BYTE byLower);
  void SetFaceTexture(const char *c_szFileName, int iFaceIndex);
  void SetCloudTexture(const char *c_szFileName);
  void SetCloudScale(const D3DXVECTOR2 &c_rv2CloudScale);
  void SetCloudHeight(float fHeight);
  void SetCloudTextureScale(const D3DXVECTOR2 &c_rv2CloudTextureScale);
  void SetCloudScrollSpeed(const D3DXVECTOR2 &c_rv2CloudScrollSpeed);
  void SetCloudColor(const TGradientColor &c_rColor,
                     const TGradientColor &c_rNextColor,
                     const DWORD &dwTransitionTime);
  void Refresh();
  void SetSkyColor(const TVectorGradientColor &c_rColorVector,
                   const TVectorGradientColor &c_rNextColorVector,
                   long lTransitionTime);
  void StartTransition();

protected:
  void SetSkyObjectQuadVertical(TSkyObjectQuadVector *pSkyObjectQuadVector,
                                const D3DXVECTOR2 *c_pv2QuadPoints);
  void SetSkyObjectQuadHorizon(TSkyObjectQuadVector *pSkyObjectQuadVector,
                               const D3DXVECTOR3 *c_pv3QuadPoints);

  // void UpdateSkyFaceQuadTransform(D3DXVECTOR3 * c_pv3QuadPoints);

protected:
  unsigned char m_ucVirticalGradientLevelUpper;
  unsigned char m_ucVirticalGradientLevelLower;
  TSkyObjectFace m_Faces[6];
};

#endif // !defined(AFX_SKYBOX_H__AB5049E1_8F1C_4C35_9406_45EC7EF4AD1B__INCLUDED_)
