#pragma once
#include <granny.h>

void DeformPWNT3432toGrannyPNGBT33332(granny_int32x Count,
                                      void const *SourceInit, void *DestInit,
                                      granny_int32x const *TransformTable,
                                      granny_matrix_4x4 const *Transforms,
                                      granny_int32x SourceStride,
                                      granny_int32x DestStride);