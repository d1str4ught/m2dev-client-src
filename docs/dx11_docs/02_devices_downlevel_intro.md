# 02 Devices Downlevel Intro

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-intro](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-intro)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Direct3D feature levels
FeedbackSummarize this article for me
## 


To handle the diversity of video cards in new and existing machines, Microsoft Direct3D 11 introduces the concept of feature levels. This topic discusses Direct3D feature levels.

Each video card implements a certain level of Microsoft DirectX (DX) functionality depending on the graphics processing units (GPUs) installed. In prior versions of Microsoft Direct3D, you could find out the version of Direct3D the video card implemented, and then program your application accordingly.

With Direct3D 11, a new paradigm is introduced called feature levels. A feature level is a well-defined set of GPU functionality. For instance, the 9_1 feature level implements the functionality that was implemented in Microsoft Direct3D 9, which exposes the capabilities of shader models[ps_2_xand[vs_2_x, while the 11_0 feature level implements the functionality that was implemented in Direct3D 11.

Now when you create a device, you can attempt to create a device for the feature level that you want to request. If the device creation works, that feature level exists, if not, the hardware does not support that feature level. You can either try to recreate a device at a lower feature level or you can choose to exit the application. For more info about creating a device, see the[D3D11CreateDevicefunction.

Using feature levels, you can develop an application for Direct3D 9, Microsoft Direct3D 10, or Direct3D 11, and then run it on 9, 10 or 11 hardware (with some exceptions; for example, new 11 features will not run on an existing 9 card). Here is a couple of other basic properties of feature levels:
- A GPU that allows a device to be created meets or exceeds the functionality of that feature level.
- A feature level always includes the functionality of previous or lower feature levels.
- A feature level does not imply performance, only functionality. Performance is dependent on hardware implementation.
- Choose a feature level when you create a Direct3D 11 device.

For information about limitations creating nonhardware-type devices on certain feature levels, see[Limitations Creating WARP and Reference Devices.

To assist you in deciding what feature level to design with, compare the features for each feature level.

The[10Level9 Referencesection lists the differences between how various[ID3D11Deviceand[ID3D11DeviceContextmethods behave at various 10Level9 feature levels.
## Formats of version numbers


There are three formats for Direct3D versions, shader models, and feature levels.
- Direct3D versions use a period; for example, Direct3D 12.0.
- Shader models use a period; for example, shader model 5.1.
- Feature levels use an underscore; for example, feature level 12_0.
## Direct3D 12 feature support (feature levels 12_2 through 11_0)


The following features are available for the feature levels listed. The headings across the top row are Direct3D 12 feature levels. The headings in the left-hand column are features. Also see[Footnotes for the tables.

| Feature \ Feature Level | 12_28 | 12_10 | 12_00 | 11_11 | 11_0 |
| Shader model | 6.5 | 5.12 | 5.12 | 5.12 | 5.12 |
| WDDM driver model | 2.0 | 1.x | 1.x | 1.x | 1.x |
| Raytracing | Tier 1.1 | Optional | Optional | Optional | Optional |
| Variable shading rate | Tier 2 | Optional | Optional | Optional | Optional |
| Mesh shader | Tier 1 | Optional | Optional | Optional | Optional |
| Sampler feedback | Tier 0.9 | Optional | Optional | Optional | Optional |
| Resource binding | Tier 3 | Tier 3 | Tier 3 | Tier 3 | Tier 1 |
| Root signature | 1.1 | 1 | 1 | 1 | 1 |
| Depth bounds test | Yes | Optional | Optional | Optional | Optional |
| Write buffer immediate | Direct, Compute, Bundle | Optional | Optional | Optional | Optional |
| GPU virtual address bits | 4010 | 4010 | 4010 |  |  |
| [Tiled resources | Tier3 | Tier26 | Tier26 | Optional | Optional |
| [Conservative Rasterization | Tier3 | Tier16 | Optional | Optional | No |
| [Rasterizer Order Views | Yes | Yes | Optional | Optional | No |
| [Min/Max Filters | Yes | Yes | Yes | Optional | No |
| Map Default Buffer | N/A | Optional | Optional | Optional | Optional |
| [Shader Specified Stencil Reference Value | Optional | Optional | Optional | Optional | No |
| Typed Unordered Access View Loads | 18 formats, more optional | 18 formats, more optional | 18 formats, more optional | 3 formats, more optional | 3 formats, more optional |
| [Geometry Shader | Yes | Yes | Yes | Yes | Yes |
| [Stream Out | Yes | Yes | Yes | Yes | Yes |
| [DirectCompute / Compute Shader | Yes | Yes | Yes | Yes | Yes |
| [Hull and Domain Shaders | Yes | Yes | Yes | Yes | Yes |
| Feature \ Feature Level | 12_28 | 12_10 | 12_00 | 11_11 | 11_0 |
| [Texture Resource Arrays | Yes | Yes | Yes | Yes | Yes |
| [Cubemap Resource Arrays | Yes | Yes | Yes | Yes | Yes |
| [BC4/BC5 Compression | Yes | Yes | Yes | Yes | Yes |
| [BC6H/BC7 Compression | Yes | Yes | Yes | Yes | Yes |
| [Alpha-to-coverage | Yes | Yes | Yes | Yes | Yes |
| [Extended Formats (BGRA, and so on) | Yes | Yes | Yes | Yes | Yes |
| [10-bit XR High Color Format | Yes | Yes | Yes | Yes | Yes |
| [Logic Operations (Output Merger) | Yes | Yes | Yes | Yes | Optional1 |
| Target-independent rasterization | Yes | Yes | Yes | Yes | No |
| [Multiple render target(MRT) with ForcedSampleCount 1 | Yes | Yes | Yes | Yes | Optional1 |
| UAV slots | Tiered9 | 64 | 64 | 64 | 8 |
| UAVs at every stage | Yes | Yes | Yes | Yes | No |
| Feature \ Feature Level | 12_28 | 12_10 | 12_00 | 11_11 | 11_0 |
| [Max forced sample count for UAV-only rendering | 16 | 16 | 16 | 16 | 8 |
| Constant buffer offsetting and partial updates | Yes | Yes | Yes | Yes | Optional1 |
| 16 bits per pixel (bpp) formats | Yes | Yes | Yes | Yes | Optional1 |
| Max Texture Dimension | 16384 | 16384 | 16384 | 16384 | 16384 |
| Max Cubemap Dimension | 16384 | 16384 | 16384 | 16384 | 16384 |
| Max Volume Extent | 2048 | 2048 | 2048 | 2048 | 2048 |
| Max Texture Repeat | 16384 | 16384 | 16384 | 16384 | 16384 |
| Max Anisotropy | 16 | 16 | 16 | 16 | 16 |
| Max Primitive Count | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 |
| Max Vertex Index | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 |
| Max Input Slots | 32 | 32 | 32 | 32 | 32 |
| Simultaneous Render Targets | 8 | 8 | 8 | 8 | 8 |
| Feature \ Feature Level | 12_28 | 12_10 | 12_00 | 11_11 | 11_0 |
| Occlusion Queries | Yes | Yes | Yes | Yes | Yes |
| Separate Alpha Blend | Yes | Yes | Yes | Yes | Yes |
| Mirror Once | Yes | Yes | Yes | Yes | Yes |
| Overlapping Vertex Elements | Yes | Yes | Yes | Yes | Yes |
| Independent Write Masks | Yes | Yes | Yes | Yes | Yes |
| Instancing | Yes | Yes | Yes | Yes | Yes |
| Nonpowers-of-2 conditionally3 | No | No | No | No | No |
| Nonpowers-of-2 unconditionally4 | Yes | Yes | Yes | Yes | Yes |

Additionally, the following flags are set:

| Feature \ Feature Level | 12_28 |
| WaveOps | TRUE |
| OutputMergerLogicOp | TRUE |
| VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportWithoutGSEmulation | TRUE |
| CopyQueueTimestampQueriesSupported | TRUE |
| CastingFullyTypedFormatSupported | TRUE |
| Int64ShaderOps | TRUE |
## Direct3D 11 feature support (feature levels 12_1 through 9_1)


The following features are available for the feature levels listed. The headings across the top row are Direct3D 11 feature levels. The headings in the left-hand column are features. Also see[Footnotes for the tables.

| Feature \ Feature Level | 12_10 | 12_00 | 11_11 | 11_0 | 10_1 | 10_0 | 9_37 | 9_2 | 9_1 |  |  |
| Shader model | 5.12 | 5.12 | 5.02 | 5.02 | 4.x | 4.0 | 2.0 (4_0_level_9_3) [vs_2_a/ps_2_x]5 | 2.0 (4_0_level_9_1) | 2.0 (4_0_level_9_1) |  |  |
| WDDM driver model | 1.x | 1.x | 1.x | 1.x | 1.x | 1.x | 1.x | 1.x | 1.x |  |  |
| [Tiled resources | Tier26 | Tier26 | Optional | Optional | No | No | No | No | No |  |  |
| [Conservative Rasterization | Tier16 | Optional | Optional | No | No | No | No | No | No |  |  |
| [Rasterizer Order Views | Yes | Optional | Optional | No | No | No | No | No | No |  |  |
| [Min/Max Filters | Yes | Yes | Optional | No | No | No | No | No | No |  |  |
| Map Default Buffer | Optional | Optional | Optional | Optional | No | No | No | No | No |  |  |
| [Shader Specified Stencil Reference Value | Optional | Optional | Optional | No | No | No | No | No | No |  |  |
| Typed Unordered Access View Loads | 18 formats, more optional | 18 formats, more optional | 3 formats, more optional | 3 formats, more optional | No | No | No | No | No |  |  |
| [Geometry Shader | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |  |  |
| [Stream Out | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |  |  |
| [DirectCompute / Compute Shader | Yes | Yes | Yes | Yes | Optional | Optional | N/A | N/A | N/A |  |  |
| [Hull and Domain Shaders | Yes | Yes | Yes | Yes | No | No | No | No | No |  |  |
| Feature \ Feature Level | 12_10 | 12_00 | 11_11 | 11_0 | 10_1 | 10_0 | 9_37 | 9_2 | 9_1 |  |  |
| [Texture Resource Arrays | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |  |  |
| [Cubemap Resource Arrays | Yes | Yes | Yes | Yes | Yes | No | No | No | No |  |  |
| [BC4/BC5 Compression | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |  |  |
| [BC6H/BC7 Compression | Yes | Yes | Yes | Yes | No | No | No | No | No |  |  |
| [Alpha-to-coverage | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |  |  |
| [Extended Formats (BGRA, and so on) | Yes | Yes | Yes | Yes | Optional | Optional | Yes | Yes | Yes |  |  |
| [10-bit XR High Color Format | Yes | Yes | Yes | Yes | Optional | Optional | N/A | N/A | N/A |  |  |
| [Logic Operations (Output Merger) | Yes | Yes | Yes | Optional1 | Optional1 | Optional1 | No | No | No |  |  |
| Target-independent rasterization | Yes | Yes | Yes | Yes | Yes | No | No | No | No | No | No |
| [Multiple render target(MRT) with ForcedSampleCount 1 | Yes | Optional1 | Optional1 | Optional1 | No | No | No |  |  |  |  |
| UAV slots | 64 | 64 | 64 | 8 | 1 | 1 | N/A | N/A | N/A |  |  |
| UAVs at every stage | Yes | Yes | Yes | No | No | No | N/A | N/A | N/A |  |  |
| Feature \ Feature Level | 12_10 | 12_00 | 11_11 | 11_0 | 10_1 | 10_0 | 9_37 | 9_2 | 9_1 |  |  |
| [Max forced sample count for UAV-only rendering | 16 | 16 | 16 | 8 | N/A | N/A | N/A | N/A | N/A |  |  |
| Constant buffer offsetting and partial updates | Yes | Yes | Yes | Optional1 | Optional1 | Optional1 | Yes1 | Yes1 | Yes1 |  |  |
| 16 bits per pixel (bpp) formats | Yes | Yes | Yes | Optional1 | Optional1 | Optional1 | Optional1 | Optional1 | Optional1 |  |  |
| Max Texture Dimension | 16384 | 16384 | 16384 | 16384 | 8192 | 8192 | 4096 | 2048 | 2048 |  |  |
| Max Cubemap Dimension | 16384 | 16384 | 16384 | 16384 | 8192 | 8192 | 4096 | 512 | 512 |  |  |
| Max Volume Extent | 2048 | 2048 | 2048 | 2048 | 2048 | 2048 | 256 | 256 | 256 |  |  |
| Max Texture Repeat | 16384 | 16384 | 16384 | 16384 | 8192 | 8192 | 8192 | 2048 | 128 |  |  |
| Max Anisotropy | 16 | 16 | 16 | 16 | 16 | 16 | 16 | 16 | 2 |  |  |
| Max Primitive Count | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 1048575 | 1048575 | 65535 |  |  |
| Max Vertex Index | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 2^32 – 1 | 1048575 | 1048575 | 65534 |  |  |
| Max Input Slots | 32 | 32 | 32 | 32 | 32 | 16 | 16 | 16 | 16 |  |  |
| Simultaneous Render Targets | 8 | 8 | 8 | 8 | 8 | 8 | 4 | 1 | 1 |  |  |
| Feature \ Feature Level | 12_28 | 12_10 | 11_11 | 11_0 | 10_1 | 10_0 | 9_37 | 9_2 | 9_1 |  |  |
| Occlusion Queries | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | No |  |  |
| Separate Alpha Blend | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | No |  |  |
| Mirror Once | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | No |  |  |
| Overlapping Vertex Elements | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | No |  |  |
| Independent Write Masks | Yes | Yes | Yes | Yes | Yes | Yes | Yes | No | No |  |  |
| Instancing | Yes | Yes | Yes | Yes | Yes | Yes | Yes7 | No | No |  |  |
| Nonpowers-of-2 conditionally3 | No | No | No | No | No | No | Yes | Yes | Yes |  |  |
| Nonpowers-of-2 unconditionally4 | Yes | Yes | Yes | Yes | Yes | Yes | No | No | No |  |  |
## Footnotes for the tables


0Requires the Direct3D 11.3 or Direct3D 12 runtime.

1Requires the Direct3D 11.1 runtime.

2Shader model 5.0 and above can optionally support double-precision shaders, extended double-precision shaders, theSAD4shader instruction, and partial-precision shaders. To determine the shader model 5.0 options that are available for DirectX 11, call[ID3D11Device::CheckFeatureSupport. Some compatibility depends on what hardware you are running on. Shader model 5.1 and above are only supported through the DirectX 12 API, regardless of the feature level that's being used. DirectX 11 only supports up to shader model 5.0. The DirectX 12 API only goes down to feature level 11_0.

3At feature levels 9_1, 9_2 and 9_3, the display device supports the use of 2-D textures with dimensions that are not powers of two under two conditions. First, only one MIP-map level for each texture can be created, and second, no wrap sampler modes for textures are allowed (that is, theAddressU,AddressV, andAddressWmembers of[D3D11_SAMPLER_DESCcannot be set to[D3D11_TEXTURE_ADDRESS_WRAP).

4At feature levels 10_0, 10_1 and 11_0, the display device unconditionally supports the use of 2-D textures with dimensions that are not powers of two.

5Vertex Shader 2a with 256 instructions, 32 temporary registers, static flow control of depth 4, dynamic flow control of depth 24, and D3DVS20CAPS_PREDICATION. Pixel Shader 2x with 512 instructions, 32 temporary registers, static flow control of depth 4, dynamic flow control of depth 24, D3DPS20CAPS_ARBITRARYSWIZZLE, D3DPS20CAPS_GRADIENTINSTRUCTIONS, D3DPS20CAPS_PREDICATION, D3DPS20CAPS_NODEPENDENTREADLIMIT, and D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT.

6Higher tiers optional.

7For Feature Level 9_3, the only rendering methods supported areDraw,DrawIndexed, andDrawIndexInstanced. Also for Feature Level 9_3, point list rendering is supported only for rendering viaDraw.

8Supported by Windows 11.

9In the Direct3D 12 API there are limits on the number of descriptors in a CBV/SRV/UAV heap. See[Hardware Tiersfor details. Separately, there's a limit on the number of UAVs in all descriptor tables across all stages, which is based on[resource binding tier.

10A 64-bit process requires 40 bits of address space available per resource and per process. A 32-bit process might be limited to 31 bits of address space. There are two capabilities (caps) available in the API—per-process and per-resource. Per-process address space is always greater than or equal to the per-resource address space.

For details of format support at different hardware feature levels, refer to:
- [DXGI format support for Direct3D Feature Level 12.1 Hardware
- [DXGI format support for Direct3D Feature Level 12.0 Hardware
- [DXGI format support for Direct3D Feature Level 11.1 Hardware
- [DXGI format support for Direct3D Feature Level 11.0 Hardware
- [Hardware support for Direct3D 10Level9 Formats
- [Hardware support for Direct3D 10.1 Formats
- [Hardware support for Direct3D 10 Formats
## Related topics

- [Direct3D 11 on downlevel hardware
- [Hardware feature levels (Direct3D 12)
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2023-07-24