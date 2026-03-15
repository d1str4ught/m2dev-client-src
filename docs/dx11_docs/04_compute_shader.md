# 04 Compute Shader

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Compute Shader Overview
FeedbackSummarize this article for me
## 


A compute shader is a programmable shader stage that expands Microsoft Direct3D 11 beyond graphics programming. The compute shader technology is also known as the DirectCompute technology.

Like other programmable shaders (vertex and geometry shaders for example), a compute shader is designed and implemented with[HLSLbut that is just about where the similarity ends. A compute shader provides high-speed general purpose computing and takes advantage of the large numbers of parallel processors on the graphics processing unit (GPU). The compute shader provides memory sharing and thread synchronization features to allow more effective parallel programming methods. You call the[ID3D11DeviceContext::Dispatchor[ID3D11DeviceContext::DispatchIndirectmethod to execute commands in a compute shader. A compute shader can run on many threads in parallel.
## Using Compute Shader on Direct3D 10.x Hardware


A compute shader on Microsoft Direct3D 10 is also known as DirectCompute 4.x.

If you use the Direct3D 11 API and updated drivers,[feature level10 and 10.1 Direct3D hardware can optionally support a limited form of DirectCompute that uses the cs_4_0 and cs_4_1[profiles. When you use DirectCompute on this hardware, keep the following limitations in mind:
- The maximum number of threads is limited to D3D11_CS_4_X_THREAD_GROUP_MAX_THREADS_PER_GROUP (768) per group.
- The X and Y dimension ofnumthreadsis limited to D3D11_CS_4_X_THREAD_GROUP_MAX_X (768) and D3D11_CS_4_X_THREAD_GROUP_MAX_Y (768).
- The Z dimension ofnumthreadsis limited to 1.
- The Z dimension of dispatch is limited to D3D11_CS_4_X_DISPATCH_MAX_THREAD_GROUPS_IN_Z_DIMENSION (1).
- Only one unordered-access view can be bound to the shader (D3D11_CS_4_X_UAV_REGISTER_COUNT is 1).
- Only[RWStructuredBuffers and[RWByteAddressBuffers are available as unordered-access views.
- A thread can only access its own region in groupshared memory for writing, though it can read from any location.
- [SV_GroupIndexor[SV_GroupThreadIDmust be used when accessinggroupsharedmemory for writing.
- Groupsharedmemory is limited to 16KB per group.
- A single thread is limited to a 256 byte region ofgroupsharedmemory for writing.
- No atomic instructions are available.
- No double-precision values are available.
## Using Compute Shader on Direct3D 11.x Hardware


A compute shader on Direct3D 11 is also known as DirectCompute 5.0.

When you use DirectCompute with cs_5_0[profiles, keep the following items in mind:
- The maximum number of threads is limited to D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP (1024) per group.
- The X and Y dimension ofnumthreadsis limited to D3D11_CS_THREAD_GROUP_MAX_X (1024) and D3D11_CS_THREAD_GROUP_MAX_Y (1024).
- The Z dimension ofnumthreadsis limited to D3D11_CS_THREAD_GROUP_MAX_Z (64).
- The maximum dimension of dispatch is limited to D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION (65535).
- The maximum number of unordered-access views that can be bound to a shader is D3D11_PS_CS_UAV_REGISTER_COUNT (8).
- Supports[RWStructuredBuffers,[RWByteAddressBuffers, and typed unordered-access views ([RWTexture1D,[RWTexture2D,[RWTexture3D, and so on).
- Atomic instructions are available.
- Double-precision support might be available. For information about how to determine whether double-precision is available, see[D3D11_FEATURE_DOUBLES.
## In this section


| Topic | Description |
| [New Resource Types
 | Several new resource types have been added in Direct3D 11.
 |
| [Accessing Resources
 | There are several ways to access[resources.
 |
| [Atomic Functions
 | To access a new resource type or shared memory, use an interlocked intrinsic function. Interlocked functions are guaranteed to operate atomically. That is, they are guaranteed to occur in the order programmed. This section lists the atomic functions.
 |
## Related topics


[Graphics Pipeline

[How To: Create a Compute Shader
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2021-04-09