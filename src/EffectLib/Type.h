#pragma once
#include <d3dx9math.h>

#define Clamp(x, min, max) x = (x < min ? min : x < max ? x : max);
#define GRAVITY D3DXVECTOR3(0.0f, 0.0f, -9.8f)

#define MAX_FRAME 20
#define MAX_TEXTURE 20

typedef struct _FVF_POINT {
  float x, y, z;
} FVF_POINT;

#ifndef D3DFVF_POINT
#define D3DFVF_POINT (D3DFVF_XYZ)
#endif

typedef struct _FVF_PT {
  float x, y, z;
  float tu, tv;
} FVF_PT;

#ifndef D3DFVF_PT
#define D3DFVF_PT (D3DFVF_XYZ | D3DFVF_TEX1)
#endif

typedef struct _FVF_PDT {
  float x, y, z;
  DWORD color;
  float tu, tv;
} FVF_PDT;

#ifndef D3DFVF_PDT
#define D3DFVF_PDT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#endif

inline FVF_PDT _FVF_PDT(float x, float y, float z, DWORD dif, float u,
                        float v) {
  FVF_PDT result;
  result.x = x;
  result.y = y;
  result.z = z;
  result.color = dif;
  result.tu = u;
  result.tv = v;
  return result;
}

enum EEffectType {
  EFFECT_TYPE_PARTICLE = 1,
  EFFECT_TYPE_ANIMATION_TEXTURE = 2,
  EFFECT_TYPE_MESH = 3,
  EFFECT_TYPE_SIMPLE_LIGHT = 4,
};

enum EMeshBillBoardType {
  MESH_BILLBOARD_TYPE_NONE,

  MESH_BILLBOARD_TYPE_ALL,
  MESH_BILLBOARD_TYPE_Y,

  MESH_BILLBOARD_TYPE_MOVE
};

enum EBillBoardType {
  BILLBOARD_TYPE_NONE,

  BILLBOARD_TYPE_ALL,
  BILLBOARD_TYPE_Y,

  BILLBOARD_TYPE_LIE, // 바닥에 누은 형상

  BILLBOARD_TYPE_2FACE, //     / and
  BILLBOARD_TYPE_3FACE, //     / and

  // BILLBOARD_TYPE_RAY, // 잔상

};

enum EMovingType {
  MOVING_TYPE_DIRECT,
  MOVING_TYPE_BEZIER_CURVE,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> struct CTimeEvent {
  typedef T value_type;

  float m_fTime;
  T m_Value;
};

template <typename T>
bool operator<(const CTimeEvent<T> &lhs, const CTimeEvent<T> &rhs) {
  return lhs.m_fTime < rhs.m_fTime;
}

template <typename T>
bool operator<(const CTimeEvent<T> &lhs, const float &rhs) {
  return lhs.m_fTime < rhs;
}

template <typename T>
bool operator<(const float &lhs, const CTimeEvent<T> &rhs) {
  return lhs < rhs.m_fTime;
}

typedef struct SEffectPosition : public CTimeEvent<D3DXVECTOR3> {
  // For Bezier Curve
  int m_iMovingType;
  D3DXVECTOR3 m_vecControlPoint;
} TEffectPosition;

typedef CTimeEvent<char> TTimeEventTypeCharacter;
typedef CTimeEvent<short> TTimeEventTypeShort;
typedef CTimeEvent<float> TTimeEventTypeFloat;
typedef CTimeEvent<WORD> TTimeEventTypeWord;
typedef CTimeEvent<DWORD> TTimeEventTypeDoubleWord;
typedef CTimeEvent<D3DXCOLOR> TTimeEventTypeColor;
typedef CTimeEvent<D3DXVECTOR2> TTimeEventTypeVector2;
typedef CTimeEvent<D3DXVECTOR3> TTimeEventTypeVector3;

typedef std::vector<float> TTimeEventTable;
typedef std::vector<TEffectPosition> TTimeEventTablePosition;
typedef std::vector<TTimeEventTypeCharacter> TTimeEventTableCharacter;
typedef std::vector<TTimeEventTypeShort> TTimeEventTableShort;
typedef std::vector<TTimeEventTypeFloat> TTimeEventTableFloat;
typedef std::vector<TTimeEventTypeWord> TTimeEventTableWord;
typedef std::vector<TTimeEventTypeDoubleWord> TTimeEventTableDoubleWord;
typedef std::vector<TTimeEventTypeColor> TTimeEventTableColor;
typedef std::vector<TTimeEventTypeVector2> TTimeEventTableVector2;
typedef std::vector<TTimeEventTypeVector3> TTimeEventTableVector3;

template <typename T>
T BlendSingleValue(float time, const CTimeEvent<T> &low,
                   const CTimeEvent<T> &high) {
  const float timeDiff = high.m_fTime - low.m_fTime;
  const float perc = (time - low.m_fTime) / timeDiff;

  const T valueDiff = high.m_Value - low.m_Value;
  return static_cast<T>(low.m_Value + perc * valueDiff);
}

inline D3DXVECTOR3 BlendSingleValue(float time, const TEffectPosition &low,
                                    const TEffectPosition &high) {
  const float timeDiff = high.m_fTime - low.m_fTime;
  const float perc = (time - low.m_fTime) / timeDiff;

  if (low.m_iMovingType == MOVING_TYPE_DIRECT)
    return low.m_Value + ((high.m_Value - low.m_Value) * perc);

  if (low.m_iMovingType == MOVING_TYPE_BEZIER_CURVE) {
    const float invPerc = 1.0f - perc;

    return low.m_Value * invPerc * invPerc +
           (low.m_Value + low.m_vecControlPoint) * invPerc * perc * 2.0f +
           high.m_Value * perc * perc;
  }

  // Unknown moving type - impossible(?)
  return D3DXVECTOR3();
}

template <typename T>
auto GetTimeEventBlendValue(float time, const std::vector<T> &vec) ->
    typename T::value_type {
  if (vec.empty())
    return typename T::value_type();

  // Single element is easy...
  if (vec.begin() + 1 == vec.end())
    return vec.front().m_Value;

  // All elements are greater than |time| - pick the smallest
  if (time < vec.front().m_fTime)
    return vec.front().m_Value;

  // All elements are smaller than |time| - pick the greatest
  if (time > vec.back().m_fTime)
    return vec.back().m_Value;

  // The two checks above make sure that result doesn't contain vec.end()
  // (We could simply check for vec.end() ourself, but above code lets us
  // skip std::equal_range() altogether, making it faster.)
  auto result = std::equal_range(vec.begin(), vec.end(), time);

  // We have one or more equal elements - pick the first
  if (result.first != result.second)
    return result.first->m_Value;

  // We need first to point to an element smaller than |time|
  // (Note that decrementing first is safe here, we already accounted for
  // vec.begin() being greater-or-equal to |time|.)
  --result.first;
  return BlendSingleValue(time, *result.first, *result.second);
}

extern BOOL GetTokenTimeEventFloat(CTextFileLoader &rTextFileLoader,
                                   const char *c_szKey,
                                   TTimeEventTableFloat *pTimeEventTableFloat);
// extern void InsertItemTimeEventFloat(TTimeEventTableFloat * pTable, float
// fTime, float fValue);

template <typename T>
void InsertItemTimeEvent(std::vector<CTimeEvent<T>> *pTable, float fTime,
                         T fValue) {
  typedef std::vector<CTimeEvent<T>>::iterator iterator;

  iterator itor = std::lower_bound(pTable->begin(), pTable->end(), fTime);

  CTimeEvent<T> TimeEvent;
  TimeEvent.m_fTime = fTime;
  TimeEvent.m_Value = fValue;

  pTable->insert(itor, TimeEvent);
}
