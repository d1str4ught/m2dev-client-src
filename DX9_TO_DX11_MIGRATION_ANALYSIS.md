# Metin2 Client: DX9 → DX11 Full Migration Analysis

> **Generated:** 2026-03-05  
> **Purpose:** Complete state analysis of the DX9→DX11 migration with annotated problems and solutions to avoid repeating mistakes.

---

## Table of Contents

1. [Current State Summary](#1-current-state-summary)
2. [Architecture Overview](#2-architecture-overview)
3. [Existing DX11 Work (Phase 1)](#3-existing-dx11-work-phase-1)
4. [Complete DX9 API Surface Area](#4-complete-dx9-api-surface-area)
5. [Problems & Solutions (Annotated)](#5-problems--solutions-annotated)
6. [Migration Phases](#6-migration-phases)
7. [File-by-File Impact Matrix](#7-file-by-file-impact-matrix)
8. [Build System Changes](#8-build-system-changes)

---

## 1. Current State Summary

The Metin2 client is a **DX9 application** with **bolt-on DX11 systems** added as a Phase 1 overlay. The actual scene rendering (geometry, textures, effects, terrain, SpeedTree, UI) still flows **100% through DX9**. The DX11 systems that exist are post-processing overlays that take the DX9 rendered frame and process it.

### What Works (DX11)
- DX11 device creation alongside DX9 device
- `CDX11StateCache` — translates DX9 render states to DX11 immutable state objects
- `CDX11ShaderManager` — FFP emulation shaders for PDT vertex format
- `CDX11PostProcess` — bloom/tone mapping post-processing pipeline
- `CDX11ShadowMap` — 2048×2048 shadow depth map for directional light
- `CDX11ComputeParticles` — GPU particle system (header only, compute shaders)
- `GrpMathCompat.h` — D3DX math replacement using DirectXMath (788 lines, very complete)
- `CGraphicVertexBuffer` / `CGraphicIndexBuffer` / `CGraphicTexture` — have DX11 member pointers (`m_pDX11Buffer`, `m_pDX11SRV`) but **no implementation**

### What Does NOT Work Yet
- **ALL actual draw calls** still go through DX9 (`STATEMANAGER.DrawPrimitive(...)`)
- Textures are loaded as `LPDIRECT3DTEXTURE9` — no DX11 texture loading path
- Vertex/Index buffer creation is DX9 only — DX11 buffer members are `nullptr`
- Fixed-function pipeline (texture stage states, transforms) has no DX11 equivalent beyond PDT shader
- The DX9 device (`ms_lpd3dDevice`) is the one used for all rendering
- Presentation is still DX9 `Present()`, not DX11 swap chain

---

## 2. Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        UserInterface                             │
│   PythonApplication.cpp (main loop), PythonMiniMap, etc.         │
├─────────────────────────────────────────────────────────────────┤
│  EterPythonLib          │  GameLib               │  EffectLib    │
│  PythonGraphic.cpp      │  MapOutdoor*.cpp       │  Particles    │
│  PythonWindow.cpp       │  ActorInstance.cpp     │  EffectMesh   │
│                         │  FlyTrace, WeaponTrace │               │
├─────────────────────────┼────────────────────────┼───────────────┤
│  SpeedTreeLib           │  EterGrnLib            │  PRTerrainLib │
│  SpeedTreeWrapper.cpp   │  Model/Material        │  Terrain.h    │
│  SpeedTreeForest.cpp    │  ModelInstanceRender   │  TextureSet.h │
├─────────────────────────┴────────────────────────┴───────────────┤
│                          EterLib (CORE GRAPHICS)                  │
│  ┌─────────────────┐  ┌──────────────────┐  ┌─────────────────┐ │
│  │  CGraphicBase   │  │  CStateManager   │  │  CGraphicDevice │ │
│  │  (static DX9+   │  │  (ALL DX9 calls  │  │  (device create │ │
│  │   DX11 ptrs)    │  │   flow through)  │  │   + DX11 init)  │ │
│  └─────────────────┘  └──────────────────┘  └─────────────────┘ │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐   │
│  │ GrpTexture   │  │ GrpVtxBuf    │  │ GrpScreen/Image/Text │   │
│  │ GrpImageTex  │  │ GrpIdxBuf    │  │ SkyBox/LensFlare     │   │
│  └──────────────┘  └──────────────┘  └──────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  DX11 Bolt-on Systems (Phase 1 — currently overlays)     │   │
│  │  DX11StateCache | DX11ShaderManager | DX11PostProcess    │   │
│  │  DX11ShadowMap  | DX11ComputeParticles                   │   │
│  └──────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│  EterImageLib (DDS loading) │  EterBase (debug/utils)            │
└─────────────────────────────────────────────────────────────────┘
```

### Key Singleton: `CStateManager`

**Every** DX9 API call in the codebase flows through `STATEMANAGER` macro, which resolves to `CStateManager::Instance()`. This is the **single most important class** for migration. Functions it wraps:
- `SetRenderState()` — 30+ files
- `SetTextureStageState()` — 27+ files  
- `SetTransform()` — 21+ files
- `SetTexture()` — all files with textures
- `DrawPrimitive()` / `DrawPrimitiveUP()` — 15+ files
- `DrawIndexedPrimitive()` / `DrawIndexedPrimitiveUP()` — 20+ files
- `SetStreamSource()` / `SetIndices()` — buffer binding
- `SetSamplerState()` — sampler configuration
- `SetMaterial()` — material management
- `BeginScene()` / `EndScene()` — frame management

---

## 3. Existing DX11 Work (Phase 1)

### 3.1 Files Created

| File | Purpose | Status |
|------|---------|--------|
| `DX11StateCache.h/.cpp` | Translates DX9 render states → DX11 immutable state objects | ✅ Complete, tested |
| `DX11ShaderManager.h/.cpp` | FFP emulation shaders (PDT, PNT variants) + constant buffers | ✅ Header + impl |
| `DX11PostProcess.h/.cpp` | Bloom, tone mapping post-process pipeline | ✅ Complete |
| `DX11ShadowMap.h/.cpp` | 2048×2048 directional shadow depth map | ✅ Complete |
| `DX11ComputeParticles.h` | GPU compute particle system | ⚠️ Header only |
| `GrpMathCompat.h` | D3DX → DirectXMath drop-in replacements | ✅ 788 lines, comprehensive |

### 3.2 Modifications to Existing Files

| File | What Changed |
|------|--------------|
| `StdAfx.h` | Added `#include <d3d11.h>`, `<d3dcompiler.h>`, `<dxgi.h>` |
| `GrpBase.h` | Forward declarations for DX11 types; added DX11 static members to `CGraphicBase` |
| `GrpDevice.h/.cpp` | Added `__CreateDX11Device()`, `__CreateDX11DepthStencil()`, `__DestroyDX11Device()` |
| `GrpVertexBuffer.h` | Added `ID3D11Buffer* m_pDX11Buffer` member and `GetDX11Buffer()` accessor |
| `GrpIndexBuffer.h` | Added `ID3D11Buffer* m_pDX11Buffer` member and `GetDX11Buffer()` accessor |
| `GrpTexture.h` | Added `ID3D11ShaderResourceView* m_pDX11SRV`, DX11 texture registry (static map) |

### 3.3 What Phase 1 Actually Does at Runtime

The DX11 systems are initialized alongside DX9 during device creation. During frame rendering:
1. DX9 renders the entire scene normally via `ms_lpd3dDevice`
2. The rendered DX9 back buffer is (supposed to be) copied to a shared DX11 texture
3. `CDX11PostProcess::ApplyAndPresent()` applies bloom/tone-mapping
4. DX11 swap chain presents the post-processed result

**Critical issue:** The DX9→DX11 frame bridge (shared surface copy) is the bottleneck. This works as a transitional approach but is NOT the final goal. The goal is to eliminate DX9 entirely.

---

## 4. Complete DX9 API Surface Area

### 4.1 Core D3D9 Types Still in Use

| Type | Used In | Count |
|------|---------|-------|
| `LPDIRECT3D9EX` | `GrpBase.h` (static `ms_lpd3d`) | 1 global |
| `LPDIRECT3DDEVICE9EX` | `GrpBase.h` (static `ms_lpd3dDevice`), `StateManager` | 2 globals |
| `LPDIRECT3DTEXTURE9` | 27+ files across all rendering libs | Deep embedding |
| `LPDIRECT3DVERTEXBUFFER9` | `GrpVertexBuffer.h`, `StateManager`, `GrpBase.h`, `SpeedTreeLib`, `GameLib` | 14+ files |
| `LPDIRECT3DINDEXBUFFER9` | `GrpIndexBuffer.h`, `StateManager`, `GrpBase.h` | 6+ files |
| `LPDIRECT3DSURFACE9` | `GrpDevice.cpp`, `GrpShadowTexture`, `GrpImageTexture` | 5+ files |
| `LPDIRECT3DVERTEXDECLARATION9` | `GrpBase.h`, `GrpDevice.cpp` | 4 declarations |
| `D3DPRESENT_PARAMETERS` | `GrpBase.h`, `GrpDevice.cpp` | 2 |
| `D3DCAPS9` | `GrpBase.h`, `GrpDevice.cpp` | 2 |
| `D3DVIEWPORT9` | `GrpBase.h`, `GrpBase.cpp` | 2 |
| `D3DMATERIAL9` | `StateManager.h` | 1 |
| `LPD3DXMESH` | `GrpBase.h` (debug meshes) | 2 |
| `ID3DXMatrixStack` | `GrpBase.h` | 1 |

### 4.2 D3D9 Functions Called (via StateManager)

| Function | Files Using It |
|----------|---------------|
| `DrawPrimitive` | `GrpScreen`, `PythonGraphic`, `MapOutdoorWater`, `SkyBox`, `SpeedTree`, `LensFlare` |
| `DrawPrimitiveUP` | `GrpTextInstance`, `EffectMesh`, `ParticleSystem`, `FlyTrace`, `WeaponTrace`, `SnowEnvironment`, `LensFlare`, `SpeedTree` |
| `DrawIndexedPrimitive` | `GrpScreen`, `GrpImageInstance`, `GrpMarkInstance`, `GrpExpandedImageInstance`, `BlockTexture`, `PythonMiniMap`, `PythonGraphic`, `MapOutdoor*`, `SpeedTree`, `SnowEnvironment`, `Decal`, `SkyBox`, `ModelInstanceRender` |
| `SetRenderState` | 30+ files (virtually every rendering file) |
| `SetTextureStageState` | 27+ files (all multi-texture/blend operations) |
| `SetTransform` | 21+ files (world/view/projection matrix setup) |
| `SetTexture` | All files that bind textures |
| `SetSamplerState` | All files with filtered textures |
| `SetStreamSource` | Buffer binding sites |
| `SetIndices` | Index buffer binding sites |

### 4.3 D3DX Library Dependencies

| D3DX Function/Type | Used In | DX11 Replacement |
|-------|---------|-------------------|
| `D3DXVECTOR2/3/4` | **Everywhere** (~100+ files) | ✅ `GrpMathCompat.h` provides drop-in |
| `D3DXMATRIX` | **Everywhere** (~50+ files) | ✅ `GrpMathCompat.h` provides drop-in |
| `D3DXQUATERNION` | Animations, model instances | ✅ `GrpMathCompat.h` provides drop-in |
| `D3DXCOLOR` | Color operations | ✅ `GrpMathCompat.h` provides drop-in |
| `D3DXCreateTextureFromFileInMemoryEx` | `GrpImageTexture.cpp` | WIC/DDSTextureLoader11 |
| `D3DXCreateMeshFVF` | `GrpDevice.cpp` (debug spheres) | Custom vertex buffers |
| `D3DXMatrixMultiply/Inverse/etc` | Math operations throughout | ✅ `GrpMathCompat.h` |
| `ID3DXMatrixStack` | `GrpBase.h` — world matrix stack | Custom matrix stack impl |
| `D3DXVec3TransformCoord` | Collision, picking | ✅ `GrpMathCompat.h` |
| `D3DXIntersectTri` | Terrain picking | ✅ `GrpMathCompat.h` |
| `ID3DXEffect` | NOT used (no .fx files) | N/A |

### 4.4 DDSTextureLoader

`EterImageLib/DDSTextureLoader9.h/.cpp` — currently loads DDS textures for DX9. Needs DX11 version (`DDSTextureLoader11`), which is available from DirectXTex/DirectXTK.

---

## 5. Problems & Solutions (Annotated)

> [!IMPORTANT]  
> These are the key problems that **will** be encountered during migration. Each has an annotated solution to prevent wasted effort.

---

### ⚠️ PROBLEM 1: Fixed-Function Pipeline Does Not Exist in DX11

**What:** DX9 uses the fixed-function pipeline extensively via `SetTextureStageState()` (27+ files). DX11 has **no** fixed-function pipeline — everything must be shaders.

**Where it manifests:**
- `D3DTSS_COLOROP`, `D3DTSS_ALPHAOP` — texture blending operations
- `D3DTSS_COLORARG1/2`, `D3DTSS_ALPHAARG1/2` — blend arguments
- Multi-texture blending (terrain splatting, environment mapping)
- `SetTransform(D3DTS_TEXTURE0, ...)` — texture coordinate transforms

**Solution:** The existing `CDX11ShaderManager` has only 1 shader variant (FFP_PDT). It must be expanded to cover ALL combinations used in the codebase:

| DX9 FFP Mode | Shader Variant Needed |
|--------------|----------------------|
| Single texture + diffuse color | `FFP_PDT` (exists) |
| Single texture + vertex normal + lighting | `FFP_PNT` |  
| Dual texture blending (terrain) | `FFP_PNT2` |
| Position + diffuse only (no texture) | `FFP_PD` |
| Position only (wireframe/debug) | `FFP_POS` |
| Alpha test (discard in pixel shader) | Material cbuffer `fAlphaRef` (exists) |
| Fog | Material cbuffer `fFogEnable` (exists) |
| Sphere mapping / env map | Dedicated env-map shader |
| Texture coordinate transform | Shader variant with mat transform |

**Key insight:** Instead of trying to emulate every `SetTextureStageState` combo, create ~8-10 shader variants and route through `CDX11ShaderManager::BindForFVF()` + material flags.

---

### ⚠️ PROBLEM 2: CStateManager is the Migration Bottleneck

**What:** `CStateManager` wraps **every** DX9 call. It stores a `LPDIRECT3DDEVICE9EX m_lpD3DDev` and calls it directly in `DrawPrimitive()`, `SetRenderState()`, etc.

**Solution: Dual-path StateManager**

The cleanest approach is to add a **DX11 backend** inside `CStateManager` itself:

```cpp
// CStateManager — add DX11 mode
class CStateManager {
    // Existing DX9 path
    LPDIRECT3DDEVICE9EX m_lpD3DDev;
    
    // NEW: DX11 path
    bool m_bUseDX11;  // runtime switch
    CDX11StateCache* m_pDX11StateCache;
    CDX11ShaderManager* m_pDX11ShaderManager;
    
    HRESULT DrawPrimitive(...) {
        if (m_bUseDX11) return DrawPrimitive_DX11(...);
        else return DrawPrimitive_DX9(...);
    }
};
```

**Why this works:** No calling code changes. Every site that calls `STATEMANAGER.DrawPrimitive(...)` continues to work. The switch happens inside the StateManager.

**Risk:** Performance overhead of the `if` check on every call. Mitigate by making the flag a compile-time `#ifdef` once DX9 is fully retired.

---

### ⚠️ PROBLEM 3: DrawPrimitiveUP Has No DX11 Equivalent

**What:** DX9's `DrawPrimitiveUP()` accepts raw CPU vertex data and draws immediately. DX11 has **no** equivalent — all vertex data must be in GPU buffers.

**Where:** 15+ call sites across `GrpTextInstance`, `EffectMesh`, `ParticleSystem`, `FlyTrace`, `WeaponTrace`, `SnowEnvironment`, `LensFlare`, `SpeedTree`.

**Solution:** Create a dynamic vertex buffer pool:

```cpp
class CDynamicVBPool {
    ID3D11Buffer* m_buffers[POOL_SIZE];
    int m_currentIndex;
    
    // Map CPU data → GPU, draw, advance ring buffer
    void DrawImmediate(const void* pVertices, UINT stride, 
                       UINT vertexCount, D3DPRIMITIVETYPE type);
};
```

**Key insight:** Use a ring buffer of `D3D11_USAGE_DYNAMIC` buffers with `Map(D3D11_MAP_WRITE_DISCARD)`. This is the standard pattern for UP-style draws in DX11. Size the ring buffer to handle worst-case per-frame UP calls.

---

### ⚠️ PROBLEM 4: Texture Loading Pipeline

**What:** All textures are loaded as `LPDIRECT3DTEXTURE9` via `D3DXCreateTextureFromFileInMemoryEx()` in `GrpImageTexture.cpp`. Textures are bound via `SetTexture(stage, pTex)`.

**Solution:**

1. **Replace `DDSTextureLoader9`** with `DDSTextureLoader11` from DirectXTK
2. **Create dual-loading path** in `CGraphicImageTexture`:
   - Load DX9 texture (existing path)
   - ALSO create `ID3D11Texture2D` + `ID3D11ShaderResourceView` from same image data
   - Store both in the texture object (members already exist: `m_pDX11SRV`, `m_pDX11Texture`)
3. **Texture registry** (already in `CGraphicTexture`) maps DX9 tex → DX11 SRV
4. Once DX9 is removed, remove the DX9 loading path

**Alternative:** Skip the dual-loading phase. Load directly as DX11 textures and remove DX9 tex loading entirely. This is cleaner but means DX9 fallback breaks immediately.

---

### ⚠️ PROBLEM 5: Vertex/Index Buffer Creation

**What:** `CGraphicVertexBuffer::Create()` creates a `LPDIRECT3DVERTEXBUFFER9` using `CreateVertexBuffer()`. The DX11 member `m_pDX11Buffer` exists but is never populated.

**Same for** `CGraphicIndexBuffer::Create()`.

**Solution:** In `Create()`, after creating the DX9 buffer, ALSO create the DX11 buffer:

```cpp
bool CGraphicVertexBuffer::Create(int vtxCount, DWORD fvf, DWORD usage, D3DPOOL pool) {
    // Existing DX9 creation...
    
    // NEW: DX11 buffer creation
    if (ms_pD3D11Device) {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = m_dwBufferSize;
        desc.Usage = (usage & D3DUSAGE_DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = (usage & D3DUSAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        ms_pD3D11Device->CreateBuffer(&desc, nullptr, &m_pDX11Buffer);
    }
}
```

**Key issue:** `Lock()`/`Unlock()` must also update the DX11 buffer. Use `Map()`/`Unmap()` for dynamic, `UpdateSubresource()` for default.

---

### ⚠️ PROBLEM 6: D3DX Math Library Removal (d3dx9.h)

**What:** `StdAfx.h` includes `<d3dx9.h>` which provides `D3DXVECTOR3`, `D3DXMATRIX`, `D3DXMatrixMultiply`, etc. This header requires the deprecated D3DX runtime DLLs.

**Solution:** ✅ **ALREADY SOLVED** — `GrpMathCompat.h` (788 lines) provides complete drop-in replacements for all D3DX math types and functions using DirectXMath internally.

**To complete:**
1. Remove `#include <d3dx9.h>` from `StdAfx.h`
2. Add `#include "GrpMathCompat.h"` before any file that uses D3DX types
3. Verify compilation — the compat header defines the same type names

**Gotcha:** `LPD3DXMESH` (used in `GrpBase.h` for debug sphere/cylinder) has NO compat replacement. Must be replaced with custom vertex buffer geometry or DirectXTK `GeometricPrimitive`.

---

### ⚠️ PROBLEM 7: ID3DXMatrixStack Replacement

**What:** `GrpBase.h` uses `ID3DXMatrixStack* ms_lpd3dMatStack` for push/pop/multiply world matrix operations. This is a D3DX utility that doesn't exist in DX11.

**Solution:** Write a simple replacement:

```cpp
class CMatrixStack {
    std::vector<D3DXMATRIX> m_stack;
public:
    void Push() { m_stack.push_back(m_stack.back()); }
    void Pop() { m_stack.pop_back(); }
    void LoadMatrix(const D3DXMATRIX& m) { m_stack.back() = m; }
    void MultMatrix(const D3DXMATRIX& m) { D3DXMatrixMultiply(&m_stack.back(), &m_stack.back(), &m); }
    const D3DXMATRIX& GetTop() const { return m_stack.back(); }
};
```

This is trivial but must be done before removing `d3dx9.h`.

---

### ⚠️ PROBLEM 8: Vertex Declarations vs Input Layouts

**What:** DX9 uses `LPDIRECT3DVERTEXDECLARATION9` (or FVF codes) to describe vertex formats. DX11 uses `ID3D11InputLayout` which is **tied to a specific vertex shader**.

**Where:** `GrpDevice.cpp` creates 4 vertex declarations:
- `CreatePTStreamVertexShader()` — Position + TexCoord
- `CreatePNTStreamVertexShader()` — Position + Normal + TexCoord
- `CreatePNT2StreamVertexShader()` — Position + Normal + 2×TexCoord
- `CreateDoublePNTStreamVertexShader()` — Double stream PNT

**Solution:** Each shader variant in `CDX11ShaderManager` already creates its own input layout. The vertex declarations in `GrpDevice.cpp` become unnecessary for DX11. The mapping is:

| DX9 Vertex Decl | DX11 Input Layout | Shader Variant |
|------------------|-------------------|----------------|
| `ms_ptVS` | Part of FFP_PT shader | `FFP_PT` |
| `ms_pntVS` | Part of FFP_PNT shader | `FFP_PNT` |
| `ms_pnt2VS` | Part of FFP_PNT2 shader | `FFP_PNT2` |

---

### ⚠️ PROBLEM 9: SpeedTreeLib DirectX Integration

**What:** SpeedTree has deep DX9 integration. `SpeedTreeWrapper.cpp` and `SpeedTreeForestDirectX.cpp` directly use:
- `LPDIRECT3DTEXTURE9` for leaf/branch textures
- `LPDIRECT3DVERTEXBUFFER9` for tree geometry
- `SetRenderState()` for alpha test, blending
- `SetTextureStageState()` for leaf blending
- `DrawPrimitive()` and `DrawIndexedPrimitive()` for geometry
- Custom vertex shaders (`LPDIRECT3DVERTEXDECLARATION9`)

**Solution:** SpeedTree is an isolated subsystem. Two approaches:

1. **Wrap approach:** Create `SpeedTreeDX11Adapter` that provides DX11 draw calls replacing the DX9 calls in both files. This keeps SpeedTree code changes minimal.

2. **Rewrite approach:** Port SpeedTree rendering to DX11 shaders (wind animation must be in vertex shader). This gives better results but is more work.

**Recommendation:** Use approach #1 first (wrap), then optimize later. The SpeedTree geometry is standard vertex buffers that can use the `CDX11ShaderManager` shader variants.

---

### ⚠️ PROBLEM 10: Terrain Rendering (MapOutdoor*)

**What:** The terrain system is the most complex rendering subsystem. Files involved:
- `MapOutdoorRender.cpp` — main terrain patches
- `MapOutdoorRenderSTP.cpp` — shadow texture passes  
- `MapOutdoorRenderHTP.cpp` — heightmap texture passes
- `MapOutdoorWater.cpp` — water rendering
- `MapOutdoorCharacterShadow.cpp` — character shadow projection

These use multi-pass rendering with complex texture stage state configurations for terrain splatting (blending up to 8 terrain textures).

**Solution:** Terrain splatting shader:

```hlsl
// Terrain splatting pixel shader
Texture2D texLayers[8] : register(t0);
Texture2D texSplat : register(t8);  // blend weights
SamplerState samLinear : register(s0);

float4 PS_Terrain(VS_OUT input) : SV_Target {
    float4 splatWeights = texSplat.Sample(samLinear, input.texCoord);
    float4 color = texLayers[0].Sample(samLinear, input.detailUV) * splatWeights.r
                 + texLayers[1].Sample(samLinear, input.detailUV) * splatWeights.g
                 + texLayers[2].Sample(samLinear, input.detailUV) * splatWeights.b
                 + texLayers[3].Sample(samLinear, input.detailUV) * splatWeights.a;
    return color;
}
```

The terrain should get its own dedicated shader rather than trying to emulate the multi-pass FFP approach.

---

### ⚠️ PROBLEM 11: Granny Model Rendering (EterGrnLib)

**What:** `EterGrnLib` handles skinned mesh rendering (characters, monsters, NPCs). Key files:
- `ModelInstanceRender.cpp` — draws skinned meshes with DX9
- `Material.cpp` — sets up textures and render states per material
- `Model.cpp` — manages vertex/index buffers

Uses `LPDIRECT3DVERTEXBUFFER9`, `SetTextureStageState`, `DrawIndexedPrimitive`, multi-stream vertex setup.

**Solution:** Skinned mesh rendering needs a dedicated shader:

```hlsl
cbuffer BoneMatrices : register(b2) {
    float4x4 gBones[96];  // max bone count
};

VS_OUT VS_Skinned(VS_SKINNED_IN input) {
    float4 pos = float4(0, 0, 0, 0);
    float3 norm = float3(0, 0, 0);
    for (int i = 0; i < 4; i++) {
        pos += input.weights[i] * mul(float4(input.pos, 1), gBones[input.indices[i]]);
        norm += input.weights[i] * mul(input.normal, (float3x3)gBones[input.indices[i]]);
    }
    // ... transform by VP, pass to pixel shader
}
```

**Note:** Granny SDK does its own CPU-side skinning and provides pre-transformed vertices. This simplifies things — we may not need a skinning shader if Granny continues to do CPU skinning. In that case, `FFP_PNT` shader is sufficient.

---

### ⚠️ PROBLEM 12: Alpha Test Emulation

**What:** DX9 has `D3DRS_ALPHATESTENABLE`, `D3DRS_ALPHAREF`, `D3DRS_ALPHAFUNC` render states. DX11 has **no alpha test** — it must be done in the pixel shader via `discard`.

**Where:** Almost every rendering file uses alpha test for:
- Foliage/tree transparency
- UI elements with transparency
- Effect particles with alpha cutout
- Fence/gate textures

**Solution:** ✅ **PARTIALLY SOLVED** — `CDX11ShaderManager` already has `CBPerMaterial.fAlphaRef` and `fAlphaTestEnable` in its material constant buffer. Every pixel shader variant must include:

```hlsl
if (cbMaterial.fAlphaTestEnable > 0.5f && color.a < cbMaterial.fAlphaRef)
    discard;
```

The `CDX11StateCache` tracks `D3DRS_ALPHATESTENABLE` and `D3DRS_ALPHAREF` — it just needs to propagate these to the shader manager's material cbuffer.

---

### ⚠️ PROBLEM 13: Present() / Swap Chain Transition

**What:** Currently DX9's `Present()` is used for screen presentation. DX11 has `IDXGISwapChain::Present()`. Both swap chains exist but only DX9 is active.

**Solution:** 

1. During transition: DX9 renders → copy to DX11 shared texture → DX11 post-process → DX11 present (this is the current Phase 1 approach)
2. Final state: Remove DX9 present entirely. DX11 swap chain is the only presenter. DX9 device creation can be skipped.

**Gotcha:** DXGI swap chain and DX9 Ex swap chain cannot share the same back buffer. The transition approach (copy frame) costs ~1-2ms per frame on modern GPUs.

---

### ⚠️ PROBLEM 14: SetViewport / Render Target Management

**What:** DX9 uses `D3DVIEWPORT9` and `SetViewport()`. DX11 uses `D3D11_VIEWPORT` and `RSSetViewports()`. Shadow passes, minimap rendering, and UI overlays all change viewports.

**Solution:** `CDX11StateCache` should intercept viewport changes and translate:

```cpp
void OnViewportChanged(const D3DVIEWPORT9& vp) {
    D3D11_VIEWPORT dx11vp;
    dx11vp.TopLeftX = (float)vp.X;
    dx11vp.TopLeftY = (float)vp.Y;
    dx11vp.Width = (float)vp.Width;
    dx11vp.Height = (float)vp.Height;
    dx11vp.MinDepth = vp.MinZ;
    dx11vp.MaxDepth = vp.MaxZ;
    m_pContext->RSSetViewports(1, &dx11vp);
}
```

---

### ⚠️ PROBLEM 15: EterImageLib DDS Texture Loader

**What:** `DDSTextureLoader9.h/.cpp` loads DDS files into `LPDIRECT3DTEXTURE9`. This must be replaced for DX11.

**Solution:** Microsoft provides `DDSTextureLoader11` as part of DirectXTK/DirectXTex. It creates `ID3D11Texture2D` + `ID3D11ShaderResourceView` directly.

Steps:
1. Add `DDSTextureLoader11.h/.cpp` to `EterImageLib`
2. Modify `CGraphicImageTexture::Create()` to call DX11 loader
3. Store result in `m_pDX11SRV` (member already exists)

---

## 6. Migration Phases

### Phase 2A: Dual-Render Foundation (Estimated: 2-3 weeks)

Goal: Make DX11 draw calls work alongside DX9.

- [ ] Expand `CDX11ShaderManager` with all shader variants (PNT, PNT2, PD, PT, skinned)
- [ ] Implement `CDynamicVBPool` for `DrawPrimitiveUP` replacement
- [ ] Add DX11 buffer creation in `CGraphicVertexBuffer::Create()` and `CGraphicIndexBuffer::Create()`
- [ ] Implement vertex data sync (`Lock`/`Unlock` → `Map`/`Unmap`)
- [ ] Add `DDSTextureLoader11` to `EterImageLib`
- [ ] Implement dual texture loading in `CGraphicImageTexture`
- [ ] Implement `CMatrixStack` replacement for `ID3DXMatrixStack`

### Phase 2B: StateManager DX11 Backend (Estimated: 2-3 weeks)

Goal: Route all draw calls through DX11 via `CStateManager`.

- [ ] Add DX11 mode flag to `CStateManager`
- [ ] Implement DX11 paths for all StateManager functions:
  - `DrawPrimitive_DX11()` / `DrawPrimitiveUP_DX11()`
  - `DrawIndexedPrimitive_DX11()` / `DrawIndexedPrimitiveUP_DX11()`
  - State translation via `CDX11StateCache` (already exists)
  - Transform matrix → shader constant buffer updates
  - Texture binding → SRV binding via texture registry
- [ ] Test with simple scenes (login screen, character select)

### Phase 2C: Subsystem-Specific Shaders (Estimated: 3-4 weeks)

Goal: Replace FFP-dependent subsystems with proper DX11 shaders.

- [ ] Terrain splatting shader (replace `MapOutdoorRender*` multi-pass FFP)
- [ ] Water shader (replace `MapOutdoorWater` blend stages)  
- [ ] Shadow mapping shader (use existing `CDX11ShadowMap`)
- [ ] SpeedTree leaf/branch shaders (wind animation)
- [ ] Effect/particle shaders (point sprites, additive blend)
- [ ] UI rendering shader (2D orthographic PDT)
- [ ] Text rendering shader (font texture atlas)
- [ ] SkyBox shader
- [ ] LensFlare shader

### Phase 3: DX9 Removal (Estimated: 1-2 weeks)

Goal: Remove all DX9 code paths.

- [ ] Remove `#include <d3d9.h>` from `StdAfx.h`
- [ ] Replace `#include <d3dx9.h>` with `#include "GrpMathCompat.h"`
- [ ] Remove `LPDIRECT3D9EX ms_lpd3d` and `LPDIRECT3DDEVICE9EX ms_lpd3dDevice` from `CGraphicBase`
- [ ] Remove `CStateManager` DX9 backend code
- [ ] Clean up `CGraphicVertexBuffer` / `CGraphicIndexBuffer` DX9 members
- [ ] Clean up `CGraphicTexture` DX9 members and texture registry
- [ ] Remove `GrpD3DXBuffer.h/.cpp`
- [ ] Remove `DDSTextureLoader9.h/.cpp`
- [ ] Remove DX9 vertex declarations from `GrpDevice.cpp`
- [ ] Remove `D3DPRESENT_PARAMETERS`, `D3DCAPS9` usage
- [ ] Update CMake to remove d3d9.lib, d3dx9.lib linking
- [ ] Final compile and link test

---

## 7. File-by-File Impact Matrix

### EterLib (Core Graphics) — HIGH IMPACT

| File | DX9 APIs Used | Migration Effort | Notes |
|------|---------------|------------------|-------|
| `StateManager.h/.cpp` | ALL DX9 wrapping | 🔴 Critical | Add DX11 backend |
| `GrpBase.h/.cpp` | Device ptrs, transforms, viewport | 🔴 Critical | Remove DX9 statics |
| `GrpDevice.h/.cpp` | Device creation, vertex decls | 🔴 Critical | Already has DX11 create |
| `GrpScreen.h/.cpp` | Draw calls, render states, transforms | 🟡 Medium | 10+ API calls |
| `GrpTextInstance.cpp` | DrawPrimitiveUP, texture stages | 🟡 Medium | Font rendering |
| `GrpTexture.h/.cpp` | `LPDIRECT3DTEXTURE9` | 🟡 Medium | Already has DX11 SRV member |
| `GrpImageTexture.cpp` | `D3DXCreateTexture*` | 🟡 Medium | Texture loading |
| `GrpVertexBuffer.h/.cpp` | `LPDIRECT3DVERTEXBUFFER9` | 🟡 Medium | Already has DX11 buffer member |
| `GrpIndexBuffer.h/.cpp` | `LPDIRECT3DINDEXBUFFER9` | 🟡 Medium | Already has DX11 buffer member |
| `GrpExpandedImageInstance.cpp` | DrawIndexed, render states | 🟢 Low | Uses STATEMANAGER |
| `GrpImageInstance.cpp` | DrawIndexed, render states | 🟢 Low | Uses STATEMANAGER |
| `GrpMarkInstance.cpp` | DrawIndexed | 🟢 Low | Uses STATEMANAGER |
| `BlockTexture.cpp` | DrawIndexed | 🟢 Low | Uses STATEMANAGER |
| `SkyBox.cpp` | Draw, transforms, tex stages | 🟡 Medium | Dedicated shader |
| `LensFlare.cpp` | DrawPrimitiveUP, tex stages | 🟡 Medium | Effect shader |
| `Decal.cpp` | DrawIndexed, transforms | 🟡 Medium | Decal shader |
| `Camera.h/.cpp` | D3DXMATRIX | 🟢 Low | GrpMathCompat |
| `CollisionData.cpp` | D3DXMATRIX, render states | 🟢 Low | Debug viz |
| `GrpShadowTexture.cpp` | `LPDIRECT3DTEXTURE9/SURFACE9` | 🟡 Medium | Shadow RT |
| `ScreenFilter.cpp` | SetTextureStageState | 🟢 Low | Post-process |
| `GrpD3DXBuffer.h/.cpp` | D3DX buffer utility | 🟢 Low | **DELETE** |
| `GrpMathCompat.h` | — | ✅ Done | Drop-in D3DX replacement |

### GameLib — MEDIUM IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `MapOutdoorRender.cpp` | DrawIndexed, transforms, tex stages | 🔴 Critical — terrain shader |
| `MapOutdoorRenderSTP.cpp` | DrawIndexed, transforms, tex stages | 🔴 Critical |
| `MapOutdoorRenderHTP.cpp` | DrawIndexed, transforms, tex stages | 🔴 Critical |
| `MapOutdoorWater.cpp` | DrawPrimitive, tex stages | 🟡 Medium — water shader |
| `MapOutdoorCharacterShadow.cpp` | Render states, transforms | 🟡 Medium |
| `MapOutdoor.h` | `LPDIRECT3DTEXTURE9` members | 🟡 Medium |
| `ActorInstanceRender.cpp` | Render states, tex stages | 🟢 Low — uses STATEMANAGER |
| `FlyTrace.cpp` | DrawPrimitiveUP | 🟢 Low |
| `WeaponTrace.cpp` | DrawPrimitiveUP | 🟢 Low |
| `SnowEnvironment.cpp` | DrawPrimitiveUP, DrawIndexed | 🟢 Low |
| `DungeonBlock.cpp` | Render states | 🟢 Low |
| `Area.cpp` | Render states, tex stages | 🟢 Low |
| `TerrainDecal.cpp` | SetTextureStageState | 🟢 Low |
| `AreaTerrain.cpp` | `LPDIRECT3DTEXTURE9` | 🟢 Low |

### SpeedTreeLib — MEDIUM IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `SpeedTreeWrapper.cpp` | DrawPrimitive, render states, vertex buffers, textures | 🟡 Medium |
| `SpeedTreeForestDirectX.cpp` | Render states, tex stages | 🟡 Medium |
| `SpeedGrassWrapper.h` | `LPDIRECT3DTEXTURE9` | 🟢 Low |

### EterGrnLib — MEDIUM IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `ModelInstanceRender.cpp` | DrawIndexed, stream source, vertex buffers | 🟡 Medium |
| `Material.cpp` | SetTexture, render states, tex stages | 🟡 Medium |
| `Model.cpp` | `LPDIRECT3DVERTEXBUFFER9` | 🟡 Medium |
| `ModelInstance.h` | `IDirect3DTexture9*` | 🟢 Low |

### EffectLib — LOW-MEDIUM IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `ParticleSystemInstance.cpp` | DrawPrimitiveUP, render states | 🟡 Medium |
| `EffectMeshInstance.cpp` | DrawPrimitiveUP, render states, tex stages | 🟡 Medium |
| `EffectInstance.cpp` | SetTextureStageState | 🟢 Low |
| `ParticleInstance.cpp` | SetRenderState | 🟢 Low |

### EterPythonLib — LOW IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `PythonGraphic.cpp` | Draw*, transforms, tex stages | 🟡 Medium — UI rendering |
| `PythonWindow.cpp` | Render states | 🟢 Low |

### UserInterface — LOW IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `PythonMiniMap.cpp` | DrawIndexed, transforms, textures | 🟡 Medium |
| `InstanceBase.cpp` | Render states | 🟢 Low |
| `PythonCharacterManager.cpp` | SetTextureStageState | 🟢 Low |
| `PythonApplication.cpp` | Present, device state | 🟢 Low |
| `PythonApplicationLogo.cpp` | `LPDIRECT3DTEXTURE9` | 🟢 Low |
| `PythonBackground.cpp` | SetTransform | 🟢 Low |

### EterImageLib — LOW IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `DDSTextureLoader9.h/.cpp` | `LPDIRECT3DTEXTURE9`, `IDirect3DDevice9` | 🟡 Replace with DX11 version |

### PRTerrainLib — LOW IMPACT

| File | DX9 APIs Used | Migration Effort |
|------|---------------|------------------|
| `Terrain.h` | `LPDIRECT3DTEXTURE9` | 🟢 Low — type change |
| `TextureSet.h` | `LPDIRECT3DTEXTURE9` | 🟢 Low — type change |
| `TerrainType.h` | `LPDIRECT3DTEXTURE9` | 🟢 Low — type change |

---

## 8. Build System Changes

### Current CMake Linking (likely)
```cmake
# Current (DX9)
target_link_libraries(EterLib d3d9.lib d3dx9.lib dxguid.lib)

# Add for transition
target_link_libraries(EterLib d3d11.lib dxgi.lib d3dcompiler.lib)

# Final (DX11 only)
target_link_libraries(EterLib d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib)
# Remove: d3d9.lib d3dx9.lib
```

### DirectXTK/DirectXTex Dependencies
For `DDSTextureLoader11`, either:
1. **Vendored:** Copy `DDSTextureLoader11.h/.cpp` into `EterImageLib` (simplest)
2. **NuGet/vcpkg:** Add DirectXTK as a package dependency

**Recommendation:** Vendor the files. DirectXTK's `DDSTextureLoader` is a standalone pair of files with no additional dependencies.

### Shader Compilation
HLSL shaders should be:
1. Compiled at **build time** using `fxc.exe` or `dxc.exe` into bytecode
2. Embedded as byte arrays in headers (current approach in `CDX11ShaderManager` uses runtime `D3DCompile`)
3. **Alternative:** Runtime compilation during init (current approach) — simpler for development but slower startup

**Recommendation:** Keep runtime compilation during development, switch to precompiled for release builds.

---

## Summary Statistics

| Metric | Count |
|--------|-------|
| Total files with DX9 dependencies | **~50** |
| Files already modified for DX11 | **6** (headers with DX11 members) |
| New DX11 files created | **9** |
| Shader variants needed | **~10-12** |
| Estimated total migration effort | **8-12 weeks** |
| Highest risk subsystem | `CStateManager` + Terrain rendering |
| Most files affected | `SetRenderState` (30+), `SetTextureStageState` (27+) |
