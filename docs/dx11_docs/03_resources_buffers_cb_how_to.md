# 03 Resources Buffers Cb How To

Source: [https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-constant-how-to](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-constant-how-to)

---

Table of contentsExit editor modeAsk LearnAsk LearnFocus modeTable of contents[Read in EnglishAddAdd to plan[Edit
#### Share via
[Facebook[x.com[LinkedIn[EmailCopy MarkdownPrint

Note

Access to this page requires authorization. You can try[signing inorchanging directories.

Access to this page requires authorization. You can trychanging directories.
# How to: Create a Constant Buffer
FeedbackSummarize this article for me
## 


[Constant bufferscontain shader constant data. This topic shows how to initialize a[constant bufferin preparation for rendering.

To initialize a constant buffer
- 

Define a structure that describes the vertex shader constant data.
- 

Allocate memory for the structure that you defined in step one. Fill this buffer with vertex shader constant data. You can usemallocornewto allocate the memory, or you can allocate memory for the structure from the stack.
- 

Create a buffer description by filling in a[D3D11_BUFFER_DESCstructure. Pass the D3D11_BIND_CONSTANT_BUFFER flag to theBindFlagsmember and pass the size of the constant buffer description structure in bytes to theByteWidthmember.

Note

The D3D11_BIND_CONSTANT_BUFFER flag cannot be combined with any other flags.


- 

Create a subresource data description by filling in a[D3D11_SUBRESOURCE_DATAstructure. ThepSysMemmember of theD3D11_SUBRESOURCE_DATAstructure must point directly to the vertex shader constant data that you created in step two.
- 

Call[ID3D11Device::CreateBufferwhile passing the[D3D11_BUFFER_DESCstructure, the[D3D11_SUBRESOURCE_DATAstructure, and the address of a pointer to the[ID3D11Bufferinterface to initialize.

These code examples demonstrate how to create a constant buffer.

This example assumes thatg_pd3dDeviceis a valid[ID3D11Deviceobject and thatg_pd3dContextis a valid[ID3D11DeviceContextobject.
```
ID3D11Buffer*   g_pConstantBuffer11 = NULL;

// Define the constant data used to communicate with shaders.
struct VS_CONSTANT_BUFFER
{
    XMFLOAT4X4 mWorldViewProj;                              
    XMFLOAT4 vSomeVectorThatMayBeNeededByASpecificShader;
    float fSomeFloatThatMayBeNeededByASpecificShader;
    float fTime;                                            
    float fSomeFloatThatMayBeNeededByASpecificShader2;
    float fSomeFloatThatMayBeNeededByASpecificShader3;
} VS_CONSTANT_BUFFER;

// Supply the vertex shader constant data.
VS_CONSTANT_BUFFER VsConstData;
VsConstData.mWorldViewProj = {...};
VsConstData.vSomeVectorThatMayBeNeededByASpecificShader = XMFLOAT4(1,2,3,4);
VsConstData.fSomeFloatThatMayBeNeededByASpecificShader = 3.0f;
VsConstData.fTime = 1.0f;
VsConstData.fSomeFloatThatMayBeNeededByASpecificShader2 = 2.0f;
VsConstData.fSomeFloatThatMayBeNeededByASpecificShader3 = 4.0f;

// Fill in a buffer description.
D3D11_BUFFER_DESC cbDesc;
cbDesc.ByteWidth = sizeof( VS_CONSTANT_BUFFER );
cbDesc.Usage = D3D11_USAGE_DYNAMIC;
cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
cbDesc.MiscFlags = 0;
cbDesc.StructureByteStride = 0;

// Fill in the subresource data.
D3D11_SUBRESOURCE_DATA InitData;
InitData.pSysMem = &VsConstData;
InitData.SysMemPitch = 0;
InitData.SysMemSlicePitch = 0;

// Create the buffer.
hr = g_pd3dDevice->CreateBuffer( &cbDesc, &InitData, 
                                 &g_pConstantBuffer11 );

if( FAILED( hr ) )
   return hr;

// Set the buffer.
g_pd3dContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer11 );
    

```


This example shows the associated HLSL cbuffer definition.
```
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix mWorldViewProj;
    float4  vSomeVectorThatMayBeNeededByASpecificShader;
    float fSomeFloatThatMayBeNeededByASpecificShader;
    float fTime;
    float fSomeFloatThatMayBeNeededByASpecificShader2;
    float fSomeFloatThatMayBeNeededByASpecificShader3;
};

```


Note

Make sure to match the VS_CONSTANT_BUFFER memory layout in C++ with the HLSL layout. For info about how HLSL handles layout in memory, see[Packing Rules for Constant Variables.


## Related topics


[Buffers

[How to Use Direct3D 11


## Feedback


Was this page helpful?YesNoNo

Need help with this topic?

Want to try using Ask Learn to clarify or guide you through this topic?Ask LearnAsk LearnSuggest a fix?
## 
				Additional resources
			

- Last updated on2020-08-19