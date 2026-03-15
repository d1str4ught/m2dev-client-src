# 02 Devices Downlevel

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Direct3D 11 on Downlevel Hardware
FeedbackSummarize this article for me
## 


This section discusses how Direct3D 11 is designed to support both new and existing hardware, from DirectX 9 to DirectX 11.

This diagram shows how Direct3D 11 supports new and existing hardware.


With Direct3D 11, a new paradigm is introduced called feature levels. A feature level is a well defined set of GPU functionality. Using a feature level, you can target a Direct3D application to run on a downlevel version of Direct3D hardware.

The[10Level9 Referencesection lists the differences between how various[ID3D11Deviceand[ID3D11DeviceContextmethods behave at various 10Level9 feature levels.
## In this section


| Topic | Description |
| [Direct3D feature levels
 | This topic discusses Direct3D feature levels.
 |
| [Exceptions
 | This topic describes exceptions when using Direct3D 11 on downlevel hardware.
 |
| [Compute Shaders on Downlevel Hardware
 | This topic discusses how to make use of[compute shadersin a Direct3D 11 app on Direct3D 10 hardware.
 |
| [Preventing Unwanted NULL Pixel Shader SRVs
 | This topic discusses how to work around the driver receivingNULLshader-resource views (SRVs) even when non-NULLSRVs are bound to the pixel shader stage.
 |
## How to topics about feature levels


| Topic | Description |
| [How To: Get the Device Feature Level
 | How to get a feature level.
 |
## Related topics


[Devices
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-11-04