# 06 Effects

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d11-graphics-programming-guide-effects](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d11-graphics-programming-guide-effects)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Effects (Direct3D 11)
FeedbackSummarize this article for me
## 


A DirectX effect is a collection of pipeline state, set by expressions written in[HLSLand some syntax that is specific to the effect framework.

After compiling an effect, use the effect framework APIs to render. Effect functionality can range from something as simple as a vertex shader that transforms geometry and a pixel shader that outputs a solid color, to a rendering technique that requires multiple passes, uses every stage of the graphics pipeline, and manipulates shader state as well as the pipeline state not associated with the programmable shaders.

The first step is to organize the state you want to control in an effect. This includes shader state (vertex, hull, domain, geometry, pixel and compute shaders), texture and sampler state used by the shaders, and other non-programmable pipeline state. You can create an effect in memory as a text string, but typically, the size gets large enough that it is handy to store effect state in an effect file (a text file that ends in a .fx extension). To use an effect, you must compile it (to check HLSL syntax as well as effect framework syntax), initialize effect state through API calls, and modify your render loop to call the rendering APIs.

An effect encapsulates all of the render state required by a particular effect into a single rendering function called a technique. A pass is a sub-set of a technique, that contains render state. To implement a multiple pass rendering effect, implement one or more passes within a technique. For example, say you wanted to render some geometry with one set of depth/stencil buffers, and then draw some sprites on top of that. You could implement the geometry rendering in the first pass, and the sprite drawing in the second pass. To render the effect, you simply render both passes in your render loop. You can implement any number of techniques in an effect. Of course, the greater the number of techniques, the greater the compile time for the effect. One way to exploit this functionality is to create effects with techniques that are designed to run on different hardware. This allows an application to gracefully downgrade performance based on the hardware capabilities detected.

A set of techniques can be grouped in a group (which uses the syntax "fxgroup"). Techniques can be grouped in any way. For example, multiple groups could be created, one per material; each material could have a technique for each hardware level; each technique would have a set of passes which define the material on the particular hardware.
## In this section


| Topic | Description |
| [Organizing State in an Effect
 | With Direct3D 11, effect state for certain pipeline stages is organized by structures.
 |
| [Effect System Interfaces
 | The effect system defines several interfaces for managing effect state.
 |
| [Specializing Interfaces
 | [ID3DX11EffectVariablehas a number of methods for casting the interface into the particular type of interface you need.
 |
| [Interfaces and Classes in Effects
 | There are many ways to use classes and interfaces in Effects 11.
 |
| [Rendering an Effect
 | An effect can be used to store information, or to render using a group of state.
 |
| [Cloning an Effect
 | Cloning an effect creates a second, almost identical copy of the effect.
 |
| [Stream Out Syntax
 | A geometry shader with stream out is declared with a particular syntax.
 |
| [Differences Between Effects 10 and Effects 11
 | This topic shows the differences between Effects 10 and Effects 11.
 |
## Related topics


[Programming Guide for Direct3D 11
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2021-06-11