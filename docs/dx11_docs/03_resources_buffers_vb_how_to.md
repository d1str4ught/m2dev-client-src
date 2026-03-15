# 03 Resources Buffers Vb How To

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# How to: Create a Vertex Buffer
FeedbackSummarize this article for me
## 


[Vertex bufferscontain per vertex data. This topic shows how to initialize a static[vertex buffer, that is, a vertex buffer that does not change. For help initializing a non-static buffer, see the[remarkssection.

To initialize a static vertex buffer
- Define a structure that describes a vertex. For example, if your vertex data contains position data and color data, your structure would have one vector that describes the position and another that describes the color.
- Allocate memory (using malloc or new) for the structure that you defined in step one. Fill this buffer with the actual vertex data that describes your geometry.
- Create a buffer description by filling in a[D3D11_BUFFER_DESCstructure. Pass the D3D11_BIND_VERTEX_BUFFER flag to theBindFlagsmember and pass the size of the structure from step one to theByteWidthmember.
- Create a subresource data description by filling in a[D3D11_SUBRESOURCE_DATAstructure. ThepSysMemmember of the[D3D11_SUBRESOURCE_DATAstructure should point directly to the resource data created in step two.
- Call[ID3D11Device::CreateBufferwhile passing the[D3D11_BUFFER_DESCstructure, the[D3D11_SUBRESOURCE_DATAstructure, and the address of a pointer to the[ID3D11Bufferinterface to initialize.

The following code example demonstrates how to create a vertex buffer. This example assumes thatg_pd3dDeviceis a valid[ID3D11Deviceobject.
```
ID3D11Buffer*      g_pVertexBuffer;

// Define the data-type that
// describes a vertex.
struct SimpleVertexCombined
{
    XMFLOAT3 Pos;  
    XMFLOAT3 Col;  
};

// Supply the actual vertex data.
SimpleVertexCombined verticesCombo[] =
{
    XMFLOAT3( 0.0f, 0.5f, 0.5f ),
    XMFLOAT3( 0.0f, 0.0f, 0.5f ),
    XMFLOAT3( 0.5f, -0.5f, 0.5f ),
    XMFLOAT3( 0.5f, 0.0f, 0.0f ),
    XMFLOAT3( -0.5f, -0.5f, 0.5f ),
    XMFLOAT3( 0.0f, 0.5f, 0.0f ),
};

// Fill in a buffer description.
D3D11_BUFFER_DESC bufferDesc;
bufferDesc.Usage            = D3D11_USAGE_DEFAULT;
bufferDesc.ByteWidth        = sizeof( SimpleVertexCombined ) * 3;
bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
bufferDesc.CPUAccessFlags   = 0;
bufferDesc.MiscFlags        = 0;

// Fill in the subresource data.
D3D11_SUBRESOURCE_DATA InitData;
InitData.pSysMem = verticesCombo;
InitData.SysMemPitch = 0;
InitData.SysMemSlicePitch = 0;

// Create the vertex buffer.
hr = g_pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &g_pVertexBuffer );
    

```

## Remarks


Here are some ways to initialize a vertex buffer that changes over time.
- Create a 2nd buffer with[D3D11_USAGE_STAGING; fill the second buffer using[ID3D11DeviceContext::Map,[ID3D11DeviceContext::Unmap; use[ID3D11DeviceContext::CopyResourceto copy from the staging buffer to the default buffer.
- Use[ID3D11DeviceContext::UpdateSubresourceto copy data from memory.
- Create a buffer with[D3D11_USAGE_DYNAMIC, and fill it with[ID3D11DeviceContext::Map,[ID3D11DeviceContext::Unmap(using the Discard and NoOverwrite flags appropriately).

#1 and #2 are useful for content that changes less than once per frame. In general, GPU reads will be fast and CPU updates will be slower.

#3 is useful for content that changes more than once per frame. In general, GPU reads will be slower, but CPU updates will be faster.
## Related topics


[Buffers

[How to Use Direct3D 11


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2019-08-23