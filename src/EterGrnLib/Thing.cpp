#include "StdAfx.h"
#include "Eterbase/Debug.h"
#include "Thing.h"
#include "ThingInstance.h"

CGraphicThing::CGraphicThing(const char *c_szFileName)
    : CResource(c_szFileName) {
  Initialize();
}

CGraphicThing::~CGraphicThing() {
  // OnClear();
  Clear();
}

void CGraphicThing::Initialize() {
  m_pgrnFile = NULL;
  m_pgrnFileInfo = NULL;
  m_pgrnAni = NULL;

  m_models = NULL;
  m_motions = NULL;
}

void CGraphicThing::OnClear() {
  if (m_motions)
    delete[] m_motions;

  if (m_models)
    delete[] m_models;

  if (m_pgrnFile)
    GrannyFreeFile(m_pgrnFile);

  Initialize();
}

CGraphicThing::TType CGraphicThing::Type() {
  static TType s_type = StringToType("CGraphicThing");
  return s_type;
}

bool CGraphicThing::OnIsEmpty() const { return m_pgrnFile ? false : true; }

bool CGraphicThing::OnIsType(TType type) {
  if (CGraphicThing::Type() == type)
    return true;

  return CResource::OnIsType(type);
}

bool CGraphicThing::CreateDeviceObjects() {
  if (!m_pgrnFileInfo)
    return true;

  for (int m = 0; m < m_pgrnFileInfo->ModelCount; ++m) {
    CGrannyModel &rModel = m_models[m];
    rModel.CreateDeviceObjects();
  }

  return true;
}

void CGraphicThing::DestroyDeviceObjects() {
  if (!m_pgrnFileInfo)
    return;

  for (int m = 0; m < m_pgrnFileInfo->ModelCount; ++m) {
    CGrannyModel &rModel = m_models[m];
    rModel.DestroyDeviceObjects();
  }
}

bool CGraphicThing::CheckModelIndex(int iModel) const {
  if (!m_pgrnFileInfo) {
    Tracef("m_pgrnFileInfo == NULL: %s\n", GetFileName());
    return false;
  }

  assert(m_pgrnFileInfo != NULL);

  if (iModel < 0)
    return false;

  if (iModel >= m_pgrnFileInfo->ModelCount)
    return false;

  return true;
}

bool CGraphicThing::CheckMotionIndex(int iMotion) const {
  // Temporary
  if (!m_pgrnFileInfo)
    return false;
  // Temporary

  assert(m_pgrnFileInfo != NULL);

  if (iMotion < 0)
    return false;

  if (iMotion >= m_pgrnFileInfo->AnimationCount)
    return false;

  return true;
}

CGrannyModel *CGraphicThing::GetModelPointer(int iModel) {
  assert(CheckModelIndex(iModel));
  assert(m_models != NULL);
  return m_models + iModel;
}

CGrannyMotion *CGraphicThing::GetMotionPointer(int iMotion) {
  assert(CheckMotionIndex(iMotion));

  if (iMotion >= m_pgrnFileInfo->AnimationCount)
    return NULL;

  assert(m_motions != NULL);
  return (m_motions + iMotion);
}

int CGraphicThing::GetModelCount() const {
  if (!m_pgrnFileInfo)
    return 0;

  return (m_pgrnFileInfo->ModelCount);
}

int CGraphicThing::GetMotionCount() const {
  if (!m_pgrnFileInfo)
    return 0;

  return (m_pgrnFileInfo->AnimationCount);
}

bool CGraphicThing::OnLoad(int iSize, const void *c_pvBuf) {
  if (!c_pvBuf)
    return false;

  m_pgrnFile = GrannyReadEntireFileFromMemory(iSize, (void *)c_pvBuf);

  if (!m_pgrnFile)
    return false;

  m_pgrnFileInfo = GrannyGetFileInfo(m_pgrnFile);

  if (!m_pgrnFileInfo)
    return false;

  LoadModels();
  LoadMotions();
  return true;
}

// SUPPORT_LOCAL_TEXTURE
static std::string gs_modelLocalPath;

const std::string &GetModelLocalPath() { return gs_modelLocalPath; }
// END_OF_SUPPORT_LOCAL_TEXTURE

bool CGraphicThing::LoadModels() {
  assert(m_pgrnFile != NULL);
  assert(m_models == NULL);

  if (m_pgrnFileInfo->ModelCount <= 0)
    return false;

  // SUPPORT_LOCAL_TEXTURE
  const std::string &fileName = GetFileNameString();

  // char localPath[256] = "";
  if (fileName.length() > 2 && fileName[1] != ':') {
    int sepPos = fileName.rfind('\\');
    gs_modelLocalPath.assign(fileName, 0, sepPos + 1);
  }
  // END_OF_SUPPORT_LOCAL_TEXTURE

  int modelCount = m_pgrnFileInfo->ModelCount;

  m_models = new CGrannyModel[modelCount];

  for (int m = 0; m < modelCount; ++m) {
    CGrannyModel &rModel = m_models[m];
    granny_model *pgrnModel = m_pgrnFileInfo->Models[m];

    if (!rModel.CreateFromGrannyModelPointer(pgrnModel))
      return false;
  }

  GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidVertexSection);
  GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidIndexSection);
  GrannyFreeFileSection(m_pgrnFile, GrannyStandardDeformableIndexSection);
  GrannyFreeFileSection(m_pgrnFile, GrannyStandardTextureSection);
  return true;
}

bool CGraphicThing::LoadMotions() {
  assert(m_pgrnFile != NULL);
  assert(m_motions == NULL);

  if (m_pgrnFileInfo->AnimationCount <= 0)
    return false;

  int motionCount = m_pgrnFileInfo->AnimationCount;

  m_motions = new CGrannyMotion[motionCount];

  for (int m = 0; m < motionCount; ++m)
    if (!m_motions[m].BindGrannyAnimation(m_pgrnFileInfo->Animations[m]))
      return false;

  return true;
}
