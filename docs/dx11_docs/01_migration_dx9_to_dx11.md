# 01 Migration Dx9 To Dx11

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d11-programming-guide-migrating](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d11-programming-guide-migrating)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Migrating to Direct3D 11
FeedbackSummarize this article for me
## 


This section provides info for migrating to Direct3D 11 from an earlier version of Direct3D.
- [Direct3D 9 to Direct3D 11
- [Direct3D 10 to Direct3D 11
- [Enumerations and Defines
- [Structures
- [Interfaces
- [Other Related Technologies
- [Direct3D 10.1 to Direct3D 11
- [Enumerations and Defines
- [Structures
- [Interfaces
- [New Features for Direct3D 11
- [New Features for DirectX 11.1
- [New Features for DirectX 11.2
- [Related topics
## Direct3D 9 to Direct3D 11


The Direct3D 11 API builds on the infrastructural improvements made in Direct3D 10 and 10.1. Porting from Direct3D 9 to Direct3D 11 is similar to moving from Direct3D 9 to Direct3D 10. The following are the key challenges in this effort.
- Removal of all fixed function pipeline usage in favor of programmable shaders authored exclusively in HLSL (compiled via D3DCompiler instead of D3DX9).
- State management based on immutable objects rather than individual state toggles.
- Updating to comply with strict linkage requirements of vertex buffer input layouts and shader signatures.
- Associating shader resource views with all texture resources.
- Mapping all image content to a DXGI_FORMAT, including the removal of all 24-bit color formats (8/8/8), and strict RGB color ordering for most scenarios.
- Breaking up global constant state usage into several small, more efficiently updated constant buffers.

For more information about moving from Direct3D 9 to Direct3D 10, see[Direct3D 9 to Direct3D 10 Considerations.
## Direct3D 10 to Direct3D 11


Converting programs written to use the Direct3D 10 or 10.1 API is a straight-forward process as Direct3D 11 is an extension of the existing API. With only one minor exception (noted below - monochrome text filtering), all methods and functionality in Direct3D 10/10.1 is available in Direct3D 11. The outline below describes the differences between the two APIs to aid in updating existing code. The key differences here include:
- Rendering operations (Draw, state, etc.) are no longer part of the Device interface, but are instead part of the new DeviceContext interface along with resource Map/Unmap and device query methods.
- Direct3D 11 includes all enhancements and changes made between Direct3D 10.0 and 10.1
### Enumerations and Defines


| Direct3D 10 | Direct3D 11 |
| DXGI_FORMAT | [DXGI_FORMATSeveral new DXGI formats were defined.
 |
| D3D10_CREATE_DEVICE_SWITCH_TO_REF | [D3D11_CREATE_DEVICE_SWITCH_TO_REFThe switch-to-ref functionality is not supported by Direct3D 11.
 |
| D3D10_DRIVER_TYPE | [D3D_DRIVER_TYPENote that the enumeration identifiers in[D3D_DRIVER_TYPEwere redefined from the identifiers in[D3D10_DRIVER_TYPE. Therefore, you should be sure to use the enumeration identifiers instead of literal numbers.
[D3D_DRIVER_TYPEis defined in D3Dcommon.h.
 |
| D3D10_RESOURCE_MISC_FLAG | [D3D11_RESOURCE_MISC_FLAGNote that many of these flags were redefined, so be sure to use enumeration identifiers instead of literal numbers.
 |
| D3D10_FILTER | [D3D11_FILTERNote that text filtering D3D10_FILTER_TEXT_1BIT was removed from Direct3D 11. See DirectWrite.
 |
| D3D10_COUNTER | [D3D11_COUNTERNote that the vendor-neutral counters were removed for Direct3D 11 as they were rarely supported.
 |
| D3D10_x | D3D11_x Many enumerations and defines are the same, have larger limits, or have additional values.
 |
### Structures


| Direct3D 10 | Direct3D 11 |
| D3D10_SO_DECLARATION_ENTRY | [D3D11_SO_DECLARATION_ENTRYAdds Stream.
 |
| D3D10_BLEND_DESC | [D3D11_BLEND_DESCNote this structure changed significantly from 10 to 10.1 to provide per render target blend state
 |
| D3D10_BUFFER_DESC | [D3D11_BUFFER_DESCAdds a StructureByteStride for use with Compute Shader resources
 |
| D3D10_SHADER_RESOURCE_VIEW_DESC | [D3D11_SHADER_RESOURCE_VIEW_DESCNote this structure had additional union members added for 10.1
 |
| D3D10_DEPTH_STENCIL_VIEW_DESC | [D3D11_DEPTH_STENCIL_VIEW_DESCHas a new Flags member.
 |
| D3D10_QUERY_DATA_PIPELINE_STATISTICS | [D3D11_QUERY_DATA_PIPELINE_STATISTICSAdds several new shader stage counters.
 |
| D3D10_x | D3D11_x Many structures are identical between the two APIs.
 |
### Interfaces


| Direct3D 10 | Direct3D 11 |
| ID3D10Device | [ID3D11Deviceand[ID3D11DeviceContext
The device interface was split into these two parts. For quick porting you can make use of[ID3D11Device::GetImmediateContext.
"ID3D10Device::GetTextFilterSize" and "SetTextFilerSize" methods no longer exist. See DirectWrite.
Create*Shader takes an additional optional parameter for[ID3D11ClassLinkage.
*SetShader and *GetShader take additional optional parameters for[ID3D11ClassInstance(s).
[CreateGeometryShaderWithStreamOutputtakes an array and count for multiple output stream strides.
The limit for the NumEntries parameter of[CreateGeometryShaderWithStreamOutputhas increased to D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT in Direct3D 11.
 |
| ID3D10Buffer | [ID3D11Buffer |
| ID3D10SwitchToRef | [ID3D11SwitchToRefSwitch-to-ref functionality is not supported in Direct3D 11.
 |
| ID3D10Texture1D | [ID3D11Texture1D |
| ID3D10Texture2D | [ID3D11Texture2D |
| ID3D10Texture3D | [ID3D11Texture3DThe Map and Unmap methods were moved to[ID3D11DeviceContext, and all Map methods use[D3D11_MAPPED_SUBRESOURCEinstead of a void**.
 |
| ID3D10Asynchronous | [ID3D11AsynchronousBegin, End, and GetData were moved to[ID3D11DeviceContext.
 |
| ID3D10x | ID3D11x Many interfaces are identical between the two APIs.
 |
### Other Related Technologies


| 10/10.1 Solution | 11 Solution |
| HLSL Complier (D3D10Compile*, D3DX10Compile*) and shader reflection APIs | D3DCompiler (see D3DCompiler.h)Note:For Windows Store apps, the[D3DCompiler APIsare supported only for development, not deployment.
 |
| Effects 10 | [Effects 11is available as shared source online.Note:This solution is not suited to Windows Store apps because it requires the[D3DCompiler APIsat runtime (deployment).
 |
| D3DX9/D3DX10 Math | [DirectXMath |
| D3DX10 | D3DX11 in the legacy DirectX SDK[DirectXTex,[DirectXTK, and[DirectXMeshoffer alternatives to many technologies in the legacy D3DX10 and D3DX11 libraries.
[Direct2Dand[DirectWriteoffer high-quality support for rendering styled lines and fonts.
 |

For info about the legacy DirectX SDK, see[Where is the DirectX SDK?.
## Direct3D 10.1 to Direct3D 11


Direct3D 10.1 is an extension of the Direct3D 10 interface, and all features of Direct3D 10.1 are available in Direct3D 11. Most of the porting from 10.1 to 11 is already addressed above moving from 10 to 11.
### Enumerations and Defines


| Direct3D 10.1 | Direct3D 11 |
| D3D10_FEATURE_LEVEL1 | [D3D_FEATURE_LEVELIdentical but defined in D3DCommon.h plus the addition of D3D_FEATURE_LEVEL_11_0 for 11 class hardware along with D3D_FEATURE_LEVEL_9_1, D3D_FEATURE_LEVEL_9_2, and D3D_FEATURE_LEVEL_9_3 for 10level9
(D3D10_FEATURE_LEVEL_9_1, D3D10_FEATURE_LEVEL_9_2, and D3D10_FEATURE_LEVEL_9_3 were also added for 10.1 to d3d10_1.h)
 |
### Structures


| Direct3D 10.1 | Direct3D 11 |
| D3D10_BLEND_DESC1 | [D3D11_BLEND_DESCThe 11 version is identical to 10.1.
 |
| D3D10_SHADER_RESOURCE_VIEW_DESC1 | [D3D11_SHADER_RESOURCE_VIEW_DESCThe 11 version is identical to 10.1.
 |
### Interfaces


| Direct3D 10.1 | Direct3D 11 |
| ID3D10Device1 | [ID3D11Deviceand[ID3D11DeviceContext
The device interface was split into these two parts. For quick porting you can make use of[ID3D11Device::GetImmediateContext.
" ID3D10Device::GetTextFilterSize" and "SetTextFilerSize" methods no longer exist. See DirectWrite.
Create*Shader takes an additional optional parameter for[ID3D11ClassLinkage.
*SetShader and *GetShader take additional optional parameters for[ID3D11ClassInstance(s).
[CreateGeometryShaderWithStreamOutputtakes an array and count for multiple output stream strides.
The limit for the NumEntries parameter of[CreateGeometryShaderWithStreamOutputhas increased to D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT in Direct3D 11.
 |
| ID3D10BlendState1 | [ID3D11BlendState |
| ID3D10ShaderResourceView1 | [ID3D11ShaderResourceView |
## New Features for Direct3D 11


Once your code is updated to use the Direct3D 11 API, there are numerous[new featuresto consider.
- Multithreaded rendering through command lists and multiple contexts
- Implementing advanced algorithms using Compute Shader (using 4.0, 4.1, or 5.0 shader profiles)
- New 11 class hardware features:
- HLSL Shader Model 5.0
- Dynamic Shader Linkage
- Tessellation through Hull and Domain shaders
- New block compression formats: BC6H for HDR images, BC7 for higher-fidelity standard images
- Utilizing[10level9 technologyfor rendering on many Shader Model 2.0 and Shader Model 3.0 devices through the DIrect3D 11 API for lower-end video hardware support on Windows Vista and Windows 7.
- Leveraging the WARP software rendering device.
## New Features for DirectX 11.1


Windows 8 includes further DirectX graphics enhancements to consider when you implement your DirectX graphics code, which include[Direct3D 11.1,[DXGI 1.2,[Windows Display Driver Model (WDDM) 1.2,[feature level11.1 hardware, Direct2D device contexts, and other improvements.

Partial support for[Direct3D 11.1is available on Windows 7 as well via the[Platform Update for Windows 7, which is available through the[Platform Update for Windows 7.
## New Features for DirectX 11.2


The Windows 8.1 includes[Direct3D 11.2,[DXGI 1.3, and other improvements.
## Related topics


[Programming Guide for Direct3D 11

[Effects (Direct3D 11)
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2021-10-06