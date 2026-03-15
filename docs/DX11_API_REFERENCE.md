# Direct3D 11 API Reference — Quick Guide

Source: [Microsoft DX11 Documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d11/atoc-dx-graphics-direct3d-11)

---

## Architecture: Device vs Context

DX11 splits functionality between two objects:

| Object | Purpose | Key APIs |
|--------|---------|----------|
| **ID3D11Device** | Resource creation (buffers, textures, shaders, state objects) | `CreateBuffer`, `CreateTexture2D`, `CreateShaderResourceView`, `CreateVertexShader`, `CreateInputLayout`, `CreateBlendState`, `CreateDepthStencilState`, `CreateRasterizerState`, `CreateSamplerState` |
| **ID3D11DeviceContext** | Rendering commands, state binding, draw calls | `IASetVertexBuffers`, `IASetIndexBuffer`, `IASetInputLayout`, `IASetPrimitiveTopology`, `VSSetShader`, `PSSetShader`, `Draw`, `DrawIndexed`, `Map`/`Unmap`, `OMSetRenderTargets`, `RSSetViewports` |

**Immediate Context**: One per device, executes commands immediately to the GPU.
**Deferred Context**: Records command lists for multi-threaded rendering (optional).

Created via:
```cpp
ID3D11Device* device;
ID3D11DeviceContext* context;
D3D11CreateDeviceAndSwapChain(
    nullptr,                    // adapter (default)
    D3D_DRIVER_TYPE_HARDWARE,   // driver type
    nullptr,                    // software rasterizer
    flags,                      // creation flags (D3D11_CREATE_DEVICE_DEBUG)
    featureLevels, numLevels,   // feature levels
    D3D11_SDK_VERSION,
    &swapChainDesc,
    &swapChain,
    &device,
    &featureLevel,
    &context);
```

---

## Buffers

### Vertex Buffer
```cpp
D3D11_BUFFER_DESC vbDesc = {};
vbDesc.ByteWidth = vertexCount * stride;
vbDesc.Usage = D3D11_USAGE_DEFAULT;       // GPU read/write
vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

D3D11_SUBRESOURCE_DATA initData = {};
initData.pSysMem = vertices;

ID3D11Buffer* pVB;
device->CreateBuffer(&vbDesc, &initData, &pVB);

// Bind:
UINT stride = sizeof(Vertex);
UINT offset = 0;
context->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
```

**Usage Modes:**
| Usage | CPU Access | GPU Access | Map | UpdateSubresource |
|-------|-----------|-----------|-----|-------------------|
| DEFAULT | None | Read/Write | No | Yes |
| DYNAMIC | Write | Read | D3D11_MAP_WRITE_DISCARD | No |
| IMMUTABLE | None | Read | No | No |
| STAGING | Read/Write | None | Yes | Yes |

### Index Buffer
```cpp
D3D11_BUFFER_DESC ibDesc = {};
ibDesc.ByteWidth = indexCount * sizeof(UINT16);
ibDesc.Usage = D3D11_USAGE_DEFAULT;
ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

device->CreateBuffer(&ibDesc, &initData, &pIB);
context->IASetIndexBuffer(pIB, DXGI_FORMAT_R16_UINT, 0);
```

### Constant Buffer
```cpp
D3D11_BUFFER_DESC cbDesc = {};
cbDesc.ByteWidth = sizeof(MyCB);  // Must be 16-byte aligned
cbDesc.Usage = D3D11_USAGE_DYNAMIC;
cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

device->CreateBuffer(&cbDesc, nullptr, &pCB);

// Update:
D3D11_MAPPED_SUBRESOURCE mapped;
context->Map(pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
memcpy(mapped.pData, &cbData, sizeof(cbData));
context->Unmap(pCB, 0);

// Bind:
context->VSSetConstantBuffers(0, 1, &pCB);
context->PSSetConstantBuffers(0, 1, &pCB);
```
Max 15 constant buffers per shader stage, each up to 4096 float4 constants.

---

## Textures

### Creating a 2D Texture
```cpp
D3D11_TEXTURE2D_DESC texDesc = {};
texDesc.Width  = width;
texDesc.Height = height;
texDesc.MipLevels = 1;
texDesc.ArraySize = 1;
texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
texDesc.SampleDesc.Count = 1;
texDesc.Usage = D3D11_USAGE_DEFAULT;
texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

D3D11_SUBRESOURCE_DATA initData = {};
initData.pSysMem = pixelData;
initData.SysMemPitch = width * 4;

ID3D11Texture2D* pTex;
device->CreateTexture2D(&texDesc, &initData, &pTex);
```

### Shader Resource View (SRV)
```cpp
D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
srvDesc.Texture2D.MipLevels = 1;

ID3D11ShaderResourceView* pSRV;
device->CreateShaderResourceView(pTex, &srvDesc, &pSRV);

// Bind to pixel shader slot 0:
context->PSSetShaderResources(0, 1, &pSRV);
```

### Sampler State
```cpp
D3D11_SAMPLER_DESC sampDesc = {};
sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

ID3D11SamplerState* pSampler;
device->CreateSamplerState(&sampDesc, &pSampler);
context->PSSetSamplers(0, 1, &pSampler);
```

---

## Graphics Pipeline Stages

```
Input Assembler (IA) → Vertex Shader (VS) → [Hull Shader → Tessellator → Domain Shader]
    → [Geometry Shader → Stream Output] → Rasterizer → Pixel Shader (PS) → Output Merger (OM)
```

### Input Assembler (IA)
- Reads vertex/index buffers, assembles primitives
- Requires `IASetVertexBuffers`, `IASetIndexBuffer`, `IASetInputLayout`, `IASetPrimitiveTopology`
- No TriangleFan support! Use TriangleList/TriangleStrip

### Vertex Shader (VS) — Required
- Processes each vertex, outputs clip-space position
- `VSSetShader`, `VSSetConstantBuffers`, `VSSetShaderResources`

### Hull/Domain/Geometry Shaders — Optional
- Tessellation via Hull + Tessellator + Domain
- Geometry shader for per-primitive operations

### Rasterizer (RS)
- Converts primitives to pixels
- `RSSetState`, `RSSetViewports`, `RSSetScissorRects`

### Pixel Shader (PS)
- Per-pixel shading, texture sampling, lighting
- `PSSetShader`, `PSSetConstantBuffers`, `PSSetShaderResources`, `PSSetSamplers`

### Output Merger (OM)
- Depth/stencil testing, blending, render target output
- `OMSetRenderTargets`, `OMSetDepthStencilState`, `OMSetBlendState`

---

## Draw Calls

```cpp
// Non-indexed:
context->Draw(vertexCount, startVertexLocation);

// Indexed:
context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);

// Instanced:
context->DrawIndexedInstanced(indexCountPerInstance, instanceCount,
                               startIndexLocation, baseVertexLocation,
                               startInstanceLocation);
```

---

## Render Targets and Depth

```cpp
// Create render target view from swap chain back buffer:
ID3D11Texture2D* pBackBuffer;
swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
ID3D11RenderTargetView* pRTV;
device->CreateRenderTargetView(pBackBuffer, nullptr, &pRTV);

// Create depth stencil:
D3D11_TEXTURE2D_DESC depthDesc = {};
depthDesc.Width = width;
depthDesc.Height = height;
depthDesc.MipLevels = 1;
depthDesc.ArraySize = 1;
depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
depthDesc.SampleDesc.Count = 1;
depthDesc.Usage = D3D11_USAGE_DEFAULT;
depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

ID3D11Texture2D* pDepthTex;
device->CreateTexture2D(&depthDesc, nullptr, &pDepthTex);

ID3D11DepthStencilView* pDSV;
device->CreateDepthStencilView(pDepthTex, nullptr, &pDSV);

// Bind:
context->OMSetRenderTargets(1, &pRTV, pDSV);

// Clear:
float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
context->ClearRenderTargetView(pRTV, clearColor);
context->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
```

---

## DX9 → DX11 Migration Key Points

From [Microsoft Migration Guide](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d11-programming-guide-migrating):

1. **No fixed-function pipeline** — All rendering via HLSL shaders
2. **Immutable state objects** — Create `BlendState`, `DepthStencilState`, `RasterizerState`, `SamplerState` once, bind as needed (not per-parameter like DX9)
3. **Input layouts replace FVF** — `D3D11_INPUT_ELEMENT_DESC` + `CreateInputLayout` linked to VS bytecode
4. **Shader Resource Views** for all textures — No direct `SetTexture`
5. **DXGI_FORMAT replaces D3DFORMAT** — No 24-bit formats, strict RGB ordering
6. **Constant buffers replace individual Set*ShaderConstant calls** — Group constants efficiently
7. **No `D3DPOOL`** — Use `D3D11_USAGE` (DEFAULT/DYNAMIC/IMMUTABLE/STAGING)
8. **No `Lock`/`Unlock`** — Use `Map`/`Unmap` on DYNAMIC or STAGING resources
9. **No TriangleFan** — Convert to TriangleList
10. **No `BeginScene`/`EndScene`** — Draw immediately through context

---

## DX11 New Features (Available for Our Use)

- **Compute Shaders** — GPU particle systems, physics, culling
- **Tessellation** — Hull/Domain shaders for dynamic LOD
- **Shader Model 5.0** — Full HLSL feature set
- **BC6H/BC7 compression** — Better texture quality/size ratio
- **DrawInstanced** — Massive batching for foliage, particles
- **Deferred Contexts** — Multi-threaded command recording
- **Structured Buffers / UAVs** — Flexible GPU data structures
