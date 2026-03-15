# 02 Devices Overview

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Devices (Direct3D 11 Graphics)
FeedbackSummarize this article for me
## 


A Direct3D device allocates and destroys objects, renders primitives, and communicates with a graphics driver and the hardware. In Direct3D 11, a device is separated into a device object for creating resources and a device-context object, which performs rendering. This section describes Direct3D 11 device and device-context objects.

Objects created from one device cannot be used directly with other devices. Use a shared resource to share data between multiple devices, with the constraint that a shared object can be used only by the device that created it.
## In this section


| Topic | Description |
| [Introduction to a Device in Direct3D 11
 | The Direct3D 11 object model separates resource creation and rendering functionality into a device and one or more contexts; this separation is designed to facilitate multithreading.
 |
| [Software Layers
 | The Direct3D 11 runtime is constructed with layers, starting with the basic functionality at the core and building optional and developer-assist functionality in outer layers. This section describes the functionality of each layer.
 |
| [Limitations Creating WARP and Reference Devices
 | Some limitations exist for creating WARP and Reference devices in Direct3D 10.1 and Direct3D 11.0. This topic discusses those limitations.
 |
| [Direct3D 11 on Downlevel Hardware
 | This section discusses how Direct3D 11 is designed to support both new and existing hardware, from DirectX 9 to DirectX 11.
 |
| [Using Direct3D 11 feature data to supplement Direct3D feature levels
 | Find out how to check device support for optional features, including features that were added in recent versions of Windows.
 |
## How to topics about devices


| Topic | Description |
| [How To: Create a Reference Device
 | Describes how to create a reference device.
 |
| [How To: Create a WARP Device
 | Describes how to create a WARP device.
 |
| [How To: Create a Swap Chain
 | Describes how to create a swap chain.
 |
| [How To: Enumerate Adapters
 | Describes how to enumerate the physical display adapters.
 |
| [How To: Get Adapter Display Modes
 | Describes how to get the supported display capabilities of an adapter.
 |
| [How To: Create a Device and Immediate Context
 | Describes how to initialize a device.
 |
## Related topics


[Programming Guide for Direct3D 11
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-12-10