#include "StdAfx.h"
#include "Type.h"

BOOL GetTokenTimeEventFloat(CTextFileLoader &rTextFileLoader,
                            const char *c_szKey,
                            TTimeEventTableFloat *pTimeEventTableFloat) {
  CTokenVector *pTokenVector;
  if (!rTextFileLoader.GetTokenVector(c_szKey, &pTokenVector))
    return FALSE;

  pTimeEventTableFloat->clear();
  pTimeEventTableFloat->resize(pTokenVector->size() / 2);

  DWORD dwIndex = 0;
  for (DWORD i = 0; i < pTokenVector->size(); i += 2, ++dwIndex) {
    pTimeEventTableFloat->at(dwIndex).m_fTime =
        atof(pTokenVector->at(i).c_str());
    pTimeEventTableFloat->at(dwIndex).m_Value =
        atof(pTokenVector->at(i + 1).c_str());
  }

  return TRUE;
}

void InsertItemTimeEventFloat(TTimeEventTableFloat *pTable, float fTime,
                              float fValue) {
  TTimeEventTableFloat::iterator itor = pTable->begin();
  for (; itor != pTable->end(); ++itor) {
    TTimeEventTypeFloat &rTimeEvent = *itor;
    if (rTimeEvent.m_fTime > fTime)
      break;
  }

  TTimeEventTypeFloat TimeEvent;
  TimeEvent.m_fTime = fTime;
  TimeEvent.m_Value = fValue;

  pTable->insert(itor, TimeEvent);
}
