#pragma once

// Lean DX11 DDS/texture loading — creates ID3D11ShaderResourceView from memory
// Supports: DDS (all common formats), raw RGBA (from stb_image or similar)

#include <cstdint>
#include <d3d11.h>


namespace DX11Tex {
// Load a DDS texture from memory → ID3D11ShaderResourceView
// Returns S_OK on success, E_FAIL on error
HRESULT CreateDDSTextureFromMemory(
    _In_ ID3D11Device *device, _In_reads_bytes_(dataSize) const uint8_t *data,
    _In_ size_t dataSize, _Out_ ID3D11ShaderResourceView **srv,
    _Out_opt_ ID3D11Texture2D **texture = nullptr);

// Create a DX11 texture + SRV from raw RGBA8 pixel data
HRESULT CreateTextureFromRGBA(
    _In_ ID3D11Device *device, _In_ UINT width, _In_ UINT height,
    _In_reads_bytes_(width * height * 4) const uint8_t *rgbaData,
    _In_ bool isBGRA, // true = B8G8R8A8, false = R8G8B8A8
    _Out_ ID3D11ShaderResourceView **srv,
    _Out_opt_ ID3D11Texture2D **texture = nullptr);

// Create an empty dynamic texture (for font textures, etc.)
HRESULT CreateDynamicTexture(_In_ ID3D11Device *device, _In_ UINT width,
                             _In_ UINT height, _In_ DXGI_FORMAT format,
                             _Out_ ID3D11ShaderResourceView **srv,
                             _Out_opt_ ID3D11Texture2D **texture = nullptr);
} // namespace DX11Tex
