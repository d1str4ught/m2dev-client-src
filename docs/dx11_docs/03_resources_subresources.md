# 03 Resources Subresources

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-subresources](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-subresources)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# Subresources (Direct3D 11 Graphics)
FeedbackSummarize this article for me
## 


This topic describes texture subresources, or portions of a resource.

Direct3D can reference an entire resource or it can reference subsets of a resource. The term subresource refers to a subset of a resource.

A buffer is defined as a single subresource. Textures are a little more complicated because there are several different texture types (1D, 2D, etc.) some of which support mipmap levels and/or texture arrays. Beginning with the simplest case, a 1D texture is defined as a single subresource, as shown in the following illustration.


This means that the array of texels that make up a 1D texture are contained in a single subresource.

If you expand a 1D texture with three mipmap levels, it can be visualized like the following illustration.


Think of this as a single texture that is made up of three subresources. A subresource can be indexed using the level-of-detail (LOD) for a single texture. When using an array of textures, accessing a particular subresource requires both the LOD and the particular texture. Alternately, the API combines these two pieces of information into a single zero-based subresource index, as shown in the following illustration.


## Selecting Subresources


Some APIs access an entire resource (for example the[ID3D11DeviceContext::CopyResourcemethod), others access a portion of a resource (for example the[ID3D11DeviceContext::UpdateSubresourcemethod or the[ID3D11DeviceContext::CopySubresourceRegionmethod). The methods that access a portion of a resource generally use a view description (such as the[D3D11_TEX2D_ARRAY_DSVstructure) to specify the subresources to access.

The illustrations in the following sections show the terms used by a view description when accessing an array of textures.
### Array Slice


Given an array of textures, each texture with mipmaps, anarray slice(represented by the white rectangle) includes one texture and all of its subresources, as shown in the following illustration.


### Mip Slice


Amip slice(represented by the white rectangle) includes one mipmap level for every texture in an array, as shown in the following illustration.


### Selecting a Single Subresource


You can use these two types of slices to choose a single subresource, as shown in the following illustration.


### Selecting Multiple Subresources


Or you can use these two types of slices with the number of mipmap levels and/or number of textures, to choose multiple subresources, as shown in the following illustration.


Note

A[render-target viewcan only use a single subresource or mip slice and cannot include subresources from more than one mip slice. That is, every texture in a render-target view must be the same size. A[shader-resource viewcan select any rectangular region of subresources, as shown in the figure.


## Related topics


[Resources


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-12-10