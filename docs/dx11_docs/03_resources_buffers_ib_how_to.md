# 03 Resources Buffers Ib How To

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-index-how-to](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-index-how-to)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# How to: Create an Index Buffer
FeedbackSummarize this article for me
## 


[Index bufferscontain index data. This topic shows how to initialize an[index bufferin preparation for rendering.

To initialize an index buffer
- Create a buffer that contains your index information.
- Create a buffer description by filling in a[D3D11_BUFFER_DESCstructure. Pass the D3D11_BIND_INDEX_BUFFER flag to theBindFlagsmember and pass the size of the buffer in bytes to theByteWidthmember.
- Create a subresource data description by filling in a[D3D11_SUBRESOURCE_DATAstructure. ThepSysMemmember should point directly to the index data created in step one.
- Call[ID3D11Device::CreateBufferwhile passing the[D3D11_BUFFER_DESCstructure, the[D3D11_SUBRESOURCE_DATAstructure, and the address of a pointer to the[ID3D11Bufferinterface to initialize.

The following code example demonstrates how to create an index buffer. This example assumes that
```
g_pd3dDevice

```


is a valid[ID3D11Deviceobject and that
```
g_pd3dContext

```


is a valid[ID3D11DeviceContextobject.
```
ID3D11Buffer *g_pIndexBuffer = NULL;

// Create indices.
unsigned int indices[] = { 0, 1, 2 };

// Fill in a buffer description.
D3D11_BUFFER_DESC bufferDesc;
bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
bufferDesc.ByteWidth       = sizeof( unsigned int ) * 3;
bufferDesc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
bufferDesc.CPUAccessFlags  = 0;
bufferDesc.MiscFlags       = 0;

// Define the resource data.
D3D11_SUBRESOURCE_DATA InitData;
InitData.pSysMem = indices;
InitData.SysMemPitch = 0;
InitData.SysMemSlicePitch = 0;

// Create the buffer with the device.
hr = g_pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &g_pIndexBuffer );
if( FAILED( hr ) )
    return hr;

// Set the buffer.
g_pd3dContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
    

```

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