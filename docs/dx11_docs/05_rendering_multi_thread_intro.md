# 05 Rendering Multi Thread Intro

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-intro](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-intro)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Introduction to Multithreading in Direct3D 11
FeedbackSummarize this article for me
## 


Multithreading is designed to improve performance by performing work using one or more threads at the same time.

In the past, this has often been done by generating a single main thread for rendering and one or more threads for doing preparation work such as object creation, loading, processing, and so on. However, with the built in synchronization in Direct3D 11, the goal behind multithreading is to utilize every CPU and GPU cycle without making a processor wait for another processor (particularly not making the GPU wait because it directly impacts frame rate). By doing so, you can generate the most amount of work while maintaining the best frame rate. The concept of a single frame for rendering is no longer as necessary since the API implements synchronization.

Multithreading requires some form of synchronization. For example, if multiple threads that run in an application must access a single device context ([ID3D11DeviceContext), that application must use some synchronization mechanism, such as critical sections, to synchronize access to that device context. This is because processing of the render commands (generally done on the GPU) and generating the render commands (generally done on the CPU through object creation, data loading, state changing, data processing) often use the same resources (textures, shaders, pipeline state, and so on). Organizing the work across multiple threads requires synchronization to prevent one thread from modifying or reading data that is being modified by another thread.

While the use of a device context ([ID3D11DeviceContext) is not thread-safe, the use of a Direct3D 11 device ([ID3D11Device) is thread-safe. Because eachID3D11DeviceContextis single threaded, only one thread can call aID3D11DeviceContextat a time. If multiple threads must access a singleID3D11DeviceContext, they must use some synchronization mechanism, such as critical sections, to synchronize access to thatID3D11DeviceContext. However, multiple threads are not required to use critical sections or synchronization primitives to access a singleID3D11Device. Therefore, if an application usesID3D11Deviceto create resource objects, that application is not required to use synchronization to create multiple resource objects at the same time.

Multithreading support divides the API into two distinct functional areas:
- [Object Creation with Multithreading
- [Immediate and Deferred Rendering

Multithreading performance depends on the driver support.[How To: Check for Driver Supportprovides more information about querying the driver and what the results mean.

Direct3D 11 has been designed from the ground up to support multithreading. Direct3D 10 implements limited support for multithreading using the[thread-safe layer. This page lists the behavior differences between the two versions of DirectX:[Threading Differences between Direct3D Versions.
## Multithreading and DXGI


Only one thread at a time should use the immediate context. However, your application should also use that same thread for Microsoft DirectX Graphics Infrastructure (DXGI) operations, especially when the application makes calls to the[IDXGISwapChain::Presentmethod.

Note

It is invalid to use an immediate context concurrently with most of the DXGI interface functions. For the March 2009 and later DirectX SDKs, the only DXGI functions that are safe are[AddRef,[Release, and[QueryInterface.


For more info about using DXGI with multiple threads, see[Multithread Considerations.
## Related topics


[Multithreading


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19