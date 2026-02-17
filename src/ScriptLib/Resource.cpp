#include "StdAfx.h"
#include "EterLib/GrpExpandedImageInstance.h"
#include "EterLib/GrpTextInstance.h"
#include "EterLib/GrpMarkInstance.h"
#include "EterLib/GrpSubImage.h"
#include "EterLib/GrpText.h"
#include "EterLib/AttributeData.h"
#include "EterGrnLib/Thing.h"
#include "EterGrnLib/ThingInstance.h"
#include "EffectLib/EffectMesh.h"
#include "EffectLib/EffectInstance.h"
#include "Gamelib/WeaponTrace.h"
#include "Gamelib/MapType.h"
#include "Gamelib/GameType.h"
#include "Gamelib/RaceData.h"
#include "Gamelib/RaceMotionData.h"
#include "Gamelib/ActorInstance.h"
#include "Gamelib/Area.h"
#include "Gamelib/ItemData.h"
#include "Gamelib/FlyingData.h"
#include "Gamelib/FlyTrace.h"
#include "Gamelib/FlyingInstance.h"
#include "Gamelib/FlyingData.h"

#include "Resource.h"

CResource *NewImage(const char *c_szFileName) {
  return new CGraphicImage(c_szFileName);
}

CResource *NewSubImage(const char *c_szFileName) {
  return new CGraphicSubImage(c_szFileName);
}

CResource *NewText(const char *c_szFileName) {
  return new CGraphicText(c_szFileName);
}

CResource *NewThing(const char *c_szFileName) {
  return new CGraphicThing(c_szFileName);
}

CResource *NewEffectMesh(const char *c_szFileName) {
  return new CEffectMesh(c_szFileName);
}

CResource *NewAttributeData(const char *c_szFileName) {
  return new CAttributeData(c_szFileName);
}

void CPythonResource::DumpFileList(const char *c_szFileName) {
  m_resManager.DumpFileListToTextFile(c_szFileName);
}

void CPythonResource::Destroy() {
  CFlyingInstance::DestroySystem();
  CActorInstance::DestroySystem();
  CArea::DestroySystem();
  CGraphicExpandedImageInstance::DestroySystem();
  CGraphicImageInstance::DestroySystem();
  CGraphicMarkInstance::DestroySystem();
  CGraphicThingInstance::DestroySystem();
  CGrannyModelInstance::DestroySystem();
  CGraphicTextInstance::DestroySystem();
  CEffectInstance::DestroySystem();
  CWeaponTrace::DestroySystem();
  CFlyTrace::DestroySystem();

  m_resManager.DestroyDeletingList();

  CFlyingData::DestroySystem();
  CItemData::DestroySystem();
  CEffectData::DestroySystem();
  CEffectMesh::SEffectMeshData::DestroySystem();
  CRaceData::DestroySystem();
  NRaceData::DestroySystem();
  CRaceMotionData::DestroySystem();

  m_resManager.Destroy();
}

CPythonResource::CPythonResource() {
  m_resManager.RegisterResourceNewFunctionPointer("sub", NewSubImage);
  m_resManager.RegisterResourceNewFunctionPointer("dds", NewImage);
  m_resManager.RegisterResourceNewFunctionPointer("jpg", NewImage);
  m_resManager.RegisterResourceNewFunctionPointer("tga", NewImage);
  m_resManager.RegisterResourceNewFunctionPointer("bmp", NewImage);
  m_resManager.RegisterResourceNewFunctionPointer("fnt", NewText);
  m_resManager.RegisterResourceNewFunctionPointer("gr2", NewThing);
  m_resManager.RegisterResourceNewFunctionPointer("mde", NewEffectMesh);
  m_resManager.RegisterResourceNewFunctionPointer("mdatr", NewAttributeData);
}

CPythonResource::~CPythonResource() {}
