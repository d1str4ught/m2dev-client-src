///////////////////////////////////////////////////////////////////////
//	SpeedTreeRT runtime configuration #defines
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com

#pragma once

const int c_nNumWindMatrices = 4;
const int c_nNumInstancesPerModel = 10;
const float c_fForestSize = 200.0f;
const float c_fSpacingTolerance = 30.0f;
const int c_nMaxPlacementIterations = 500;
const int c_nDefaultAlphaTestValue = 84;
const float c_fNearLodFactor = 2.0f;
const float c_fFarLodFactor = 9.0f;
const float c_fBenchmarkPeriod = 1.0f;

// vertex shader constant locations
const int c_nVertexShader_LeafLightingAdjustment = 70;
const int c_nVertexShader_Light = 71;
const int c_nVertexShader_Material = 74;
const int c_nVertexShader_TreePos = 52;
const int c_nVertexShader_CompoundMatrix = 0;
const int c_nVertexShader_WindMatrices = 54;
const int c_nVertexShader_LeafTables = 4;
const int c_nVertexShader_Fog = 85;

// lighting
const float c_afLightPosition[4] = {-0.707f, 0.0f, 0.707f, 0.0f};
const float c_afLightAmbient[4] = {0.5f, 0.5f, 0.5f, 1.0f};
const float c_afLightDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
const float c_afLightSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
const float c_afLightGlobalAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};

// setup lighting (enable ONE of the two below)
#define WRAPPER_USE_STATIC_LIGHTING
// #define WRAPPER_USE_DYNAMIC_LIGHTING

#if defined WRAPPER_USE_STATIC_LIGHTING && defined WRAPPER_USE_DYNAMIC_LIGHTING
#error Please define exactly one lighting mode
#endif

// setup wind (enable ONE of the three below)
// #define WRAPPER_USE_GPU_WIND
// #define WRAPPER_USE_CPU_WIND
#define WRAPPER_USE_NO_WIND

#if defined WRAPPER_USE_GPU_WIND && defined WRAPPER_USE_CPU_WIND
#error Please define exactly one lighting mode
#elif defined WRAPPER_USE_GPU_WIND && defined WRAPPER_USE_NO_WIND
#error Please define exactly one lighting mode
#elif defined WRAPPER_USE_CPU_WIND && defined WRAPPER_USE_NO_WIND
#error Please define exactly one lighting mode
#endif

// leaf placement algorithm (enable ONE of the two below)
#define WRAPPER_USE_GPU_LEAF_PLACEMENT
// #define WRAPPER_USE_CPU_LEAF_PLACEMENT

#if defined WRAPPER_USE_GPU_LEAF_PLACEMENT &&                                  \
    defined WRAPPER_USE_CPU_LEAF_PLACEMENT
#error Please define exactly one leaf placement algorithm
#endif

// texture coordinates (enable this define for DirectX-based engines)
#define WRAPPER_FLIP_T_TEXCOORD

// up vector
// #define WRAPPER_UP_POS_Y
#define WRAPPER_UP_POS_Z

#if defined WRAPPER_UP_POS_Y && defined WRAPPER_UP_POS_Z
#error Please define exactly one up vector
#endif

// loading from STF or clones/instances? (enable ONE of the two below)
// #define WRAPPER_FOREST_FROM_STF
#define WRAPPER_FOREST_FROM_INSTANCES

#if defined WRAPPER_FOREST_FROM_STF && defined WRAPPER_FOREST_FROM_INSTANCES
#error Please define exactly one loading mechanism
#endif

// billboard modes
#define WRAPPER_BILLBOARD_MODE
// #define WRAPPER_RENDER_HORIZONTAL_BILLBOARD

// render self-shadows
#define WRAPPER_RENDER_SELF_SHADOWS

// use fog
#define WRAPPER_USE_FOG

// derived constants
#ifdef WRAPPER_USE_GPU_WIND
#define BRANCHES_USE_SHADERS
#define FRONDS_USE_SHADERS
#define LEAVES_USE_SHADERS
#endif

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
#define LEAVES_USE_SHADERS
#endif
