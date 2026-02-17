#include "StdAfx.h"
#include "MapUtil.h"

void Environment_Init(SEnvironmentData &envData) {
  for (int i = 0; i < ENV_DIRLIGHT_NUM; ++i) {
    envData.bDirLightsEnable[i] = false;
    envData.DirLights[i].Type = D3DLIGHT_DIRECTIONAL;
    envData.DirLights[i].Direction = D3DXVECTOR3(0.5f, 0.5f, -0.5f);
    envData.DirLights[i].Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    envData.DirLights[i].Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    envData.DirLights[i].Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    envData.DirLights[i].Ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
    envData.DirLights[i].Range = 0.0f;   // Used by Point Light & Spot Light
    envData.DirLights[i].Falloff = 1.0f; // Used by Spot Light
    envData.DirLights[i].Theta = 0.0f;   // Used by Spot Light
    envData.DirLights[i].Phi = 0.0f;     // Used by Spot Light
    envData.DirLights[i].Attenuation0 = 0.0f;
    envData.DirLights[i].Attenuation1 = 1.0f;
    envData.DirLights[i].Attenuation2 = 0.0f;
  }

  envData.Material.Diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
  envData.Material.Ambient = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
  envData.Material.Emissive = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
  envData.Material.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
  envData.Material.Power = 0.0f;

  // MR-14: Fog update by Alaric
  envData.bFogEnable = TRUE;
  envData.bDensityFog = TRUE;
  // MR-14: -- END OF -- Fog update by Alaric
  envData.m_fFogNearDistance = 25600.0f * 0.5f;
  envData.m_fFogFarDistance = 25600.0f * 0.7f;
  envData.FogColor = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);

  envData.bFilteringEnable = FALSE;
  envData.FilteringColor = D3DXCOLOR(0.3f, 0.1f, 0.1f, 0.0f);
  envData.byFilteringAlphaSrc = D3DBLEND_ONE;
  envData.byFilteringAlphaDest = D3DBLEND_ONE;

  envData.fWindStrength = 0.2f;
  envData.fWindRandom = 0.0f;

  envData.v3SkyBoxScale = D3DXVECTOR3(3500.0f, 3500.0f, 3500.0f);
  envData.bySkyBoxGradientLevelUpper = 0;
  envData.bySkyBoxGradientLevelLower = 0;
  envData.bSkyBoxTextureRenderMode = FALSE;

  envData.v2CloudScale = D3DXVECTOR2(200000.0f, 200000.0f);
  envData.fCloudHeight = 30000.0f;
  envData.v2CloudTextureScale = D3DXVECTOR2(4.0f, 4.0f);
  envData.v2CloudSpeed = D3DXVECTOR2(0.001f, 0.001f);
  envData.strCloudTextureFileName = "";
  envData.CloudGradientColor.m_FirstColor = .0f;
  envData.CloudGradientColor.m_SecondColor = .0f;

  envData.SkyBoxGradientColorVector.clear();

  envData.bLensFlareEnable = FALSE;
  envData.LensFlareBrightnessColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
  envData.fLensFlareMaxBrightness = 1.0f;

  envData.bMainFlareEnable = FALSE;
  envData.strMainFlareTextureFileName = "";
  envData.fMainFlareSize = 0.2f;

  envData.bReserve = FALSE;
}

bool Environment_Load(SEnvironmentData &envData, const char *envFileName) {
  CTextFileLoader textLoader;

  if (!textLoader.Load(envFileName))
    return false;

  textLoader.SetTop();

  textLoader.GetTokenBoolean("reserved", &envData.bReserve);

  if (textLoader.SetChildNode("directionallight")) {
    D3DVECTOR v3Dir;
    textLoader.GetTokenDirection("direction", &v3Dir);

    if (textLoader.SetChildNode("background")) {
      envData.DirLights[ENV_DIRLIGHT_BACKGROUND].Direction = v3Dir;
      textLoader.GetTokenBoolean(
          "enable", &envData.bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND]);
      textLoader.GetTokenColor(
          "diffuse", &envData.DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse);
      textLoader.GetTokenColor(
          "ambient", &envData.DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient);
      textLoader.SetParentNode();
    }

    if (textLoader.SetChildNode("character")) {
      envData.DirLights[ENV_DIRLIGHT_CHARACTER].Direction = v3Dir;
      textLoader.GetTokenBoolean(
          "enable", &envData.bDirLightsEnable[ENV_DIRLIGHT_CHARACTER]);
      textLoader.GetTokenColor(
          "diffuse", &envData.DirLights[ENV_DIRLIGHT_CHARACTER].Diffuse);
      textLoader.GetTokenColor(
          "ambient", &envData.DirLights[ENV_DIRLIGHT_CHARACTER].Ambient);
      textLoader.SetParentNode();
    }

    textLoader.SetParentNode();
  }

  if (textLoader.SetChildNode("material")) {
    textLoader.GetTokenColor("diffuse", &envData.Material.Diffuse);
    textLoader.GetTokenColor("ambient", &envData.Material.Ambient);
    textLoader.GetTokenColor("emissive", &envData.Material.Emissive);
    textLoader.SetParentNode();
  }

  if (textLoader.SetChildNode("fog")) {
    // MR-14: Fog update by Alaric
    // textLoader.GetTokenBoolean("enable", &envData.bFogEnable);
    // textLoader.GetTokenBoolean("isdensity", &envData.bDensityFog);
    // textLoader.GetTokenFloat("neardistance", &envData.m_fFogNearDistance);
    // textLoader.GetTokenFloat("fardistance", &envData.m_fFogFarDistance);
    textLoader.GetTokenByte("foglevel", &envData.bFogLevel);
    // MR-14: -- END OF -- Fog update by Alaric
    textLoader.GetTokenColor("color", &envData.FogColor);
    textLoader.SetParentNode();
  }

  if (textLoader.SetChildNode("filter")) {
    textLoader.GetTokenBoolean("enable", (BOOL *)&envData.bFilteringEnable);
    textLoader.GetTokenColor("color", &envData.FilteringColor);
    textLoader.GetTokenByte("alphasrc", &envData.byFilteringAlphaSrc);
    textLoader.GetTokenByte("alphadest", &envData.byFilteringAlphaDest);
    textLoader.SetParentNode();
  }

  if (textLoader.SetChildNode("skybox")) {
    textLoader.GetTokenBoolean("btexturerendermode",
                               (BOOL *)&envData.bSkyBoxTextureRenderMode);
    textLoader.GetTokenVector3("scale", &envData.v3SkyBoxScale);
    textLoader.GetTokenByte("gradientlevelupper",
                            &envData.bySkyBoxGradientLevelUpper);
    textLoader.GetTokenByte("gradientlevellower",
                            &envData.bySkyBoxGradientLevelLower);

    textLoader.GetTokenString("frontfacefilename",
                              &envData.strSkyBoxFaceFileName[0]);
    textLoader.GetTokenString("backfacefilename",
                              &envData.strSkyBoxFaceFileName[1]);
    textLoader.GetTokenString("leftfacefilename",
                              &envData.strSkyBoxFaceFileName[2]);
    textLoader.GetTokenString("rightfacefilename",
                              &envData.strSkyBoxFaceFileName[3]);
    textLoader.GetTokenString("topfacefilename",
                              &envData.strSkyBoxFaceFileName[4]);
    textLoader.GetTokenString("bottomfacefilename",
                              &envData.strSkyBoxFaceFileName[5]);

    textLoader.GetTokenVector2("cloudscale", &envData.v2CloudScale);
    textLoader.GetTokenFloat("cloudheight", &envData.fCloudHeight);
    textLoader.GetTokenVector2("cloudtexturescale",
                               &envData.v2CloudTextureScale);
    textLoader.GetTokenVector2("cloudspeed", &envData.v2CloudSpeed);
    textLoader.GetTokenString("cloudtexturefilename",
                              &envData.strCloudTextureFileName);

    CTokenVector *pTokenVectorCloudColor;
    if (textLoader.GetTokenVector("cloudcolor", &pTokenVectorCloudColor))
      if (0 == pTokenVectorCloudColor->size() % 8) {
        envData.CloudGradientColor.m_FirstColor.r =
            atof(pTokenVectorCloudColor->at(0).c_str());
        envData.CloudGradientColor.m_FirstColor.g =
            atof(pTokenVectorCloudColor->at(1).c_str());
        envData.CloudGradientColor.m_FirstColor.b =
            atof(pTokenVectorCloudColor->at(2).c_str());
        envData.CloudGradientColor.m_FirstColor.a =
            atof(pTokenVectorCloudColor->at(3).c_str());

        envData.CloudGradientColor.m_SecondColor.r =
            atof(pTokenVectorCloudColor->at(4).c_str());
        envData.CloudGradientColor.m_SecondColor.g =
            atof(pTokenVectorCloudColor->at(5).c_str());
        envData.CloudGradientColor.m_SecondColor.b =
            atof(pTokenVectorCloudColor->at(6).c_str());
        envData.CloudGradientColor.m_SecondColor.a =
            atof(pTokenVectorCloudColor->at(7).c_str());
      }

    BYTE byGradientCount =
        envData.bySkyBoxGradientLevelUpper + envData.bySkyBoxGradientLevelLower;
    CTokenVector *pTokenVector;
    if (textLoader.GetTokenVector("gradient", &pTokenVector))
      if (0 == pTokenVector->size() % 8)
        if (byGradientCount == pTokenVector->size() / 8) {
          envData.SkyBoxGradientColorVector.clear();
          envData.SkyBoxGradientColorVector.resize(byGradientCount);
          for (DWORD i = 0; i < byGradientCount; ++i) {
            envData.SkyBoxGradientColorVector[i].m_FirstColor.r =
                atof(pTokenVector->at(i * 8 + 0).c_str());
            envData.SkyBoxGradientColorVector[i].m_FirstColor.g =
                atof(pTokenVector->at(i * 8 + 1).c_str());
            envData.SkyBoxGradientColorVector[i].m_FirstColor.b =
                atof(pTokenVector->at(i * 8 + 2).c_str());
            envData.SkyBoxGradientColorVector[i].m_FirstColor.a =
                atof(pTokenVector->at(i * 8 + 3).c_str());

            envData.SkyBoxGradientColorVector[i].m_SecondColor.r =
                atof(pTokenVector->at(i * 8 + 4).c_str());
            envData.SkyBoxGradientColorVector[i].m_SecondColor.g =
                atof(pTokenVector->at(i * 8 + 5).c_str());
            envData.SkyBoxGradientColorVector[i].m_SecondColor.b =
                atof(pTokenVector->at(i * 8 + 6).c_str());
            envData.SkyBoxGradientColorVector[i].m_SecondColor.a =
                atof(pTokenVector->at(i * 8 + 7).c_str());
          }
        }

    textLoader.SetParentNode();
  }

  if (textLoader.SetChildNode("lensflare")) {
    textLoader.GetTokenBoolean("enable", &envData.bLensFlareEnable);
    textLoader.GetTokenColor("brightnesscolor",
                             &envData.LensFlareBrightnessColor);
    textLoader.GetTokenFloat("maxbrightness", &envData.fLensFlareMaxBrightness);
    textLoader.GetTokenBoolean("mainflareenable", &envData.bMainFlareEnable);
    textLoader.GetTokenString("mainflaretexturefilename",
                              &envData.strMainFlareTextureFileName);
    textLoader.GetTokenFloat("mainflaresize", &envData.fMainFlareSize);

    textLoader.SetParentNode();
  }
  return true;
}

void GetInterpolatedPosition(float curPositionRate,
                             TPixelPosition *PixelPosition) {}

float GetLinearInterpolation(float begin, float end, float curRate) {
  return (end - begin) * curRate + begin;
}

void PixelPositionToAttributeCellPosition(TPixelPosition PixelPosition,
                                          TCellPosition *pAttrCellPosition) {
  pAttrCellPosition->x = PixelPosition.x / c_Section_xAttributeCellSize;
  pAttrCellPosition->y = PixelPosition.y / c_Section_yAttributeCellSize;
}

void AttributeCellPositionToPixelPosition(TCellPosition AttrCellPosition,
                                          TPixelPosition *pPixelPosition) {
  pPixelPosition->x = AttrCellPosition.x * c_Section_xAttributeCellSize;
  pPixelPosition->y = AttrCellPosition.y * c_Section_yAttributeCellSize;
}

float GetPixelPositionDistance(const TPixelPosition &c_rsrcPosition,
                               const TPixelPosition &c_rdstPosition) {
  int idx = c_rsrcPosition.x - c_rdstPosition.x;
  int idy = c_rsrcPosition.y - c_rdstPosition.y;

  return sqrtf(float(idx * idx + idy * idy));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEaseOutInterpolation::CEaseOutInterpolation() { Initialize(); }

CEaseOutInterpolation::~CEaseOutInterpolation() {}

void CEaseOutInterpolation::Initialize() {
  m_fRemainingTime = 0.0f;
  m_fValue = 0.0f;
  m_fSpeed = 0.0f;
  m_fAcceleration = 0.0f;
  m_fStartValue = 0.0f;
  m_fLastValue = 0.0f;
}

BOOL CEaseOutInterpolation::Setup(float fStart, float fEnd, float fTime) {
  // for safety
  if (fabs(fTime) < FLT_EPSILON) {
    fTime = 0.01f;
  }

  m_fValue = fStart;
  m_fStartValue = fStart;
  m_fLastValue = fStart;

  m_fSpeed = (2.0f * (fEnd - fStart)) / fTime;
  m_fAcceleration =
      2.0f * (fEnd - fStart) / (fTime * fTime) - 2.0f * m_fSpeed / fTime;
  m_fRemainingTime = fTime;

  return TRUE;
}

void CEaseOutInterpolation::Interpolate(float fElapsedTime) {
  m_fLastValue = m_fValue;

  m_fRemainingTime -= fElapsedTime;
  m_fSpeed += m_fAcceleration * fElapsedTime;
  m_fValue += m_fSpeed * fElapsedTime;

  if (!isPlaying()) {
    m_fValue = 0.0f;
    m_fLastValue = 0.0f;
  }
}

BOOL CEaseOutInterpolation::isPlaying() { return m_fRemainingTime > 0.0f; }

float CEaseOutInterpolation::GetValue() { return m_fValue; }

float CEaseOutInterpolation::GetChangingValue() {
  return m_fValue - m_fLastValue;
}