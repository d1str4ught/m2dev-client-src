# 07 Dx11 2 Features

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-2-features](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-2-features)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Direct3D 11.2 Features
FeedbackSummarize this article for me
## 


The following functionality has been added in Direct3D 11.2, which is included with Windows 8.1, Windows RT 8.1, and Windows Server 2012 R2.
- [Tiled resources
- [Check tiled resources support
- [Extended support for WARP devices
- [Annotate graphics commands
- [HLSL shader linking
- [Function linking graph (FLG)
- [Inbox HLSL compiler
- [Related topics
## Tiled resources


Direct3D 11.2 lets you create tiled resources that can be thought of as large logical resources that use small amounts of physical memory. Tiled resources are useful (for example) with terrain in games, and app UI.

Tiled resources are created by specifying the[D3D11_RESOURCE_MISC_TILEDflag. To work with tiled resource, use these API:
- [ID3D11Device2::GetResourceTiling
- [ID3D11DeviceContext2::UpdateTiles
- [ID3D11DeviceContext2::UpdateTileMappings
- [ID3D11DeviceContext2::CopyTiles
- [ID3D11DeviceContext2::CopyTileMappings
- [ID3D11DeviceContext2::ResizeTilePool
- [ID3D11DeviceContext2::TiledResourceBarrier
- D3D11_DEBUG_FEATURE_DISABLE_TILED_RESOURCE_MAPPING_TRACKING_AND_VALIDATIONflag with[ID3D11Debug::SetFeatureMask

For more info about tiled resources, see[Tiled resources.
### Check tiled resources support


Before you use tiled resources, you need to find out if the device supports tiled resources. Here's how you check for support for tiled resources:
```
HRESULT hr = D3D11CreateDevice(
    nullptr,                    // Specify nullptr to use the default adapter.
    D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
    0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
    creationFlags,              // Set debug and Direct2D compatibility flags.
    featureLevels,              // List of feature levels this app can support.
    ARRAYSIZE(featureLevels),   // Size of the list above.
    D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
    &device,                    // Returns the Direct3D device created.
    &m_d3dFeatureLevel,         // Returns feature level of device created.
    &context                    // Returns the device immediate context.
    );

if (SUCCEEDED(hr))
{
    D3D11_FEATURE_DATA_D3D11_OPTIONS1 featureData;
    DX::ThrowIfFailed(
        device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &featureData, sizeof(featureData))
        );

    m_tiledResourcesTier = featureData.TiledResourcesTier;
}

```

## Extended support for WARP devices


Direct3D 11.2 extends support for[WARPdevices, which you create by passing[D3D_DRIVER_TYPE_WARPin theDriverTypeparameter of[D3D11CreateDevice. The WARP software renderer in Direct3D 11.2 adds full support for Direct3D[feature level11_1, including[tiled resources,[IDXGIDevice3::Trim, shared BCn surfaces, minblend, and map default.[Doublesupport in HLSL shaders has also been enabled along with support for 16x MSAA.
## Annotate graphics commands


Direct3D 11.2 lets you annotate graphics commands with these API:
- [ID3D11DeviceContext2::IsAnnotationEnabled
- [ID3D11DeviceContext2::BeginEventInt
- [ID3D11DeviceContext2::SetMarkerInt
- [ID3D11DeviceContext2::EndEvent
## HLSL shader linking


Windows 8.1 adds separate compilation and linking of HLSL shaders, which allows graphics programmers to create precompiled HLSL functions, package them into libraries, and link them into full shaders at run-time. This is essentially an equivalent of C/C++ separate compilation, libraries, and linking, and it allows programmers to compose precompiled HLSL code when more information becomes available to finalize the computation. For more info about how to use shader linking, see[Using shader linking.

Complete these steps to create a final shader using dynamic linkage at run time.

To create and use shader linking
- Create a[ID3D11Linkerlinker object, which represents a linking context. A single context can't be used to produce multiple shaders; a linking context is used to produce a single shader and then the linking context is thrown away.
- Use[D3DLoadModuleto load and set libraries from their library blobs.
- Use[D3DLoadModuleto load and set an entry shader blob, or create an[FLG shader.
- Use[ID3D11Module::[CreateInstanceto create[ID3D11ModuleInstanceobjects, then call functions on these objects to rebind resources to their final slots.
- Add the libraries to the linker, then call[ID3D11Linker::[Linkto produce final shader byte code that can then be loaded and used in the runtime just like a fully precompiled and linked shader.
### Function linking graph (FLG)


Windows 8.1 also adds the Function Linking Graph (FLG). You can use FLG to construct shaders that consist of a sequence of precompiled function invocations that pass values to each other. When using the FLG, there is no need to write HLSL and invoke the HLSL compiler. Instead, the shader structure is specified programmatically using C++ API calls. FLG nodes represent input and output signatures and invocations of precompiled library functions. The order of registering the function-call nodes defines the sequence of invocations. The input signature node must be specified first, while the output signature node must be specified last. FLG edges define how values are passed from one node to another. The data types of passed values must be the same; there is no implicit type conversion. Shape and swizzling rules follow the HLSL behavior and values can only be passed forward in this sequence. For info on the FLG API, see[ID3D11FunctionLinkingGraph.
## Inbox HLSL compiler


The HLSL compiler is now inbox on Windows 8.1 and later. Now, most APIs for shader programming can be used in Windows Store apps that are built for Windows 8.1 and later. Many APIs for shader programming couldn't be used in Windows Store apps that were built for Windows 8; the reference pages for these APIs were marked with a note. But some shader APIs (for example,[D3DCompileFromFile) can still only be used to develop Windows Store apps, and not in apps that you submit to the Windows Store; the reference pages for these APIs are still marked with a note.
## Related topics


[What's new in Direct3D 11


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19