# 04 Primitive Topologies

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-primitive-topologies](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-primitive-topologies)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Primitive Topologies
FeedbackSummarize this article for me
## 


Direct3D 10 and higher supports several primitive types (or topologies) that are represented by the[D3D_PRIMITIVE_TOPOLOGYenumerated type. These types define how vertices are interpreted and rendered by the pipeline.
- [Basic Primitive Types
- [Primitive Adjacency
- [Winding Direction and Leading Vertex Positions
- [Generating Multiple Strips
- [Related topics
## Basic Primitive Types


The following basic primitive types are supported:
- [Point List
- [Line List
- [Line Strip
- [Triangle List
- [Triangle Strip

For a visualization of each primitive type, see the diagram later in this topic in[Winding Direction and Leading Vertex Positions.

The input-assembler stage reads data from vertex and index buffers, assembles the data into these primitives, and then sends the data to the remaining pipeline stages. (You can use the[ID3D11DeviceContext::IASetPrimitiveTopologymethod to specify the primitive type for the input-assembler stage.)
## Primitive Adjacency


All Direct3D 10 and higher primitive types (except the point list) are available in two versions: one primitive type with adjacency and one primitive type without adjacency. Primitives with adjacency contain some of the surrounding vertices, while primitives without adjacency contain only the vertices of the target primitive. For example, the line list primitive (represented by theD3D_PRIMITIVE_TOPOLOGY_LINELISTvalue) has a corresponding line list primitive that includes adjacency (represented by theD3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJvalue.)

Adjacent primitives are intended to provide more information about your geometry and are only visible through a geometry shader. Adjacency is useful for geometry shaders that use silhouette detection, shadow volume extrusion, and so on.

For example, suppose you want to draw a triangle list with adjacency. A triangle list that contains 36 vertices (with adjacency) will yield 6 completed primitives. Primitives with adjacency (except line strips) contain exactly twice as many vertices as the equivalent primitive without adjacency, where each additional vertex is an adjacent vertex.
## Winding Direction and Leading Vertex Positions


As shown in the following illustration, a leading vertex is the first non-adjacent vertex in a primitive. A primitive type can have multiple leading vertices defined, as long as each one is used for a different primitive. For a triangle strip with adjacency, the leading vertices are 0, 2, 4, 6, and so on. For a line strip with adjacency, the leading vertices are 1, 2, 3, and so on. An adjacent primitive, on the other hand, has no leading vertex.

The following illustration shows the vertex ordering for all of the primitive types that the input assembler can produce.


The symbols in the preceding illustration are described in the following table.

| Symbol | Name | Description |
|  | Vertex | A point in 3D space. |
|  | Winding Direction | The vertex order when assembling a primitive. Can be clockwise or counterclockwise; specify this by calling[ID3D11Device1::CreateRasterizerState1. |
|  | Leading Vertex | The first non-adjacent vertex in a primitive that contains per-constant data. |


## Generating Multiple Strips


You can generate multiple strips through strip cutting. You can perform a strip cut by explicitly calling the[RestartStripHLSL function, or by inserting a special index value into the index buffer. This value is –1, which is 0xffffffff for 32-bit indices or 0xffff for 16-bit indices. An index of –1 indicates an explicit 'cut' or 'restart' of the current strip. The previous index completes the previous primitive or strip and the next index starts a new primitive or strip. For more info about generating multiple strips, see[Geometry-Shader Stage.

Note

You need[feature level10.0 or higher hardware because not all 10level9 hardware implements this functionality.


## Related topics


[Getting Started with the Input-Assembler Stage

[Pipeline Stages (Direct3D 10)


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19