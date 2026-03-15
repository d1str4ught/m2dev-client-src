# 07 Dx11 1 Features

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-1-features](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-1-features)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Direct3D 11.1 Features
FeedbackSummarize this article for me
## 


The following functionality has been added in Direct3D 11.1, which is included with Windows 8, Windows RT, and Windows Server 2012. Partial support for[Direct3D 11.1is available on Windows 7 and Windows Server 2008 R2 via the[Platform Update for Windows 7, which is available through the[Platform Update for Windows 7.
- [Shader tracing and compiler enhancements
- [Direct3D device sharing
- [Check support of new Direct3D 11.1 features and formats
- [Use HLSL minimum precision
- [Specify user clip planes in HLSL on feature level 9 and higher
- [Create larger constant buffers than a shader can access
- [Use logical operations in a render target
- [Force the sample count to create a rasterizer state
- [Process video resources with shaders
- [Extended support for shared Texture2D resources
- [Change subresources with new copy options
- [Discard resources and resource views
- [Support a larger number of UAVs
- [Bind a subrange of a constant buffer to a shader
- [Retrieve the subrange of a constant buffer that is bound to a shader
- [Clear all or part of a resource view
- [Map SRVs of dynamic buffers with NO_OVERWRITE
- [Use UAVs at every pipeline stage
- [Extended support for WARP devices
- [Use Direct3D in Session 0 processes
- [Support for shadow buffer on feature level 9
- [Related topics
## Shader tracing and compiler enhancements


Direct3D 11.1 lets you use shader tracing to ensure that your code is performing as intended and if it isn’t you can discover and remedy the problem. The Windows Software Development Kit (SDK) for Windows 8 contains HLSL compiler enhancements. Shader tracing and the HLSL compiler are implemented in D3dcompiler_nn.dll.

The shader tracing API and the enhancements to the HLSL compiler consists of the following methods and functions.
- [ID3D11RefDefaultTrackingOptions::SetTrackingOptions
- [ID3D11RefTrackingOptions::SetTrackingOptions
- [ID3D11TracingDevice::SetShaderTrackingOptions
- [ID3D11TracingDevice::SetShaderTrackingOptionsByType
- [ID3D11ShaderTraceFactory::CreateShaderTrace
- [ID3D11ShaderTrace::TraceReady
- [ID3D11ShaderTrace::ResetTrace
- [ID3D11ShaderTrace::GetTraceStats
- [ID3D11ShaderTrace::PSSelectStamp
- [ID3D11ShaderTrace::GetInitialRegisterContents
- [ID3D11ShaderTrace::GetStep
- [ID3D11ShaderTrace::GetWrittenRegister
- [ID3D11ShaderTrace::GetReadRegister
- [D3DCompile2
- [D3DCompileFromFile
- [D3DDisassemble11Trace
- [D3DDisassembleRegion
- [D3DGetTraceInstructionOffsets
- [D3DReadFileToBlob
- [D3DSetBlobPart
- [D3DWriteBlobToFile

The D3dcompiler.lib library requires D3dcompiler_nn.dll. This DLL is not part of Windows 8; it is in the \bin folder of the Windows SDK for Windows 8 along with the Fxc.exe command-line version of the HLSL compiler.

Note

While you can use this library and DLL combination for development, you can't deploy Windows Store apps that use this combination. Therefore, you must instead compile HLSL shaders before you ship your Windows Store app. You can write HLSL compilation binaries to disk, or the compiler can generate headers with static byte arrays that contain the shader blob data. You use the[ID3DBlobinterface to access the blob data. To develop your Windows Store app, call[D3DCompile2or[D3DCompileFromFileto compile the raw HLSL source, and then feed the resulting blob data to Direct3D.


## Direct3D device sharing


Direct3D 11.1 enables Direct3D 10 APIs and Direct3D 11 APIs to use one underlying rendering device.

This Direct3D 11.1 feature consists of the following methods and interface.
- [ID3D11Device1::CreateDeviceContextState
- [ID3D11DeviceContext1::SwapDeviceContextState
- [ID3DDeviceContextState
## Check support of new Direct3D 11.1 features and formats


Direct3D 11.1 lets you check for new features that the graphics driver might support and new ways that a format is supported on a device. Microsoft DirectX Graphics Infrastructure (DXGI) 1.2 also specifies new[DXGI_FORMATvalues.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11Device::CheckFeatureSupportwith[D3D11_FEATURE_DATA_D3D11_OPTIONS,[D3D11_FEATURE_DATA_ARCHITECTURE_INFO,[D3D11_FEATURE_DATA_D3D9_OPTIONS,[D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORT, and[D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORTstructures
- [ID3D11Device::CheckFormatSupportwith[D3D11_FORMAT_SUPPORT_DECODER_OUTPUT,[D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_OUTPUT,[D3D11_FORMAT_SUPPORT_VIDEO_PROCESSOR_INPUT,[D3D11_FORMAT_SUPPORT_VIDEO_ENCODER, and[D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP
## Use HLSL minimum precision


Starting with Windows 8, graphics drivers can implement minimum precision[HLSL scalar data typesby using any precision greater than or equal to their specified bit precision. When your HLSL minimum precision shader code is used on hardware that implements HLSL minimum precision, you use less memory bandwidth and as a result you also use less system power.

You can query for the minimum precision support that the graphics driver provides by calling[ID3D11Device::CheckFeatureSupportwith the[D3D11_FEATURE_SHADER_MIN_PRECISION_SUPPORTvalue. In thisID3D11Device::CheckFeatureSupportcall, pass a pointer to a[D3D11_FEATURE_DATA_SHADER_MIN_PRECISION_SUPPORTstructure thatID3D11Device::CheckFeatureSupportfills with the minimum precision levels that the driver supports for the pixel shader stage and for other shader stages. The returned info just indicates that the graphics hardware can perform HLSL operations at a lower precision than the standard 32-bit float precision, but doesn’t guarantee that the graphics hardware will actually run at a lower precision.

You don't need to author multiple shaders that do and don't use minimum precision. Instead, create shaders with minimum precision, and the minimum precision variables behave at full 32-bit precision if the graphics driver reports that it doesn't support any minimum precision. For more info about HLSL minimum precision, see[Using HLSL minimum precision.

HLSL minimum precision shaders don't work on operating systems earlier than Windows 8.
## Specify user clip planes in HLSL on feature level 9 and higher


Starting with Windows 8, you can use theclipplanesfunction attribute in an HLSL[function declarationrather than[SV_ClipDistanceto make your shader work on[feature level9_x as well as feature level 10 and higher. For more info, see[User clip planes on feature level 9 hardware.
## Create larger constant buffers than a shader can access


Direct3D 11.1 lets you create constant buffers that are larger than the maximum constant buffer size that a shader can access (4096 32-bit*4-component constants – 64KB). Later, when you bind the buffers to the pipeline, for example, via[PSSetConstantBuffersor[PSSetConstantBuffers1, you can specify a range of buffers that the shader can access that fits within the 4096 limit.

Direct3D 11.1 updates the[ID3D11Device::CreateBuffermethod for this feature.
## Use logical operations in a render target


Direct3D 11.1 lets you use logical operations rather than blending in a render target. However, you can’t mix logic operations with blending across multiple render targets.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11Device1::CreateBlendState1with[D3D11_LOGIC_OP
## Force the sample count to create a rasterizer state


Direct3D 11.1 lets you specify a force sample count when you create a rasterizer state.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11Device1::CreateRasterizerState1

Note

If you want to render with the sample count forced to 1 or greater, you must follow these guidelines:
- Don't bind depth-stencil views.
- Disable depth testing.
- Ensure the shader doesn't output depth.
- If you have any render-target views bound ([D3D11_BIND_RENDER_TARGET) and you forced the sample count to greater than 1, ensure that every render target has only a single sample.
- Don't operate the shader at sample frequency. Therefore,[ID3D11ShaderReflection::IsSampleFrequencyShaderreturnsFALSE.

Otherwise, rendering behavior is undefined. For info about how to configure depth-stencil, see[Configuring Depth-Stencil Functionality.


## Process video resources with shaders


Direct3D 11.1 lets you create views (SRV/RTV/UAV) to video resources so that Direct3D shaders can process those video resources. The format of an underlying video resource restricts the formats that the view can use. The[DXGI_FORMATenumeration contains new video resource format values. TheseDXGI_FORMATvalues specify the valid view formats that you can create and how the Direct3D 11.1 runtime maps the view. You can create multiple views of different parts of the same surface, and depending on the format, the sizes of the views can differ from each other.

Direct3D 11.1 updates the following methods for this feature.
- [ID3D11Device::CreateShaderResourceView
- [ID3D11Device::CreateRenderTargetView
- [ID3D11Device::CreateUnorderedAccessView
## Extended support for shared Texture2D resources


Direct3D 11.1 guarantees that you can share Texture2D resources that you created with particular resource types and formats. To share Texture2D resources, use the[D3D11_RESOURCE_MISC_SHARED,[D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX, or a combination of theD3D11_RESOURCE_MISC_SHARED_KEYEDMUTEXand[D3D11_RESOURCE_MISC_SHARED_NTHANDLE(new for Windows 8) flags when you create those resources.

Direct3D 11.1 guarantees that you can share Texture2D resources that you created with these[DXGI_FORMATvalues:
- DXGI_FORMAT_R8G8B8A8_UNORM
- DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
- DXGI_FORMAT_B8G8R8A8_UNORM
- DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
- DXGI_FORMAT_B8G8R8X8_UNORM
- DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
- DXGI_FORMAT_R10G10B10A2_UNORM
- DXGI_FORMAT_R16G16B16A16_FLOAT

In addition, Direct3D 11.1 guarantees that you can share Texture2D resources that you created with a mipmap level of 1, array size of 1, bind flags of[D3D11_BIND_SHADER_RESOURCEand[D3D11_BIND_RENDER_TARGETcombined, usage default ([D3D11_USAGE_DEFAULT), and only these[D3D11_RESOURCE_MISC_FLAGvalues:
- [D3D11_RESOURCE_MISC_SHARED
- [D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
- [D3D11_RESOURCE_MISC_SHARED_NTHANDLE
- [D3D11_RESOURCE_MISC_GDI_COMPATIBLE

Direct3D 11.1 lets you share a greater variety of Texture2D resource types and formats. You can query for whether the graphics driver and hardware support extended Texture2D resource sharing by calling[ID3D11Device::CheckFeatureSupportwith the[D3D11_FEATURE_D3D11_OPTIONSvalue. In thisID3D11Device::CheckFeatureSupportcall, pass a pointer to a[D3D11_FEATURE_DATA_D3D11_OPTIONSstructure.ID3D11Device::CheckFeatureSupportsets theExtendedResourceSharingmember ofD3D11_FEATURE_DATA_D3D11_OPTIONStoTRUEif the hardware and driver support extended Texture2D resource sharing.

If[ID3D11Device::CheckFeatureSupportreturnsTRUEinExtendedResourceSharing, you can share resources that you created with these[DXGI_FORMATvalues:
- DXGI_FORMAT_R32G32B32A32_TYPELESS
- DXGI_FORMAT_R32G32B32A32_FLOAT
- DXGI_FORMAT_R32G32B32A32_UINT
- DXGI_FORMAT_R32G32B32A32_SINT
- DXGI_FORMAT_R16G16B16A16_TYPELESS
- DXGI_FORMAT_R16G16B16A16_FLOAT
- DXGI_FORMAT_R16G16B16A16_UNORM
- DXGI_FORMAT_R16G16B16A16_UINT
- DXGI_FORMAT_R16G16B16A16_SNORM
- DXGI_FORMAT_R16G16B16A16_SINT
- DXGI_FORMAT_R10G10B10A2_UNORM
- DXGI_FORMAT_R10G10B10A2_UINT
- DXGI_FORMAT_R8G8B8A8_TYPELESS
- DXGI_FORMAT_R8G8B8A8_UNORM
- DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
- DXGI_FORMAT_R8G8B8A8_UINT
- DXGI_FORMAT_R8G8B8A8_SNORM
- DXGI_FORMAT_R8G8B8A8_SINT
- DXGI_FORMAT_B8G8R8A8_TYPELESS
- DXGI_FORMAT_B8G8R8A8_UNORM
- DXGI_FORMAT_B8G8R8X8_UNORM
- DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
- DXGI_FORMAT_B8G8R8X8_TYPELESS
- DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
- DXGI_FORMAT_R32_TYPELESS
- DXGI_FORMAT_R32_FLOAT
- DXGI_FORMAT_R32_UINT
- DXGI_FORMAT_R32_SINT
- DXGI_FORMAT_R16_TYPELESS
- DXGI_FORMAT_R16_FLOAT
- DXGI_FORMAT_R16_UNORM
- DXGI_FORMAT_R16_UINT
- DXGI_FORMAT_R16_SNORM
- DXGI_FORMAT_R16_SINT
- DXGI_FORMAT_R8_TYPELESS
- DXGI_FORMAT_R8_UNORM
- DXGI_FORMAT_R8_UINT
- DXGI_FORMAT_R8_SNORM
- DXGI_FORMAT_R8_SINT
- DXGI_FORMAT_A8_UNORM
- DXGI_FORMAT_AYUV
- DXGI_FORMAT_YUY2
- DXGI_FORMAT_NV12
- DXGI_FORMAT_NV11
- DXGI_FORMAT_P016
- DXGI_FORMAT_P010
- DXGI_FORMAT_Y216
- DXGI_FORMAT_Y210
- DXGI_FORMAT_Y416
- DXGI_FORMAT_Y410

If[ID3D11Device::CheckFeatureSupportreturnsTRUEinExtendedResourceSharing, you can share resources that you created with these features and flags:
- [D3D11_USAGE_DEFAULT
- [D3D11_BIND_SHADER_RESOURCE
- [D3D11_BIND_RENDER_TARGET
- [D3D11_RESOURCE_MISC_GENERATE_MIPS
- [D3D11_BIND_UNORDERED_ACCESS
- Mipmap levels (one or more levels) in the 2D texture resources (specified in theMipLevelsmember of[D3D11_TEXTURE2D_DESC)
- Arrays of 2D texture resources (one or more textures) (specified in theArraySizemember of[D3D11_TEXTURE2D_DESC)
- [D3D11_BIND_DECODER
- [D3D11_RESOURCE_MISC_RESTRICTED_CONTENT
- [D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE
- [D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER

Note

WhenExtendedResourceSharingisTRUE, you have more flexibility when you specify bind flags for sharing Texture2D resources. Not only does the graphics driver and hardware support more bind flags but also more possible combinations of bind flags. For example, you can specify just[D3D11_BIND_RENDER_TARGETor no bind flags, and so on.


Even if[ID3D11Device::CheckFeatureSupportreturnsTRUEinExtendedResourceSharing, you still can't share resources that you created with these features and flags:
- [D3D11_BIND_DEPTH_STENCIL
- 2D texture resources in multisample antialiasing (MSAA) mode (specified with[DXGI_SAMPLE_DESC)
- [D3D11_RESOURCE_MISC_RESOURCE_CLAMP
- [D3D11_USAGE_IMMUTABLE,[D3D11_USAGE_DYNAMIC, or[D3D11_USAGE_STAGING(that is, mappable)
- [D3D11_RESOURCE_MISC_TEXTURECUBE
## Change subresources with new copy options


Direct3D 11.1 lets you use new copy flags to copy and update subresources. When you copy a subresource, the source and destination resources can be identical and the source and destination regions can overlap.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11DeviceContext1::CopySubresourceRegion1
- [ID3D11DeviceContext1::UpdateSubresource1
## Discard resources and resource views


Direct3D 11.1 lets you discard resources and views of resources from the device context. This new functionality informs the GPU that existing content in resources or resource views are no longer needed.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11DeviceContext1::DiscardResource
- [ID3D11DeviceContext1::DiscardView
- [ID3D11DeviceContext1::DiscardView1
## Support a larger number of UAVs


Direct3D 11.1 lets you use a larger number of UAVs when you bind resources to the[output-merger stageand when you set an array of views for an unordered resource.

Direct3D 11.1 updates the following methods for this feature.
- [ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews
- [ID3D11DeviceContext::CSSetUnorderedAccessViews
## Bind a subrange of a constant buffer to a shader


Direct3D 11.1 lets you bind a subrange of a constant buffer for a shader to access. You can supply a larger constant buffer and specify the subrange that the shader can use.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11DeviceContext1::CSSetConstantBuffers1
- [ID3D11DeviceContext1::DSSetConstantBuffers1
- [ID3D11DeviceContext1::GSSetConstantBuffers1
- [ID3D11DeviceContext1::HSSetConstantBuffers1
- [ID3D11DeviceContext1::PSSetConstantBuffers1
- [ID3D11DeviceContext1::VSSetConstantBuffers1
## Retrieve the subrange of a constant buffer that is bound to a shader


Direct3D 11.1 lets you retrieve the subrange of a constant buffer that is bound to a shader.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11DeviceContext1::CSGetConstantBuffers1
- [ID3D11DeviceContext1::DSGetConstantBuffers1
- [ID3D11DeviceContext1::GSGetConstantBuffers1
- [ID3D11DeviceContext1::HSGetConstantBuffers1
- [ID3D11DeviceContext1::PSGetConstantBuffers1
- [ID3D11DeviceContext1::VSGetConstantBuffers1
## Clear all or part of a resource view


Direct3D 11.1 lets you clear a resource view (RTV, UAV, or any video view of a Texture2D surface). You apply the same color value to all parts of the view.

This Direct3D 11.1 feature consists of the following API.
- [ID3D11DeviceContext1::ClearView
## Map SRVs of dynamic buffers with NO_OVERWRITE


Direct3D 11.1 lets you map shader resource views (SRV) of dynamic buffers with D3D11_MAP_WRITE_NO_OVERWRITE. The Direct3D 11 and earlier runtimes limited mapping to vertex or index buffers.

Direct3D 11.1 updates the[ID3D11DeviceContext::Mapmethod for this feature.
## Use UAVs at every pipeline stage


Direct3D 11.1 lets you use the following shader model 5.0 instructions at all shader stages that were previously used in just pixel shaders and compute shaders.
- [dcl_uav_typed
- [dcl_uav_raw
- [dcl_uav_structured
- [ld_raw
- [ld_structured
- [ld_uav_typed
- [store_raw
- [store_structured
- [store_uav_typed
- [sync_uglobal
- All atomics and immediate atomics (for example,[atomic_andand[imm_atomic_and)

Direct3D 11.1 updates the following methods for this feature.
- [ID3D11DeviceContext::CreateDomainShader
- [ID3D11DeviceContext::CreateGeometryShader
- [ID3D11DeviceContext::CreateGeometryShaderWithStreamOutput
- [ID3D11DeviceContext::CreateHullShader
- [ID3D11DeviceContext::CreateVertexShader

These instructions existed in Direct3D 11.0 in ps_5_0 and cs_5_0. Because Direct3D 11.1 makes UAVs available at all shader stages, these instructions are available at all shader stages.

If you pass compiled shaders (VS/HS/DS/HS) that use any of these instructions to a create-shader function, like[CreateVertexShader, on devices that don’t support UAVs at every stage (including existing drivers that are not implemented with this feature), the create-shader function fails. The create-shader function also fails if the shader tries to use a UAV slot beyond the set of UAV slots that the hardware supports.

The UAVs that are referenced by these instructions are shared across all pipeline stages. For example, a UAV that is bound at slot 0 at the[output-merger stageis available at slot 0 to VS/HS/DS/GS/PS.

UAV accesses that you issue from within or across shader stages that execute within a given Draw*() or that you issue from the compute shader within a Dispatch*() aren't guaranteed to finish in the order in which you issued them. But all UAV accesses finish at the end of the Draw*() or Dispatch*().
## Extended support for WARP devices


Direct3D 11.1 extends support for[WARPdevices, which you create by passing[D3D_DRIVER_TYPE_WARPin theDriverTypeparameter of[D3D11CreateDevice.

Starting with Direct3D 11.1 WARP devices support:
- All Direct3D[feature levelsfrom 9.1 through to 11.1
- [Compute shadersand[tessellation
- Shared surfaces. That is, you can fully share surfaces between WARP devices, as well as between WARP devices in different processes.

WARP devices don't support these optional features:
- [doubles
- [video encode or decode
- [minimum precision shader support

When you run a virtual machine (VM), Hyper-V, with your graphics processing unit (GPU) disabled, or without a display driver, you get a WARP device whose friendly name is "Microsoft Basic Display Adapter." This WARP device can run the full Windows experience, which includes DWM, Internet Explorer, and Windows Store apps. This WARP device also supports running legacy apps that use[DirectDrawor running apps that use Direct3D 3 through Direct3D 11.1.
## Use Direct3D in Session 0 processes


Starting with Windows 8 and Windows Server 2012, you can use most of the Direct3D APIs in Session 0 processes.

Note

These output, window, swap chain, and presentation-related APIs are not available in Session 0 processes because they don't apply to the Session 0 environment:
- [IDXGIFactory::CreateSwapChain
- [IDXGIFactory::GetWindowAssociation
- [IDXGIFactory::MakeWindowAssociation
- [IDXGIAdapter::EnumOutputs
- [ID3D11Debug::SetFeatureMask
- [ID3D11Debug::SetPresentPerRenderOpDelay
- [ID3D11Debug::SetSwapChain
- [ID3D10Debug::SetFeatureMask
- [ID3D10Debug::SetPresentPerRenderOpDelay
- [ID3D10Debug::SetSwapChain
- [D3D10CreateDeviceAndSwapChain
- [D3D10CreateDeviceAndSwapChain1
- [D3D11CreateDeviceAndSwapChain

If you call one of the preceding APIs in a Session 0 process, it returns[DXGI_ERROR_NOT_CURRENTLY_AVAILABLE.


## Support for shadow buffer on feature level 9


Use a subset of Direct3D 10_0+ shadow buffer features to implement shadow effects on[feature level9_x. For info about what you need to do to implement shadow effects on feature level 9_x, see[Implementing shadow buffers for Direct3D feature level 9.
## Related topics


[What's new in Direct3D 11


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19