# 03 Texture Block Compression

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11](https://learn.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Texture Block Compression in Direct3D 11
FeedbackSummarize this article for me
## 


Block Compression (BC) support for textures has been extended in Direct3D 11 to include the BC6H and BC7 algorithms. BC6H supports high-dynamic range color source data, and BC7 provides better-than-average quality compression with less artifacts for standard RGB source data.

For more specific information about block compression algorithm support prior to Direct3D 11, including support for the BC1 through BC5 formats, see[Block Compression (Direct3D 10).

Note about File Formats:The BC6H and BC7 texture compression formats use the DDS file format for storing the compressed texture data. For more information, see the[Programming Guide for DDSfor details.
## Block Compression Formats Supported in Direct3D 11


| Source data | Minimum required data compression resolution | Recommended format | Minimum supported feature level |
| Three-channel color with alpha channel | Three color channels (5 bits:6 bits:5 bits), with 0 or 1 bit(s) of alpha | BC1 | Direct3D 9.1 |
| Three-channel color with alpha channel | Three color channels (5 bits:6 bits:5 bits), with 4 bits of alpha | BC2 | Direct3D 9.1 |
| Three-channel color with alpha channel | Three color channels (5 bits:6 bits:5 bits) with 8 bits of alpha | BC3 | Direct3D 9.1 |
| One-channel color | One color channel (8 bits) | BC4 | Direct3D 10 |
| Two-channel color | Two color channels (8 bits:8 bits) | BC5 | Direct3D 10 |
| Three-channel high dynamic range (HDR) color | Three color channels (16 bits:16 bits:16 bits) in "half" floating point* | BC6H | Direct3D 11 |
| Three-channel color, alpha channel optional | Three color channels (4 to 7 bits per channel) with 0 to 8 bits of alpha | BC7 | Direct3D 11 |

*"Half" floating point is a 16 bit value that consists of an optional sign bit, a 5 bit biased exponent, and a 10 or 11 bit mantissa.
## BC1, BC2, and B3 Formats


The BC1, BC2, and BC3 formats are equivalent to the Direct3D 9 DXTn texture compression formats, and are the same as the corresponding Direct3D 10 BC1, BC2, and BC3 formats. Support for these three formats is required by all feature levels (D3D_FEATURE_LEVEL_9_1, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1, and D3D_FEATURE_LEVEL_11_0).

| Block compression format | DXGI format | Direct3D 9 equivalent format | Bytes per 4x4 pixel block |
| BC1 | DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM_SRGB, DXGI_FORMAT_BC1_TYPELESS | D3DFMT_DXT1, FourCC="DXT1" | 8 |
| BC2 | DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM_SRGB, DXGI_FORMAT_BC2_TYPELESS | D3DFMT_DXT2*, FourCC="DXT2", D3DFMT_DXT3, FourCC="DXT3" | 16 |
| BC3 | DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM_SRGB, DXGI_FORMAT_BC3_TYPELESS | D3DFMT_DXT4*, FourCC="DXT4", D3DFMT_DXT5, FourCC="DXT5" | 16 |

*These compression schemes (DXT2 and DXT4) make no distinction between the Direct3D 9 pre-multiplied alpha formats and the standard alpha formats. This distinction must be handled by the programmable shaders at render time.
## BC4 and BC5 Formats


| Block compression format | DXGI format | Direct3D 9 equivalent format | Bytes per 4x4 pixel block |
| BC4 | DXGI_FORMAT_BC4_UNORM, DXGI_FORMAT_BC4_SNORM, DXGI_FORMAT_BC4_TYPELESS | FourCC="ATI1" | 8 |
| BC5 | DXGI_FORMAT_BC5_UNORM, DXGI_FORMAT_BC5_SNORM, DXGI_FORMAT_BC5_TYPELESS | FourCC="ATI2" | 16 |
## BC6H Format


For more detailed information about this format, see the[BC6H Formatdocumentation.

| Block compression format | DXGI format | Direct3D 9 equivalent format | Bytes per 4x4 pixel block |
| BC6H | DXGI_FORMAT_BC6H_UF16, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC6H_TYPELESS | N/A | 16 |

The BC6H format can select different encoding modes for each 4x4 pixel block. A total of 14 different encoding modes are available, each with slightly different trade-offs in the resulting visual quality of the displayed texture. The choice of modes allows for fast decoding by the hardware with the quality level selected or adapted according to the source content, but it also greatly increases the complexity of the search space.
## BC7 Format


For more detailed information about this format, see the[BC7 Formatdocumentation.

| Block compression format | DXGI format | Direct3D 9 equivalent format | Bytes per 4x4 pixel block |
| BC7 | DXGI_FORMAT_BC7_UNORM, DXGI_FORMAT_BC7_UNORM_SRGB, DXGI_FORMAT_BC7_TYPELESS | N/A | 16 |

The BC7 format can select different encoding modes for each 4x4 pixel block. A total of 8 different encoding modes are available, each with slightly different trade-offs in the resulting visual quality of the displayed texture. The choice of modes allows for fast decoding by the hardware with the quality level selected or adapted according to the source content, but it also greatly increases the complexity of the search space.
## In this section


| Topic | Description |
| [BC6H Format
 | The BC6H format is a texture compression format designed to support high-dynamic range (HDR) color spaces in source data.
 |
| [BC7 Format
 | The BC7 format is a texture compression format used for high-quality compression of RGB and RGBA data.
 |
| [BC7 Format Mode Reference
 | This documentation contains a list of the 8 block modes and bit allocations for BC7 texture compression format blocks.
 |
## Related topics


[Block Compression (Direct3D 10)

[Textures
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19