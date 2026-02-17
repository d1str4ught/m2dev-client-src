#include "StdAfx.h"
#include "PythonApplication.h"

PyObject *sndPlaySound2D(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  SoundEngine::Instance().PlaySound2D(szFileName);
  return Py_BuildNone();
}

PyObject *sndPlaySound3D(PyObject *poSelf, PyObject *poArgs) {
  float fx;
  if (!PyTuple_GetFloat(poArgs, 0, &fx))
    return Py_BuildException();
  float fy;
  if (!PyTuple_GetFloat(poArgs, 1, &fy))
    return Py_BuildException();
  float fz;
  if (!PyTuple_GetFloat(poArgs, 2, &fz))
    return Py_BuildException();
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 3, &szFileName))
    return Py_BuildException();

  SoundEngine::Instance().PlaySound3D(szFileName, fx, fy, fz);
  return Py_BuildNone();
}

PyObject *sndFadeInMusic(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  SoundEngine::Instance().FadeInMusic(szFileName,
                                      SoundEngine::Instance().GetMusicVolume());
  return Py_BuildNone();
}

PyObject *sndFadeOutMusic(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  SoundEngine::Instance().FadeOutMusic(szFileName);
  return Py_BuildNone();
}

PyObject *sndFadeOutAllMusic(PyObject *poSelf, PyObject *poArgs) {
  SoundEngine::Instance().FadeOutAllMusic();
  return Py_BuildNone();
}

PyObject *sndFadeLimitOutMusic(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  float fLimitVolume;
  if (!PyTuple_GetFloat(poArgs, 1, &fLimitVolume))
    return Py_BuildException();

  SoundEngine::Instance().FadeOutMusic(szFileName, fLimitVolume);
  return Py_BuildNone();
}

PyObject *sndStopAllSound(PyObject *poSelf, PyObject *poArgs) {
  SoundEngine::Instance().StopAllSound3D();
  return Py_BuildNone();
}

PyObject *sndSetMasterVolume(PyObject *poSelf, PyObject *poArgs) {
  float fVolume;
  if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
    return Py_BuildException();

  SoundEngine::Instance().SetMasterVolume(fVolume);
  return Py_BuildNone();
}

PyObject *sndSetMusicVolume(PyObject *poSelf, PyObject *poArgs) {
  float fVolume;
  if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
    return Py_BuildException();

  SoundEngine::Instance().SetMusicVolume(fVolume);
  return Py_BuildNone();
}

PyObject *sndSetSoundVolume(PyObject *poSelf, PyObject *poArgs) {
  float fVolume;
  if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
    return Py_BuildException();

  SoundEngine::Instance().SetSoundVolume(fVolume);
  return Py_BuildNone();
}

void initsnd() {
  static PyMethodDef s_methods[] = {
      {"PlaySound", sndPlaySound2D, METH_VARARGS},
      {"PlaySound3D", sndPlaySound3D, METH_VARARGS},
      {"FadeInMusic", sndFadeInMusic, METH_VARARGS},
      {"FadeOutMusic", sndFadeOutMusic, METH_VARARGS},
      {"FadeOutAllMusic", sndFadeOutAllMusic, METH_VARARGS},
      {"FadeLimitOutMusic", sndFadeLimitOutMusic, METH_VARARGS},
      {"StopAllSound", sndStopAllSound, METH_VARARGS},

      {"SetMasterVolume", sndSetMasterVolume, METH_VARARGS},
      {"SetMusicVolume", sndSetMusicVolume, METH_VARARGS},
      {"SetSoundVolume", sndSetSoundVolume, METH_VARARGS},
      {NULL, NULL, NULL},
  };

  Py_InitModule("snd", s_methods);
}
