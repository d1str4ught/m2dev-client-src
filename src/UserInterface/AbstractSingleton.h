#pragma once

template <typename T> class TAbstractSingleton {
  static T *ms_singleton;

public:
  TAbstractSingleton() {
    assert(!ms_singleton);
    intptr_t offset = (intptr_t)(T *)1 - (intptr_t)(CSingleton<T> *)(T *)1;
    ms_singleton = (T *)((intptr_t)this + offset);
  }

  virtual ~TAbstractSingleton() {
    assert(ms_singleton);
    ms_singleton = 0;
  }

  __forceinline static T &GetSingleton() {
    assert(ms_singleton != NULL);
    return (*ms_singleton);
  }
};

template <typename T> T *TAbstractSingleton<T>::ms_singleton = 0;