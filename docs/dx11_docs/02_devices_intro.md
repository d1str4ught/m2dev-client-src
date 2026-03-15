# 02 Devices Intro

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-intro](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-intro)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Introduction to a Device in Direct3D 11
FeedbackSummarize this article for me
## 


The Direct3D 11 object model separates resource creation and rendering functionality into a device and one or more contexts; this separation is designed to facilitate multithreading.
- [Device
- [Device Context
- [Immediate Context
- [Deferred Context
- [Threading Considerations
- [Related topics
## Device


A device is used to create resources and to enumerate the capabilities of a display adapter. In Direct3D 11, a device is represented with an[ID3D11Deviceinterface.

Each application must have at least one device, most applications only create one device. Create a device for one of the hardware drivers installed on your machine by calling[D3D11CreateDeviceor[D3D11CreateDeviceAndSwapChainand specify the driver type with the[D3D_DRIVER_TYPEflag. Each device can use one or more device contexts, depending on the functionality desired.
## Device Context


A device context contains the circumstance or setting in which a device is used. More specifically, a device context is used to set pipeline state and generate rendering commands using the resources owned by a device. Direct3D 11 implements two types of device contexts, one for immediate rendering and the other for deferred rendering; both contexts are represented with an[ID3D11DeviceContextinterface.
### Immediate Context


An immediate context renders directly to the driver. Each device has one and only one immediate context which can retrieve data from the GPU. An immediate context can be used to immediately render (or play back) a[command list.

There are two ways to get an immediate context:
- By calling either[D3D11CreateDeviceor[D3D11CreateDeviceAndSwapChain.
- By calling[ID3D11Device::GetImmediateContext.
### Deferred Context


A deferred context records GPU commands into a[command list. A deferred context is primarily used for multithreading and is not necessary for a single-threaded application. A deferred context is generally used by a worker thread instead of the main rendering thread. When you create a deferred context, it does not inherit any state from the immediate context.

To get a deferred context, call[ID3D11Device::CreateDeferredContext.

Any context -- immediate or deferred -- can be used on any thread as long as the context is only used in one thread at a time.
## Threading Considerations


This table highlights the differences in the threading model in Direct3D 11 from prior versions of Direct3D.

Differences between Direct3D 11 and previous versions of Direct3D:
All[ID3D11Deviceinterface methods are free-threaded, which means it is safe to have multiple threads call the functions at the same time.

- All[ID3D11DeviceChild-derived interfaces ([ID3D11Buffer,[ID3D11Query, etc.) are free-threaded.
- Direct3D 11 splits resource creating and rendering into two interfaces. Map, Unmap, Begin, End, and GetData are implemented on[ID3D11DeviceContextbecause[ID3D11Devicestrongly defines the order of operations.[ID3D11Resourceand[ID3D11Asynchronousinterfaces also implement methods for free-threaded operations.
- The[ID3D11DeviceContextmethods (except for those that exist on[ID3D11DeviceChild) are not free-threaded, that is, they require single threading. Only one thread may safely be calling any of its methods (Draw, Copy, Map, etc.) at a time.
- In general, free-threading minimizes the number of synchronization primitives used as well as their duration. However, an application that uses synchronization held for a long time can directly impact how much concurrency an application can expect to achieve.ID3D10Device interface methods are not designed to be free-threaded. ID3D10Device implements all create and rendering functionality (as does ID3D9Device in Direct3D 9. Map and Unmap are implemented on ID3D10Resource-derived interfaces, Begin, End, and GetData are implemented on ID3D10Asynchronous-derived interfaces.


## Related topics


[Devices
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2022-04-26