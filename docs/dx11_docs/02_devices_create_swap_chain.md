# 02 Devices Create Swap Chain

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-create-swap-chain](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-create-swap-chain)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# How To: Create a Swap Chain
FeedbackSummarize this article for me
## 


This topic show how to create a swap chain that encapsulates two or more buffers that are used for rendering and display. They usually contain a front buffer that is presented to the display device and a back buffer that serves as the render target. After the immediate context is done rendering to the back buffer, the swap chain presents the back buffer by swapping the two buffers.

The swap chain defines several rendering characteristics including:
- The size of the render area.
- The display refresh rate.
- The display mode.
- The surface format.

Define the characteristics of the swap chain by filling in a[DXGI_SWAP_CHAIN_DESCstructure and initializing an[IDXGISwapChaininterface. Initialize a swap chain by calling[IDXGIFactory::CreateSwapChainor[D3D11CreateDeviceAndSwapChain.
## Create a device and a swap chain


To initialize a device and swap chain, use one of the following two functions:
- 

Use the[D3D11CreateDeviceAndSwapChainfunction when you want to initialize the swap chain at the same time as device initialization. This usually is the easiest option.
- 

Use the[D3D11CreateDevicefunction when you have already created a swap chain using[IDXGIFactory::CreateSwapChain.
## Related topics


[Devices

[How to Use Direct3D 11


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19