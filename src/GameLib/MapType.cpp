#include "StdAfx.h"
#include "EterBase/Filename.h"
#include "Property.h"

float SEnvironmentData::GetFogNearDistance() const {
  return m_fFogNearDistance;
}

float SEnvironmentData::GetFogFarDistance() const { return m_fFogFarDistance; }

float SPixelPosition_CalculateDistanceSq3d(
    const TPixelPosition &c_rkPPosLeft, const TPixelPosition &c_rkPPosRight) {
  float dx = c_rkPPosLeft.x - c_rkPPosRight.x;
  float dy = c_rkPPosLeft.y - c_rkPPosRight.y;
  float dz = c_rkPPosLeft.z - c_rkPPosRight.z;

  return dx * dx + dy * dy + dz * dz;
}

namespace prt {

DWORD GetPropertyType(const char *c_szTypeName) {
  for (DWORD i = 0; i < PROPERTY_TYPE_MAX_NUM; ++i) {
    if (!strcmp(c_szPropertyTypeName[i], c_szTypeName))
      return i;
  }

  return PROPERTY_TYPE_NONE;
}

const char *GetPropertyExtension(DWORD dwType) {
  if (dwType >= PROPERTY_TYPE_MAX_NUM)
    return c_szPropertyExtension[0];

  return c_szPropertyExtension[dwType];
}

const char *IntegerNumberToString(int iNumber) {
  static char szString[16 + 1];
  _snprintf(szString, sizeof(szString), "%d", iNumber);
  return szString;
}

const char *FloatNumberToString(float fNumber) {
  static char szString[16 + 1];
  _snprintf(szString, sizeof(szString), "%f", fNumber);
  return szString;
}

bool PropertyTreeDataToString(TPropertyTree *pData, CProperty *pProperty) {
  pProperty->Clear();

  pProperty->PutString("PropertyType", "Tree");
  pProperty->PutString("PropertyName", pData->strName.c_str());

  pProperty->PutString("TreeFile", pData->strFileName.c_str());
  pProperty->PutString("TreeSize", FloatNumberToString(pData->fSize));
  pProperty->PutString("TreeVariance", FloatNumberToString(pData->fVariance));
  return true;
}

bool PropertyTreeStringToData(CProperty *pProperty, TPropertyTree *pData) {
  const char *c_pszPropertyType;
  const char *c_pszPropertyName;

  if (!pProperty->GetString("PropertyType", &c_pszPropertyType))
    return false;

  if (!pProperty->GetString("PropertyName", &c_pszPropertyName))
    return false;

  if (strcmp(c_pszPropertyType, "Tree"))
    return false;

  pData->strName = c_pszPropertyName;

  ///////////////////////////////////////////////////////////////////////////////////

  const char *c_pszTreeName;
  const char *c_pszTreeSize;
  const char *c_pszTreeVariance;
  if (!pProperty->GetString("TreeFile", &c_pszTreeName))
    return false;
  if (!pProperty->GetString("TreeSize", &c_pszTreeSize))
    return false;
  if (!pProperty->GetString("TreeVariance", &c_pszTreeVariance))
    return false;

  pData->strFileName = c_pszTreeName;
  pData->fSize = atof(c_pszTreeSize);
  pData->fVariance = atof(c_pszTreeVariance);

  return true;
}

bool PropertyBuildingDataToString(TPropertyBuilding *pData,
                                  CProperty *pProperty) {
  pProperty->Clear();

  pProperty->PutString("PropertyType", "Building");
  pProperty->PutString("PropertyName", pData->strName.c_str());
  pProperty->PutString("BuildingFile", pData->strFileName.c_str());
  pProperty->PutString("ShadowFlag",
                       IntegerNumberToString(pData->isShadowFlag));
  return true;
}

bool PropertyBuildingStringToData(CProperty *pProperty,
                                  TPropertyBuilding *pData) {
  const char *c_pszPropertyType;
  const char *c_pszPropertyName;

  if (!pProperty->GetString("PropertyType", &c_pszPropertyType))
    return false;

  if (!pProperty->GetString("PropertyName", &c_pszPropertyName))
    return false;

  if (strcmp(c_pszPropertyType, "Building"))
    return false;

  pData->strName = c_pszPropertyName;

  ///////////////////////////////////////////////////////////////////////////////////

  const char *c_pszBuildingName;
  if (!pProperty->GetString("BuildingFile", &c_pszBuildingName))
    return false;

  pData->strFileName = c_pszBuildingName;
  pData->strAttributeDataFileName =
      CFileNameHelper::NoExtension(pData->strFileName) + ".mdatr";

  const char *c_pszShadowFlag;
  if (!pProperty->GetString("ShadowFlag", &c_pszShadowFlag)) {
    pData->isShadowFlag = FALSE;
  } else {
    pData->isShadowFlag = atoi(c_pszShadowFlag);
  }

  return true;
}

bool PropertyEffectDataToString(TPropertyEffect *pData, CProperty *pProperty) {
  pProperty->Clear();

  pProperty->PutString("PropertyType", "Effect");
  pProperty->PutString("PropertyName", pData->strName.c_str());
  pProperty->PutString("EffectFile", pData->strFileName.c_str());
  return true;
}

bool PropertyEffectStringToData(CProperty *pProperty, TPropertyEffect *pData) {
  const char *c_pszPropertyType;
  const char *c_pszPropertyName;

  if (!pProperty->GetString("PropertyType", &c_pszPropertyType))
    return false;

  if (!pProperty->GetString("PropertyName", &c_pszPropertyName))
    return false;

  if (strcmp(c_pszPropertyType, "Effect"))
    return false;

  pData->strName = c_pszPropertyName;

  ///////////////////////////////////////////////////////////////////////////////////

  const char *c_pszEffectName;
  if (!pProperty->GetString("EffectFile", &c_pszEffectName))
    return false;

  pData->strFileName = c_pszEffectName;

  return true;
}

bool PropertyAmbienceDataToString(TPropertyAmbience *pData,
                                  CProperty *pProperty) {
  pProperty->Clear();
  pProperty->PutString("PropertyType", "Ambience");
  pProperty->PutString("PropertyName", pData->strName.c_str());
  pProperty->PutString("PlayType", pData->strPlayType.c_str());
  pProperty->PutString("PlayInterval",
                       FloatNumberToString(pData->fPlayInterval));
  pProperty->PutString("PlayIntervalVariation",
                       FloatNumberToString(pData->fPlayIntervalVariation));
  pProperty->PutString("MaxVolumeAreaPercentage",
                       FloatNumberToString(pData->fMaxVolumeAreaPercentage));

  CTokenVector AmbienceSoundVector;
  std::vector<std::string>::iterator itor = pData->AmbienceSoundVector.begin();
  for (; itor != pData->AmbienceSoundVector.end(); ++itor) {
    std::string &rstrToken = *itor;
    AmbienceSoundVector.push_back(rstrToken.c_str());
  }
  pProperty->PutVector("AmbienceSoundVector", AmbienceSoundVector);
  return true;
}

bool PropertyAmbienceStringToData(CProperty *pProperty,
                                  TPropertyAmbience *pData) {
  const char *c_pszPropertyType;
  const char *c_pszPropertyName;

  if (!pProperty->GetString("PropertyType", &c_pszPropertyType))
    return false;

  if (!pProperty->GetString("PropertyName", &c_pszPropertyName))
    return false;

  if (strcmp(c_pszPropertyType, "Ambience"))
    return false;

  pData->strName = c_pszPropertyName;

  ///////////////////////////////////////////////////////////////////////////////////

  const char *c_pszPlayType;
  const char *c_pszPlayInterval;
  const char *c_pszPlayIntervalVariation;
  const char *c_pszMaxVolumeAreaPercentage = NULL;
  CTokenVector AmbienceSoundVector;
  if (!pProperty->GetString("PlayType", &c_pszPlayType))
    return false;
  if (!pProperty->GetString("PlayInterval", &c_pszPlayInterval))
    return false;
  if (!pProperty->GetString("PlayIntervalVariation",
                            &c_pszPlayIntervalVariation))
    return false;
  if (!pProperty->GetString("MaxVolumeAreaPercentage",
                            &c_pszMaxVolumeAreaPercentage)) {
    pData->fMaxVolumeAreaPercentage = 0.0f;
  }
  if (!pProperty->GetVector("AmbienceSoundVector", AmbienceSoundVector))
    return false;

  pData->strPlayType = c_pszPlayType;
  pData->fPlayInterval = atof(c_pszPlayInterval);
  pData->fPlayIntervalVariation = atof(c_pszPlayIntervalVariation);
  if (c_pszMaxVolumeAreaPercentage)
    pData->fMaxVolumeAreaPercentage = atof(c_pszMaxVolumeAreaPercentage);
  for (CTokenVector::iterator itor = AmbienceSoundVector.begin();
       itor != AmbienceSoundVector.end(); ++itor)
    pData->AmbienceSoundVector.push_back(*itor);

  return true;
}

bool PropertyDungeonBlockDataToString(TPropertyDungeonBlock *pData,
                                      CProperty *pProperty) {
  pProperty->Clear();
  pProperty->PutString("PropertyType", "DungeonBlock");
  pProperty->PutString("PropertyName", pData->strName.c_str());
  pProperty->PutString("DungeonBlockFile", pData->strFileName.c_str());
  return true;
}
bool PropertyDungeonBlockStringToData(CProperty *pProperty,
                                      TPropertyDungeonBlock *pData) {
  const char *c_pszPropertyType;
  const char *c_pszPropertyName;

  if (!pProperty->GetString("PropertyType", &c_pszPropertyType))
    return false;

  if (!pProperty->GetString("PropertyName", &c_pszPropertyName))
    return false;

  if (strcmp(c_pszPropertyType, "DungeonBlock"))
    return false;

  pData->strName = c_pszPropertyName;

  ///////////////////////////////////////////////////////////////////////////////////

  const char *c_pszDungeonBlockFileName = NULL;
  if (!pProperty->GetString("dungeonblockfile", &c_pszDungeonBlockFileName))
    return false;

  pData->strFileName = c_pszDungeonBlockFileName;
  pData->strAttributeDataFileName =
      CFileNameHelper::NoExtension(pData->strFileName) + std::string(".mdatr");

  return true;
}

}; // namespace prt
