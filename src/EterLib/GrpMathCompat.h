#pragma once

/******************************************************************************
 * GrpMathCompat.h - D3DX → DirectXMath compatibility layer
 *
 * Drop-in replacements for D3DX math functions using DirectXMath (no D3DX
 * dependency). This header defines D3DXMATRIX, D3DXVECTOR2/3/4, D3DXQUATERNION,
 * D3DXCOLOR as thin wrappers around DirectXMath storage types, preserving
 * binary layout.
 *
 * Phase 0 of the DX11 migration.
 ******************************************************************************/

#include <DirectXMath.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>
#include <windows.h>

// ============================================================================
// Type Definitions — Binary-compatible with D3DX types
// ============================================================================

#ifndef D3DX_PI
#define D3DX_PI 3.14159265358979323846f
#endif

#ifndef D3DX_FILTER_LINEAR
#define D3DX_FILTER_LINEAR 0x00000002
#endif

// Forward declare types that D3DX normally provides
#ifndef D3DXToRadian
inline float D3DXToRadian(float deg) { return deg * (D3DX_PI / 180.0f); }
#endif

#ifndef D3DXToDegree
inline float D3DXToDegree(float rad) { return rad * (180.0f / D3DX_PI); }
#endif

// ----------- D3DXVECTOR2 -----------
struct D3DXVECTOR2 {
  float x, y;

  D3DXVECTOR2() : x(0.0f), y(0.0f) {}
  D3DXVECTOR2(float _x, float _y) : x(_x), y(_y) {}

  D3DXVECTOR2 operator+(const D3DXVECTOR2 &rhs) const {
    return {x + rhs.x, y + rhs.y};
  }
  D3DXVECTOR2 operator-(const D3DXVECTOR2 &rhs) const {
    return {x - rhs.x, y - rhs.y};
  }
  D3DXVECTOR2 operator*(float s) const { return {x * s, y * s}; }
  D3DXVECTOR2 &operator+=(const D3DXVECTOR2 &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  D3DXVECTOR2 &operator-=(const D3DXVECTOR2 &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// ----------- D3DXVECTOR3 -----------
struct D3DXVECTOR3 {
  float x, y, z;

  D3DXVECTOR3() : x(0.0f), y(0.0f), z(0.0f) {}
  D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

  D3DXVECTOR3 operator+(const D3DXVECTOR3 &rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  D3DXVECTOR3 operator-(const D3DXVECTOR3 &rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }
  D3DXVECTOR3 operator*(float s) const { return {x * s, y * s, z * s}; }
  D3DXVECTOR3 operator/(float s) const { return {x / s, y / s, z / s}; }
  D3DXVECTOR3 operator-() const { return {-x, -y, -z}; }
  D3DXVECTOR3 &operator+=(const D3DXVECTOR3 &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
  D3DXVECTOR3 &operator-=(const D3DXVECTOR3 &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }
  D3DXVECTOR3 &operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// ----------- D3DXVECTOR4 -----------
struct D3DXVECTOR4 {
  float x, y, z, w;

  D3DXVECTOR4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
  D3DXVECTOR4(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
  D3DXVECTOR4(const D3DXVECTOR3 &v, float _w) : x(v.x), y(v.y), z(v.z), w(_w) {}

  D3DXVECTOR4 operator+(const D3DXVECTOR4 &rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
  }
  D3DXVECTOR4 operator-(const D3DXVECTOR4 &rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
  }
  D3DXVECTOR4 operator*(float s) const { return {x * s, y * s, z * s, w * s}; }
  D3DXVECTOR4 &operator+=(const D3DXVECTOR4 &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
  }

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// ----------- D3DXQUATERNION -----------
struct D3DXQUATERNION {
  float x, y, z, w;

  D3DXQUATERNION() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
  D3DXQUATERNION(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w) {}

  operator float *() { return &x; }
  operator const float *() const { return &x; }
};

// ----------- D3DXCOLOR -----------
struct D3DXCOLOR {
  float r, g, b, a;

  D3DXCOLOR() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
  D3DXCOLOR(float _r, float _g, float _b, float _a)
      : r(_r), g(_g), b(_b), a(_a) {}
  D3DXCOLOR(DWORD argb) {
    a = ((argb >> 24) & 0xFF) / 255.0f;
    r = ((argb >> 16) & 0xFF) / 255.0f;
    g = ((argb >> 8) & 0xFF) / 255.0f;
    b = ((argb) & 0xFF) / 255.0f;
  }

  operator DWORD() const {
    DWORD dwR = (DWORD)(r * 255.0f) & 0xFF;
    DWORD dwG = (DWORD)(g * 255.0f) & 0xFF;
    DWORD dwB = (DWORD)(b * 255.0f) & 0xFF;
    DWORD dwA = (DWORD)(a * 255.0f) & 0xFF;
    return (dwA << 24) | (dwR << 16) | (dwG << 8) | dwB;
  }

  operator float *() { return &r; }
  operator const float *() const { return &r; }

  D3DXCOLOR operator*(float s) const { return {r * s, g * s, b * s, a * s}; }
  D3DXCOLOR operator+(const D3DXCOLOR &rhs) const {
    return {r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a};
  }
  D3DXCOLOR operator-(const D3DXCOLOR &rhs) const {
    return {r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a};
  }
};

// ----------- D3DXMATRIX -----------
// 4x4 row-major matrix, binary-compatible with the original D3DXMATRIX
struct D3DXMATRIX {
  union {
    struct {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4];
  };

  D3DXMATRIX() { memset(m, 0, sizeof(m)); }

  D3DXMATRIX(float m11, float m12, float m13, float m14, float m21, float m22,
             float m23, float m24, float m31, float m32, float m33, float m34,
             float m41, float m42, float m43, float m44) {
    _11 = m11;
    _12 = m12;
    _13 = m13;
    _14 = m14;
    _21 = m21;
    _22 = m22;
    _23 = m23;
    _24 = m24;
    _31 = m31;
    _32 = m32;
    _33 = m33;
    _34 = m34;
    _41 = m41;
    _42 = m42;
    _43 = m43;
    _44 = m44;
  }

  float &operator()(int row, int col) { return m[row][col]; }
  float operator()(int row, int col) const { return m[row][col]; }

  operator float *() { return &_11; }
  operator const float *() const { return &_11; }

  D3DXMATRIX operator*(const D3DXMATRIX &rhs) const {
    D3DXMATRIX result;
    D3DXMatrixMultiply(&result, this, &rhs);
    return result;
  }

  D3DXMATRIX &operator*=(const D3DXMATRIX &rhs) {
    D3DXMATRIX tmp;
    D3DXMatrixMultiply(&tmp, this, &rhs);
    *this = tmp;
    return *this;
  }

  // Forward declaration used above — defined below
  static D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1,
                                        const D3DXMATRIX *pM2);
};

// ============================================================================
// Matrix Functions
// ============================================================================

inline D3DXMATRIX *D3DXMATRIX::D3DXMatrixMultiply(D3DXMATRIX *pOut,
                                                  const D3DXMATRIX *pM1,
                                                  const D3DXMATRIX *pM2) {
  using namespace DirectX;
  XMMATRIX m1 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM1));
  XMMATRIX m2 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM2));
  XMMATRIX result = XMMatrixMultiply(m1, m2);
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut), result);
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixIdentity(D3DXMATRIX *pOut) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut), XMMatrixIdentity());
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1,
                                      const D3DXMATRIX *pM2) {
  return D3DXMATRIX::D3DXMatrixMultiply(pOut, pM1, pM2);
}

inline D3DXMATRIX *D3DXMatrixTranspose(D3DXMATRIX *pOut, const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut), XMMatrixTranspose(mat));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDeterminant,
                                     const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMVECTOR det;
  XMMATRIX inv = XMMatrixInverse(&det, mat);
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut), inv);
  if (pDeterminant)
    *pDeterminant = XMVectorGetX(det);
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y,
                                         float z) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixTranslation(x, y, z));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy,
                                     float sz) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixScaling(sx, sy, sz));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationX(D3DXMATRIX *pOut, float angle) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixRotationX(angle));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationY(D3DXMATRIX *pOut, float angle) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixRotationY(angle));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *pOut, float angle) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixRotationZ(angle));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationYawPitchRoll(D3DXMATRIX *pOut, float yaw,
                                                  float pitch, float roll) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationQuaternion(D3DXMATRIX *pOut,
                                                const D3DXQUATERNION *pQ) {
  using namespace DirectX;
  XMVECTOR q = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pQ));
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixRotationQuaternion(q));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationAxis(D3DXMATRIX *pOut,
                                          const D3DXVECTOR3 *pV, float angle) {
  using namespace DirectX;
  XMVECTOR axis = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixRotationAxis(axis, angle));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixLookAtRH(D3DXMATRIX *pOut, const D3DXVECTOR3 *pEye,
                                      const D3DXVECTOR3 *pAt,
                                      const D3DXVECTOR3 *pUp) {
  using namespace DirectX;
  XMVECTOR eye = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pEye));
  XMVECTOR at = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pAt));
  XMVECTOR up = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pUp));
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixLookAtRH(eye, at, up));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixLookAtLH(D3DXMATRIX *pOut, const D3DXVECTOR3 *pEye,
                                      const D3DXVECTOR3 *pAt,
                                      const D3DXVECTOR3 *pUp) {
  using namespace DirectX;
  XMVECTOR eye = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pEye));
  XMVECTOR at = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pAt));
  XMVECTOR up = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pUp));
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixLookAtLH(eye, at, up));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixPerspectiveFovRH(D3DXMATRIX *pOut, float fovy,
                                              float aspect, float zn,
                                              float zf) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixPerspectiveFovRH(fovy, aspect, zn, zf));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixPerspectiveFovLH(D3DXMATRIX *pOut, float fovy,
                                              float aspect, float zn,
                                              float zf) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixPerspectiveFovLH(fovy, aspect, zn, zf));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixOrthoLH(D3DXMATRIX *pOut, float w, float h,
                                     float zn, float zf) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixOrthographicLH(w, h, zn, zf));
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixOrthoOffCenterLH(D3DXMATRIX *pOut, float l,
                                              float r, float b, float t,
                                              float zn, float zf) {
  using namespace DirectX;
  XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4 *>(pOut),
                  XMMatrixOrthographicOffCenterLH(l, r, b, t, zn, zf));
  return pOut;
}

// ============================================================================
// Vector Functions
// ============================================================================

inline float D3DXVec2Length(const D3DXVECTOR2 *pV) {
  return sqrtf(pV->x * pV->x + pV->y * pV->y);
}

inline float D3DXVec3Length(const D3DXVECTOR3 *pV) {
  return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

inline float D3DXVec3LengthSq(const D3DXVECTOR3 *pV) {
  return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
}

inline float D3DXVec3Dot(const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2) {
  return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

inline D3DXVECTOR3 *D3DXVec3Cross(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1,
                                  const D3DXVECTOR3 *pV2) {
  pOut->x = pV1->y * pV2->z - pV1->z * pV2->y;
  pOut->y = pV1->z * pV2->x - pV1->x * pV2->z;
  pOut->z = pV1->x * pV2->y - pV1->y * pV2->x;
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3Normalize(D3DXVECTOR3 *pOut,
                                      const D3DXVECTOR3 *pV) {
  float len = D3DXVec3Length(pV);
  if (len > 0.0f) {
    float invLen = 1.0f / len;
    pOut->x = pV->x * invLen;
    pOut->y = pV->y * invLen;
    pOut->z = pV->z * invLen;
  } else {
    pOut->x = pOut->y = pOut->z = 0.0f;
  }
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3Scale(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV,
                                  float s) {
  pOut->x = pV->x * s;
  pOut->y = pV->y * s;
  pOut->z = pV->z * s;
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3Add(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1,
                                const D3DXVECTOR3 *pV2) {
  pOut->x = pV1->x + pV2->x;
  pOut->y = pV1->y + pV2->y;
  pOut->z = pV1->z + pV2->z;
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3Subtract(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1,
                                     const D3DXVECTOR3 *pV2) {
  pOut->x = pV1->x - pV2->x;
  pOut->y = pV1->y - pV2->y;
  pOut->z = pV1->z - pV2->z;
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3Lerp(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1,
                                 const D3DXVECTOR3 *pV2, float s) {
  pOut->x = pV1->x + s * (pV2->x - pV1->x);
  pOut->y = pV1->y + s * (pV2->y - pV1->y);
  pOut->z = pV1->z + s * (pV2->z - pV1->z);
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *pOut,
                                           const D3DXVECTOR3 *pV,
                                           const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMVECTOR result = XMVector3TransformCoord(v, mat);
  XMStoreFloat3(reinterpret_cast<XMFLOAT3 *>(pOut), result);
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3TransformNormal(D3DXVECTOR3 *pOut,
                                            const D3DXVECTOR3 *pV,
                                            const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMVECTOR result = XMVector3TransformNormal(v, mat);
  XMStoreFloat3(reinterpret_cast<XMFLOAT3 *>(pOut), result);
  return pOut;
}

inline D3DXVECTOR4 *D3DXVec3Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR3 *pV,
                                      const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMVECTOR result = XMVector3Transform(v, mat);
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), result);
  return pOut;
}

inline D3DXVECTOR4 *D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV,
                                      const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMVECTOR v = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pV));
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMVECTOR result = XMVector4Transform(v, mat);
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), result);
  return pOut;
}

// D3DXVec3Project / Unproject — uses D3DVIEWPORT9 while DX9 headers are still
// included. Will be updated to D3D11_VIEWPORT in Phase 1.
template <typename TViewport>
inline D3DXVECTOR3 *
D3DXVec3Project(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV,
                const TViewport *pViewport, const D3DXMATRIX *pProjection,
                const D3DXMATRIX *pView, const D3DXMATRIX *pWorld) {
  using namespace DirectX;
  XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMMATRIX proj =
      pProjection
          ? XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pProjection))
          : XMMatrixIdentity();
  XMMATRIX view =
      pView ? XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pView))
            : XMMatrixIdentity();
  XMMATRIX world =
      pWorld ? XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pWorld))
             : XMMatrixIdentity();

  XMVECTOR result =
      XMVector3Project(v, (float)pViewport->X, (float)pViewport->Y,
                       (float)pViewport->Width, (float)pViewport->Height,
                       pViewport->MinZ, pViewport->MaxZ, proj, view, world);
  XMStoreFloat3(reinterpret_cast<XMFLOAT3 *>(pOut), result);
  return pOut;
}

template <typename TViewport>
inline D3DXVECTOR3 *
D3DXVec3Unproject(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV,
                  const TViewport *pViewport, const D3DXMATRIX *pProjection,
                  const D3DXMATRIX *pView, const D3DXMATRIX *pWorld) {
  using namespace DirectX;
  XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMMATRIX proj =
      pProjection
          ? XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pProjection))
          : XMMatrixIdentity();
  XMMATRIX view =
      pView ? XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pView))
            : XMMatrixIdentity();
  XMMATRIX world =
      pWorld ? XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pWorld))
             : XMMatrixIdentity();

  XMVECTOR result =
      XMVector3Unproject(v, (float)pViewport->X, (float)pViewport->Y,
                         (float)pViewport->Width, (float)pViewport->Height,
                         pViewport->MinZ, pViewport->MaxZ, proj, view, world);
  XMStoreFloat3(reinterpret_cast<XMFLOAT3 *>(pOut), result);
  return pOut;
}

// ============================================================================
// Quaternion Functions
// ============================================================================

inline D3DXQUATERNION *D3DXQuaternionRotationAxis(D3DXQUATERNION *pOut,
                                                  const D3DXVECTOR3 *pV,
                                                  float angle) {
  using namespace DirectX;
  XMVECTOR axis = XMLoadFloat3(reinterpret_cast<const XMFLOAT3 *>(pV));
  XMVECTOR q = XMQuaternionRotationAxis(axis, angle);
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), q);
  return pOut;
}

inline D3DXQUATERNION *D3DXQuaternionRotationMatrix(D3DXQUATERNION *pOut,
                                                    const D3DXMATRIX *pM) {
  using namespace DirectX;
  XMMATRIX mat = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4 *>(pM));
  XMVECTOR q = XMQuaternionRotationMatrix(mat);
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), q);
  return pOut;
}

inline D3DXQUATERNION *D3DXQuaternionSlerp(D3DXQUATERNION *pOut,
                                           const D3DXQUATERNION *pQ1,
                                           const D3DXQUATERNION *pQ2, float t) {
  using namespace DirectX;
  XMVECTOR q1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pQ1));
  XMVECTOR q2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pQ2));
  XMVECTOR result = XMQuaternionSlerp(q1, q2, t);
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), result);
  return pOut;
}

inline D3DXQUATERNION *D3DXQuaternionMultiply(D3DXQUATERNION *pOut,
                                              const D3DXQUATERNION *pQ1,
                                              const D3DXQUATERNION *pQ2) {
  using namespace DirectX;
  XMVECTOR q1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pQ1));
  XMVECTOR q2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pQ2));
  XMVECTOR result = XMQuaternionMultiply(q1, q2);
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), result);
  return pOut;
}

inline D3DXQUATERNION *D3DXQuaternionIdentity(D3DXQUATERNION *pOut) {
  using namespace DirectX;
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), XMQuaternionIdentity());
  return pOut;
}

inline D3DXQUATERNION *D3DXQuaternionNormalize(D3DXQUATERNION *pOut,
                                               const D3DXQUATERNION *pQ) {
  using namespace DirectX;
  XMVECTOR q = XMLoadFloat4(reinterpret_cast<const XMFLOAT4 *>(pQ));
  XMStoreFloat4(reinterpret_cast<XMFLOAT4 *>(pOut), XMQuaternionNormalize(q));
  return pOut;
}

// ============================================================================
// Plane Functions
// ============================================================================

struct D3DXPLANE {
  float a, b, c, d;
  D3DXPLANE() : a(0), b(0), c(0), d(0) {}
  D3DXPLANE(float _a, float _b, float _c, float _d)
      : a(_a), b(_b), c(_c), d(_d) {}
  operator float *() { return &a; }
  operator const float *() const { return &a; }
};

inline D3DXPLANE *D3DXPlaneFromPointNormal(D3DXPLANE *pOut,
                                           const D3DXVECTOR3 *pPoint,
                                           const D3DXVECTOR3 *pNormal) {
  pOut->a = pNormal->x;
  pOut->b = pNormal->y;
  pOut->c = pNormal->z;
  pOut->d = -(pNormal->x * pPoint->x + pNormal->y * pPoint->y +
              pNormal->z * pPoint->z);
  return pOut;
}

inline float D3DXPlaneDotCoord(const D3DXPLANE *pP, const D3DXVECTOR3 *pV) {
  return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z + pP->d;
}

// ============================================================================
// ID3DXMatrixStack replacement — simple stack using std::vector
// ============================================================================

class CD3DXMatrixStack {
public:
  CD3DXMatrixStack() {
    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    m_Stack.push_back(identity);
  }

  void Push() { m_Stack.push_back(m_Stack.back()); }

  void Pop() {
    if (m_Stack.size() > 1)
      m_Stack.pop_back();
  }

  void LoadIdentity() { D3DXMatrixIdentity(&m_Stack.back()); }

  void LoadMatrix(const D3DXMATRIX *pM) { m_Stack.back() = *pM; }

  void MultMatrix(const D3DXMATRIX *pM) {
    D3DXMatrixMultiply(&m_Stack.back(), &m_Stack.back(), pM);
  }

  void MultMatrixLocal(const D3DXMATRIX *pM) {
    D3DXMatrixMultiply(&m_Stack.back(), pM, &m_Stack.back());
  }

  void RotateAxis(const D3DXVECTOR3 *pV, float angle) {
    D3DXMATRIX mat;
    D3DXMatrixRotationAxis(&mat, pV, angle);
    MultMatrix(&mat);
  }

  void RotateAxisLocal(const D3DXVECTOR3 *pV, float angle) {
    D3DXMATRIX mat;
    D3DXMatrixRotationAxis(&mat, pV, angle);
    MultMatrixLocal(&mat);
  }

  void RotateYawPitchRoll(float yaw, float pitch, float roll) {
    D3DXMATRIX mat;
    D3DXMatrixRotationYawPitchRoll(&mat, yaw, pitch, roll);
    MultMatrix(&mat);
  }

  void RotateYawPitchRollLocal(float yaw, float pitch, float roll) {
    D3DXMATRIX mat;
    D3DXMatrixRotationYawPitchRoll(&mat, yaw, pitch, roll);
    MultMatrixLocal(&mat);
  }

  void Scale(float x, float y, float z) {
    D3DXMATRIX mat;
    D3DXMatrixScaling(&mat, x, y, z);
    MultMatrix(&mat);
  }

  void ScaleLocal(float x, float y, float z) {
    D3DXMATRIX mat;
    D3DXMatrixScaling(&mat, x, y, z);
    MultMatrixLocal(&mat);
  }

  void Translate(float x, float y, float z) {
    D3DXMATRIX mat;
    D3DXMatrixTranslation(&mat, x, y, z);
    MultMatrix(&mat);
  }

  void TranslateLocal(float x, float y, float z) {
    D3DXMATRIX mat;
    D3DXMatrixTranslation(&mat, x, y, z);
    MultMatrixLocal(&mat);
  }

  const D3DXMATRIX *GetTop() const { return &m_Stack.back(); }

  // COM-like interface for compatibility
  ULONG AddRef() { return ++m_RefCount; }
  ULONG Release() {
    if (--m_RefCount == 0) {
      delete this;
      return 0;
    }
    return m_RefCount;
  }

private:
  std::vector<D3DXMATRIX> m_Stack;
  ULONG m_RefCount = 1;
};

// Alias: in original D3DX, ID3DXMatrixStack is a COM interface.
// Existing code uses `ID3DXMatrixStack*` as a pointer to the interface.
// By typedef'ing the class name directly, `ID3DXMatrixStack*` =
// `CD3DXMatrixStack*`.
typedef CD3DXMatrixStack ID3DXMatrixStack;

inline HRESULT D3DXCreateMatrixStack(DWORD flags, ID3DXMatrixStack **ppStack) {
  (void)flags;
  *ppStack = new CD3DXMatrixStack();
  return S_OK;
}

// ============================================================================
// Color utility
// ============================================================================

inline D3DXCOLOR *D3DXColorLerp(D3DXCOLOR *pOut, const D3DXCOLOR *pC1,
                                const D3DXCOLOR *pC2, float s) {
  pOut->r = pC1->r + s * (pC2->r - pC1->r);
  pOut->g = pC1->g + s * (pC2->g - pC1->g);
  pOut->b = pC1->b + s * (pC2->b - pC1->b);
  pOut->a = pC1->a + s * (pC2->a - pC1->a);
  return pOut;
}
