#include "Deform.h"
#include <xmmintrin.h>
#include <emmintrin.h>

namespace {
constexpr float kInv255 = 1.0f / 255.0f;

inline void TransformPositionNormal(const float *matrix, const __m128 px,
                                    const __m128 py, const __m128 pz,
                                    const __m128 pw, const __m128 nx,
                                    const __m128 ny, const __m128 nz,
                                    __m128 &position, __m128 &normal) {
  const __m128 r0 = _mm_loadu_ps(matrix + 0);
  const __m128 r1 = _mm_loadu_ps(matrix + 4);
  const __m128 r2 = _mm_loadu_ps(matrix + 8);
  const __m128 r3 = _mm_loadu_ps(matrix + 12);

  position = _mm_add_ps(_mm_mul_ps(r0, px), _mm_mul_ps(r1, py));
  position = _mm_add_ps(position, _mm_mul_ps(r2, pz));
  position = _mm_add_ps(position, _mm_mul_ps(r3, pw));

  normal = _mm_add_ps(_mm_mul_ps(r0, nx), _mm_mul_ps(r1, ny));
  normal = _mm_add_ps(normal, _mm_mul_ps(r2, nz));
}

inline void AccumulateWeightedBone(__m128 &blendedPosition,
                                   __m128 &blendedNormal, const float *matrix,
                                   const granny_uint8 weight, const __m128 px,
                                   const __m128 py, const __m128 pz,
                                   const __m128 pw, const __m128 nx,
                                   const __m128 ny, const __m128 nz) {
  __m128 p;
  __m128 n;
  TransformPositionNormal(matrix, px, py, pz, pw, nx, ny, nz, p, n);

  const __m128 w = _mm_set1_ps(static_cast<float>(weight) * kInv255);
  blendedPosition = _mm_add_ps(blendedPosition, _mm_mul_ps(p, w));
  blendedNormal = _mm_add_ps(blendedNormal, _mm_mul_ps(n, w));
}

inline void StoreVec3(float *out, const __m128 v) {
  _mm_store_ss(out + 0, v);
  _mm_store_ss(out + 1, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)));
  _mm_store_ss(out + 2, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)));
}
} // namespace

void DeformPWNT3432toGrannyPNGBT33332D(granny_int32x Count,
                                       void const *SourceInit, void *DestInit,
                                       granny_matrix_4x4 const *Transforms,
                                       granny_int32x SourceStride,
                                       granny_int32x DestStride) {
  const granny_pwnt3432_vertex *src =
      (const granny_pwnt3432_vertex *)SourceInit;
  granny_pnt332_vertex *dst = (granny_pnt332_vertex *)DestInit;
  const __m128 pw = _mm_set1_ps(1.0f);

  while (Count--) {
    const __m128 px = _mm_set1_ps(src->Position[0]);
    const __m128 py = _mm_set1_ps(src->Position[1]);
    const __m128 pz = _mm_set1_ps(src->Position[2]);

    const __m128 nx = _mm_set1_ps(src->Normal[0]);
    const __m128 ny = _mm_set1_ps(src->Normal[1]);
    const __m128 nz = _mm_set1_ps(src->Normal[2]);

    const granny_uint8 w0 = src->BoneWeights[0];
    const granny_uint8 w1 = src->BoneWeights[1];
    const granny_uint8 w2 = src->BoneWeights[2];
    const granny_uint8 w3 = src->BoneWeights[3];

    __m128 P;
    __m128 N;

    if (w0 == 255 && ((w1 | w2 | w3) == 0)) {
      const float *m = (const float *)(&Transforms[src->BoneIndices[0]]);
      TransformPositionNormal(m, px, py, pz, pw, nx, ny, nz, P, N);

      const __m128 rigidWeight = _mm_set1_ps(static_cast<float>(w0) * kInv255);
      P = _mm_mul_ps(P, rigidWeight);
      N = _mm_mul_ps(N, rigidWeight);
    } else {
      P = _mm_setzero_ps();
      N = _mm_setzero_ps();

      if (w0) {
        const float *m = (const float *)(&Transforms[src->BoneIndices[0]]);
        AccumulateWeightedBone(P, N, m, w0, px, py, pz, pw, nx, ny, nz);
      }
      if (w1) {
        const float *m = (const float *)(&Transforms[src->BoneIndices[1]]);
        AccumulateWeightedBone(P, N, m, w1, px, py, pz, pw, nx, ny, nz);
      }
      if (w2) {
        const float *m = (const float *)(&Transforms[src->BoneIndices[2]]);
        AccumulateWeightedBone(P, N, m, w2, px, py, pz, pw, nx, ny, nz);
      }
      if (w3) {
        const float *m = (const float *)(&Transforms[src->BoneIndices[3]]);
        AccumulateWeightedBone(P, N, m, w3, px, py, pz, pw, nx, ny, nz);
      }
    }

    StoreVec3(dst->Position, P);
    StoreVec3(dst->Normal, N);

    dst->UV[0] = src->UV[0];
    dst->UV[1] = src->UV[1];

    src = (const granny_pwnt3432_vertex *)((const granny_uint8 *)src +
                                           SourceStride);
    dst = (granny_pnt332_vertex *)((granny_uint8 *)dst + DestStride);
  }
}

void DeformPWNT3432toGrannyPNGBT33332I(granny_int32x Count,
                                       void const *SourceInit, void *DestInit,
                                       granny_int32x const *TransformTable,
                                       granny_matrix_4x4 const *Transforms,
                                       granny_int32x SourceStride,
                                       granny_int32x DestStride) {
  const granny_pwnt3432_vertex *src =
      (const granny_pwnt3432_vertex *)SourceInit;
  granny_pnt332_vertex *dst = (granny_pnt332_vertex *)DestInit;
  const __m128 pw = _mm_set1_ps(1.0f);

  while (Count--) {
    const __m128 px = _mm_set1_ps(src->Position[0]);
    const __m128 py = _mm_set1_ps(src->Position[1]);
    const __m128 pz = _mm_set1_ps(src->Position[2]);

    const __m128 nx = _mm_set1_ps(src->Normal[0]);
    const __m128 ny = _mm_set1_ps(src->Normal[1]);
    const __m128 nz = _mm_set1_ps(src->Normal[2]);

    const granny_uint8 w0 = src->BoneWeights[0];
    const granny_uint8 w1 = src->BoneWeights[1];
    const granny_uint8 w2 = src->BoneWeights[2];
    const granny_uint8 w3 = src->BoneWeights[3];

    __m128 P;
    __m128 N;

    if (w0 == 255 && ((w1 | w2 | w3) == 0)) {
      const int bi = TransformTable[src->BoneIndices[0]];
      const float *m = (const float *)(&Transforms[bi]);
      TransformPositionNormal(m, px, py, pz, pw, nx, ny, nz, P, N);

      const __m128 rigidWeight = _mm_set1_ps(static_cast<float>(w0) * kInv255);
      P = _mm_mul_ps(P, rigidWeight);
      N = _mm_mul_ps(N, rigidWeight);
    } else {
      P = _mm_setzero_ps();
      N = _mm_setzero_ps();

      if (w0) {
        const int bi = TransformTable[src->BoneIndices[0]];
        const float *m = (const float *)(&Transforms[bi]);
        AccumulateWeightedBone(P, N, m, w0, px, py, pz, pw, nx, ny, nz);
      }
      if (w1) {
        const int bi = TransformTable[src->BoneIndices[1]];
        const float *m = (const float *)(&Transforms[bi]);
        AccumulateWeightedBone(P, N, m, w1, px, py, pz, pw, nx, ny, nz);
      }
      if (w2) {
        const int bi = TransformTable[src->BoneIndices[2]];
        const float *m = (const float *)(&Transforms[bi]);
        AccumulateWeightedBone(P, N, m, w2, px, py, pz, pw, nx, ny, nz);
      }
      if (w3) {
        const int bi = TransformTable[src->BoneIndices[3]];
        const float *m = (const float *)(&Transforms[bi]);
        AccumulateWeightedBone(P, N, m, w3, px, py, pz, pw, nx, ny, nz);
      }
    }

    StoreVec3(dst->Position, P);
    StoreVec3(dst->Normal, N);

    dst->UV[0] = src->UV[0];
    dst->UV[1] = src->UV[1];

    src = (const granny_pwnt3432_vertex *)((const granny_uint8 *)src +
                                           SourceStride);
    dst = (granny_pnt332_vertex *)((granny_uint8 *)dst + DestStride);
  }
}

void DeformPWNT3432toGrannyPNGBT33332(granny_int32x Count,
                                      void const *SourceInit, void *DestInit,
                                      granny_int32x const *TransformTable,
                                      granny_matrix_4x4 const *Transforms,
                                      granny_int32x SourceStride,
                                      granny_int32x DestStride) {
  if (TransformTable) [[likely]] {
    DeformPWNT3432toGrannyPNGBT33332I(Count, SourceInit, DestInit,
                                      TransformTable, Transforms, SourceStride,
                                      DestStride);
  } else [[unlikely]] {
    DeformPWNT3432toGrannyPNGBT33332D(Count, SourceInit, DestInit, Transforms,
                                      SourceStride, DestStride);
  }
}
