# 03 Resources Textures Fill Manually

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-textures-how-to-fill-manually](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-textures-how-to-fill-manually)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# How to: Initialize a Texture Programmatically
FeedbackSummarize this article for me
## 


You can initialize a texture during object creation, or you can fill the object programmatically after it is created. This topic has several examples showing how to initialize textures that are created with different types of usages. This example assumes you already know how to[Create a Texture.
- [Default Usage
- [Dynamic Usage
- [Staging Usage
- [Related topics
## Default Usage


The most common type of usage is default usage. To fill a default texture (one created withD3D11_USAGE_DEFAULT) you can either:
- 

Call[ID3D11Device::CreateTexture2Dand initializepInitialDatato point to data provided from an application.

or
- 

After calling[ID3D11Device::CreateTexture2D, use[ID3D11DeviceContext::UpdateSubresourceto fill the default texture with data from a pointer provided by the application.
## Dynamic Usage


To fill a dynamic texture (one created withD3D11_USAGE_DYNAMIC):
- Get a pointer to the texture memory by passing inD3D11_MAP_WRITE_DISCARDwhen calling[ID3D11DeviceContext::Map.
- Write data to the memory.
- Call[ID3D11DeviceContext::Unmapwhen you are finished writing data.
## Staging Usage


To fill a staging texture (one created withD3D11_USAGE_STAGING):
- Get a pointer to the texture memory by passing inD3D11_MAP_WRITEwhen calling[ID3D11DeviceContext::Map.
- Write data to the memory.
- Call[ID3D11DeviceContext::Unmapwhen you are finished writing data.

A staging texture can then be used as the source parameter to[ID3D11DeviceContext::CopyResourceor[ID3D11DeviceContext::CopySubresourceRegionto fill a default or dynamic resource.
## Related topics


[How to Use Direct3D 11

[Textures


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2019-08-23