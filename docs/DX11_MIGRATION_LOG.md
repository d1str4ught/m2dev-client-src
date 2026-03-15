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

### Problem 6: DX11 and DX9 headers coexistence
**Problem**: Including both `d3d9.h` and `d3d11.h` in the same translation unit — potential type conflicts?

**Solution**: No conflicts — DX9 and DX11 headers use different type prefixes (`IDirect3D*` vs `ID3D11*`). Both can coexist safely. `StdAfx.h` includes both during the transition period.

---

### Problem 7: IDE clang lint errors about missing d3dx9.h
**Problem**: Clang IDE analyzer reports `d3dx9.h` file not found, `D3DXMATRIX` unknown type, etc. across 40+ errors.

**Solution**: False positives — clang doesn't have the `extern/include` path in its search paths. MSVC build works perfectly through the CMake configuration. These are **IDE-only errors**, not build errors.

---

### Problem 8: Static member initialization for DX11 types
**Problem**: New DX11 static members (`ID3D11Device*`, `IDXGISwapChain*`, etc.) need proper initialization alongside existing DX9 members.

**Solution**: All DX11 pointers initialized to `nullptr`, feature level to `D3D_FEATURE_LEVEL_11_0` in `GrpBase.cpp`. Build verified — `EterLib.lib` (27.3 MB) compiles successfully.

---

### Problem 9: GrpBase.h file lock prevents forward declarations
**Problem**: `GrpBase.h` is locked by VS Code, preventing forward declarations for DX11 COM types (`ID3D11Device`, etc.) from being written to disk. The edit tool reports success but the changes don't persist.

**Solution**: Killed stale MSBuild processes that were locking the file, then used PowerShell `[System.IO.File]::WriteAllText()` to write the forward declarations directly. Build succeeded after fix.

Also added `<string>` include to `GrpDevice.h` (another latent dependency exposed by the change).

---

### Problem 10: Ray.h missing `<cassert>` include (latent bug)
**Problem**: `Ray.h` uses `assert()` without including `<cassert>`, relying on PCH ordering. Exposed when `GrpBase.h` changes altered PCH compilation order.

**Solution**: Added `#include <cassert>` to `Ray.h`.

## Phase 2: State Manager
*(To be filled)*

## Phase 3: Buffers & Textures
*(To be filled)*

## Phase 4: Shaders & FFP Emulation
*(To be filled)*

## Phase 5: Subsystem Porting
*(To be filled)*
