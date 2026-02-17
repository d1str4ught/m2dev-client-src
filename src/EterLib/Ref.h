#ifndef __INC_REF_H__
#define __INC_REF_H__

#include "ReferenceObject.h"

#include <assert.h>

template <typename T> class CRef {
public:
  struct FClear {
    void operator()(CRef<T> &rRef) { rRef.Clear(); }
  };

public:
  CRef() : m_pObject(NULL) {}

  CRef(CReferenceObject *pObject) {
    m_pObject = NULL;
    Initialize(pObject);
  }

  CRef(const CRef &c_rRef) {
    m_pObject = NULL;
    Initialize(c_rRef.m_pObject);
  }

  ~CRef() { Clear(); }

  void operator=(CReferenceObject *pObject) { SetPointer(pObject); }

  void operator=(const CRef &c_rRef) { SetPointer(c_rRef.m_pObject); }

  void Clear() {
    if (m_pObject) {
      m_pObject->Release();
      m_pObject = NULL;
    }
  }

  bool IsNull() const { return m_pObject == NULL ? true : false; }

  void SetPointer(CReferenceObject *pObject) {
    CReferenceObject *pOldObject = m_pObject;

    m_pObject = pObject;

    if (m_pObject)
      m_pObject->AddReference();

    if (pOldObject)
      pOldObject->Release();
  }

  T *GetPointer() const { return static_cast<T *>(m_pObject); }

  T *operator->() const {
    assert(m_pObject != NULL);
    return static_cast<T *>(m_pObject);
  }

private:
  void Initialize(CReferenceObject *pObject) {
    assert(m_pObject == NULL);

    m_pObject = pObject;

    if (m_pObject)
      m_pObject->AddReference();
  }

private:
  CReferenceObject *m_pObject;
};

#endif
