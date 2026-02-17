#include "StdAfx.h"

bool PyTuple_GetImageInstance(PyObject *poArgs, int pos,
                              CGraphicImageInstance **ppRetImageInstance) {
  unsigned long long handle;

  if (!PyTuple_GetUnsignedLongLong(poArgs, pos, &handle))
    return false;

  if (!handle)
    return false;

  *ppRetImageInstance = (CGraphicImageInstance *)handle;
  return true;
}

bool PyTuple_GetExpandedImageInstance(
    PyObject *poArgs, int pos,
    CGraphicExpandedImageInstance **ppRetImageInstance) {
  unsigned long long handle;

  if (!PyTuple_GetUnsignedLongLong(poArgs, pos, &handle))
    return false;

  if (!handle)
    return false;

  if (!((CGraphicImageInstance *)handle)
           ->IsType(CGraphicExpandedImageInstance::Type()))
    return false;

  *ppRetImageInstance = (CGraphicExpandedImageInstance *)handle;

  return true;
}

PyObject *grpImageGenerate(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;

  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BadArgument();

  if (!*szFileName)
    return Py_BuildValue("K", 0);

  CResource *pResource =
      CResourceManager::Instance().GetResourcePointer(szFileName);

  if (!pResource->IsType(CGraphicImage::Type()))
    return Py_BuildException("Resource is not an image (filename: %s)",
                             szFileName);

  CGraphicImageInstance *pImageInstance = CGraphicImageInstance::New();
  pImageInstance->SetImagePointer(static_cast<CGraphicImage *>(pResource));

  if (pImageInstance->IsEmpty())
    return Py_BuildException("Cannot load image (filename: %s)", szFileName);

  return Py_BuildValue("K", pImageInstance);
}

PyObject *grpImageGenerateExpanded(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;

  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BadArgument();

  if (strlen(szFileName) <= 0)
    return Py_BuildValue("K", 0);

  CResource *pResource =
      CResourceManager::Instance().GetResourcePointer(szFileName);

  if (!pResource->IsType(CGraphicImage::Type()))
    return Py_BuildException("Resource is not an image (filename: %s)",
                             szFileName);

  CGraphicExpandedImageInstance *pImageInstance =
      CGraphicExpandedImageInstance::New();
  pImageInstance->SetImagePointer(static_cast<CGraphicImage *>(pResource));

  if (pImageInstance->IsEmpty())
    return Py_BuildException("Cannot load image (filename: %s)", szFileName);

  return Py_BuildValue("K", pImageInstance);
}

PyObject *grpImageGenerateFromHandle(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImage *iHandle;
  if (!PyTuple_GetPointer(poArgs, 0, &iHandle))
    return Py_BadArgument();

  CGraphicImageInstance *pImageInstance = CGraphicImageInstance::New();
  pImageInstance->SetImagePointer(iHandle);

  return Py_BuildValue("K", pImageInstance);
}

PyObject *grpImageDelete(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;

  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  if (!pImageInstance)
    return Py_BuildNone();

  CGraphicImageInstance::Delete(pImageInstance);

  return Py_BuildNone();
}

PyObject *grpImageDeleteExpanded(PyObject *poSelf, PyObject *poArgs) {
  CGraphicExpandedImageInstance *pExpandedImageInstance;
  if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pExpandedImageInstance))
    return Py_BadArgument();

  CGraphicExpandedImageInstance::Delete(pExpandedImageInstance);
  return Py_BuildNone();
}

PyObject *grpImageSetFileName(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;
  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  char *szFileName;
  if (!PyTuple_GetString(poArgs, 1, &szFileName))
    return Py_BadArgument();

  CResource *pResource =
      CResourceManager::Instance().GetResourcePointer(szFileName);

  if (!pResource->IsType(CGraphicImage::Type()))
    return Py_BuildException("Resource is not an image (filename: %s)",
                             szFileName);

  pImageInstance->SetImagePointer(static_cast<CGraphicImage *>(pResource));
  return Py_BuildNone();
}

PyObject *grpImageRender(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;
  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  pImageInstance->Render();
  return Py_BuildNone();
}

PyObject *grpSetImagePosition(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;
  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  float x;
  if (!PyTuple_GetFloat(poArgs, 1, &x))
    return Py_BadArgument();

  float y;
  if (!PyTuple_GetFloat(poArgs, 2, &y))
    return Py_BadArgument();

  pImageInstance->SetPosition(x, y);
  return Py_BuildNone();
}

PyObject *grpSetImageOrigin(PyObject *poSelf, PyObject *poArgs) {
  CGraphicExpandedImageInstance *pImageInstance;
  if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  float x;
  if (!PyTuple_GetFloat(poArgs, 1, &x))
    return Py_BadArgument();
  float y;
  if (!PyTuple_GetFloat(poArgs, 2, &y))
    return Py_BadArgument();

  pImageInstance->SetOrigin(x, y);
  return Py_BuildNone();
}

PyObject *grpSetImageRotation(PyObject *poSelf, PyObject *poArgs) {
  CGraphicExpandedImageInstance *pImageInstance;
  if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  float Degree;
  if (!PyTuple_GetFloat(poArgs, 1, &Degree))
    return Py_BadArgument();

  pImageInstance->SetRotation(Degree);
  return Py_BuildNone();
}

PyObject *grpSetImageScale(PyObject *poSelf, PyObject *poArgs) {
  CGraphicExpandedImageInstance *pImageInstance;
  if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  float x;
  if (!PyTuple_GetFloat(poArgs, 1, &x))
    return Py_BadArgument();
  float y;
  if (!PyTuple_GetFloat(poArgs, 2, &y))
    return Py_BadArgument();

  pImageInstance->SetScale(x, y);
  return Py_BuildNone();
}

PyObject *grpSetRenderingRect(PyObject *poSelf, PyObject *poArgs) {
  CGraphicExpandedImageInstance *pImageInstance;
  if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  float fLeft;
  if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
    return Py_BadArgument();
  float fTop;
  if (!PyTuple_GetFloat(poArgs, 2, &fTop))
    return Py_BadArgument();
  float fRight;
  if (!PyTuple_GetFloat(poArgs, 3, &fRight))
    return Py_BadArgument();
  float fBottom;
  if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
    return Py_BadArgument();

  pImageInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom);
  return Py_BuildNone();
}

PyObject *grpSetImageDiffuseColor(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;
  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  float r;
  if (!PyTuple_GetFloat(poArgs, 1, &r))
    return Py_BadArgument();
  float g;
  if (!PyTuple_GetFloat(poArgs, 2, &g))
    return Py_BadArgument();
  float b;
  if (!PyTuple_GetFloat(poArgs, 3, &b))
    return Py_BadArgument();
  float a;
  if (!PyTuple_GetFloat(poArgs, 4, &a))
    return Py_BadArgument();

  pImageInstance->SetDiffuseColor(r, g, b, a);
  return Py_BuildNone();
}

PyObject *grpGetWidth(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;

  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  if (pImageInstance->IsEmpty())
    return Py_BuildException("Image is empty");

  return Py_BuildValue("i", pImageInstance->GetWidth());
}

PyObject *grpGetHeight(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImageInstance;

  if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
    return Py_BadArgument();

  if (pImageInstance->IsEmpty())
    return Py_BuildException("Image is empty");

  return Py_BuildValue("i", pImageInstance->GetHeight());
}

void initgrpImage() {
  static PyMethodDef s_methods[] = {
      {"Render", grpImageRender, METH_VARARGS},
      {"SetPosition", grpSetImagePosition, METH_VARARGS},
      {"Generate", grpImageGenerate, METH_VARARGS},
      {"GenerateExpanded", grpImageGenerateExpanded, METH_VARARGS},
      {"GenerateFromHandle", grpImageGenerateFromHandle, METH_VARARGS},
      {"Delete", grpImageDelete, METH_VARARGS},
      {"DeleteExpanded", grpImageDeleteExpanded, METH_VARARGS},
      {"SetFileName", grpImageSetFileName, METH_VARARGS},
      {"SetOrigin", grpSetImageOrigin, METH_VARARGS},
      {"SetRotation", grpSetImageRotation, METH_VARARGS},
      {"SetScale", grpSetImageScale, METH_VARARGS},
      {"SetRenderingRect", grpSetRenderingRect, METH_VARARGS},
      {"SetDiffuseColor", grpSetImageDiffuseColor, METH_VARARGS},
      {"GetWidth", grpGetWidth, METH_VARARGS},
      {"GetHeight", grpGetHeight, METH_VARARGS},
      {NULL, NULL, NULL},
  };

  Py_InitModule("grpImage", s_methods);
}
