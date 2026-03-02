# DX11 Migration — Problems & Solutions Log

## Reference Target

The visual quality target is a modern Metin2 client with:
- PBR-quality lighting and shading
- Dense 3D grass with wind animation
- Atmospheric fog and depth
- High-detail character models with specular reflections
- Volumetric-style lighting through trees
- Soft shadows

---

## Phase 0: D3DX → DirectXMath Compatibility

### Problem 1: D3DX types redefined
**Problem**: `GrpMathCompat.h` defines `D3DXMATRIX`, `D3DXVECTOR3`, etc. — these conflict with `d3dx9math.h` which defines them too.

**Solution**: Phase 0 keeps `d3dx9.h` included alongside `GrpMathCompat.h` (which is **not yet included** in StdAfx.h). The compatibility header will replace `d3dx9.h` when the switchover happens. Types are binary-compatible so no conversion is needed.

---

### Problem 2: Clang lint errors — `DWORD`, `ULONG`, `HRESULT` unknown
**Problem**: `GrpMathCompat.h` uses Windows types but clang analyzed it standalone without `windows.h`.

**Solution**: Added `#include <windows.h>` and `#include <vector>` at the top to make the header self-contained.

---

### Problem 3: `ID3DXMatrixStack*` pointer mismatch
**Problem**: Original D3DX defines `ID3DXMatrixStack` as a COM interface — code uses `ID3DXMatrixStack*` as a pointer. Initial typedef `typedef CD3DXMatrixStack* ID3DXMatrixStack` made `ID3DXMatrixStack*` = `CD3DXMatrixStack**` (double pointer).

**Solution**: Changed to `typedef CD3DXMatrixStack ID3DXMatrixStack` — now `ID3DXMatrixStack*` = `CD3DXMatrixStack*` which matches how the code uses it.

---

### Problem 4: `D3DXVec3Project`/`Unproject` viewport type mismatch
**Problem**: Original D3DX uses `D3DVIEWPORT9*` parameter. DX11 uses `D3D11_VIEWPORT` with different field names (`X/Y/Width/Height/MinZ/MaxZ` vs `TopLeftX/TopLeftY/Width/Height/MinDepth/MaxDepth`).  

**Solution**: Made functions templated on viewport type — `template<typename TViewport>`. Accesses `pViewport->X`, `pViewport->Y`, `pViewport->MinZ`, `pViewport->MaxZ` which matches `D3DVIEWPORT9`. Will be updated when switching to DX11.

---

### Problem 5: D3DX non-math functions still needed
**Problem**: Removing `d3dx9.h` also removes D3DX utility functions used throughout the codebase:
- `D3DXCreateTextureFromFileInMemoryEx` (texture loading)
- `D3DXCreateEffect` (FX framework)
- `D3DXAssembleShader` / `D3DXAssembleShaderFromFileW` (shader compilation)
- `D3DXCreateSphere` / `D3DXCreateCylinder` (debug meshes)
- `D3DXCreateTexture` (texture creation)
- `LPD3DXMESH`, `LPD3DXBUFFER`, `LPD3DXEFFECT` (interfaces)

**Solution**: These will be stubbed/replaced in later phases:
- Texture loading → DirectXTK `CreateWICTextureFromMemory` / `CreateDDSTextureFromMemory`
- Shader compilation → `D3DCompile` from `d3dcompiler.h`
- Debug meshes → Custom vertex buffer generation or DirectXTK `GeometricPrimitive`
- FX framework → Direct shader binding (no effects framework in DX11)

---

## Phase 1: Device & Swap Chain
*(To be filled as work progresses)*

## Phase 2: State Manager
*(To be filled)*

## Phase 3: Buffers & Textures
*(To be filled)*

## Phase 4: Shaders & FFP Emulation
*(To be filled)*

## Phase 5: Subsystem Porting
*(To be filled)*
