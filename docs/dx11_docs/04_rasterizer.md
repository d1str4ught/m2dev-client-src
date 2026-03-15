# 04 Rasterizer

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Rasterizer Stage
FeedbackSummarize this article for me
## 


The rasterization stage converts vector information (composed of shapes or primitives) into a raster image (composed of pixels) for the purpose of displaying real-time 3D graphics.

During rasterization, each primitive is converted into pixels, while interpolating per-vertex values across each primitive. Rasterization includes clipping vertices to the view frustum, performing a divide by z to provide perspective, mapping primitives to a 2D viewport, and determining how to invoke the pixel shader. While using a pixel shader is optional, the rasterizer stage always performs clipping, a perspective divide to transform the points into homogeneous space, and maps the vertices to the viewport.

Vertices (x,y,z,w), coming into the rasterizer stage are assumed to be in homogeneous clip-space. In this coordinate space the X axis points right, Y points up and Z points away from camera.

You may disable rasterization by telling the pipeline there is no pixel shader (set the pixel shader stage toNULLwith[ID3D11DeviceContext::PSSetShader), and disabling depth and stencil testing (setDepthEnableandStencilEnabletoFALSEin[D3D11_DEPTH_STENCIL_DESC). While disabled, rasterization-related pipeline counters will not update. There is also a complete description of the[rasterization rules.

On hardware that implements hierarchical Z-buffer optimizations, you may enable preloading the z-buffer by setting the pixel shader stage toNULLwhile enabling depth and stencil testing.
## In this section


| Topic | Description |
| [Getting Started with the Rasterizer Stage
 | This section describes setting the viewport, the scissors rectangle, the rasterizer state, and multi-sampling.
 |
| [Rasterization Rules
 | Rasterization rules define how vector data is mapped into raster data. The raster data is snapped to integer locations that are then culled and clipped (to draw the minimum number of pixels), and per-pixel attributes are interpolated (from per-vertex attributes) before being passed to a pixel shader.
 |
## Related topics


[Graphics Pipeline

[Pipeline Stages (Direct3D 10)
## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-11-04