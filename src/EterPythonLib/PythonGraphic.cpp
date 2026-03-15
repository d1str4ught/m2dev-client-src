#include "StdAfx.h"
#include "EterLib/StateManager.h"
#include "EterLib/JpegFile.h"
#include "PythonGraphic.h"
#include <d3d11.h>
#include <utf8.h>

bool g_isScreenShotKey = false;

void CPythonGraphic::Destroy()
{	
}

LPDIRECT3D9EX CPythonGraphic::GetD3D()
{
	return ms_lpd3d;
}

float CPythonGraphic::GetOrthoDepth()
{
	return m_fOrthoDepth;
}

void CPythonGraphic::SetInterfaceRenderState()
{
	STATEMANAGER.SetTransform(D3DTS_PROJECTION, &ms_matIdentity);
 	STATEMANAGER.SetTransform(D3DTS_VIEW, &ms_matIdentity);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &ms_matIdentity);

	STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

	CPythonGraphic::Instance().SetBlendOperation();
	CPythonGraphic::Instance().SetOrtho2D(ms_iWidth, ms_iHeight, GetOrthoDepth());

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
}

void CPythonGraphic::SetGameRenderState()
{
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
	STATEMANAGER.SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
}

void CPythonGraphic::SetCursorPosition(int x, int y)
{
	CScreen::SetCursorPosition(x, y, ms_iWidth, ms_iHeight);
}

void CPythonGraphic::SetOmniLight()
{
    // Set up a material
    D3DMATERIAL9 Material;
	Material.Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
	Material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	Material.Emissive = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
    STATEMANAGER.SetMaterial(&Material);

	D3DLIGHT9 Light;
	Light.Type = D3DLIGHT_SPOT;
    Light.Position = D3DXVECTOR3(50.0f, 150.0f, 350.0f);
    Light.Direction = D3DXVECTOR3(-0.15f, -0.3f, -0.9f);
    Light.Theta = D3DXToRadian(30.0f);
    Light.Phi = D3DXToRadian(45.0f);
    Light.Falloff = 1.0f;
    Light.Attenuation0 = 0.0f;
    Light.Attenuation1 = 0.005f;
    Light.Attenuation2 = 0.0f;
    Light.Diffuse.r = 1.0f;
    Light.Diffuse.g = 1.0f;
    Light.Diffuse.b = 1.0f;
	Light.Diffuse.a = 1.0f;
	Light.Ambient.r = 1.0f;
	Light.Ambient.g = 1.0f;
	Light.Ambient.b = 1.0f;
	Light.Ambient.a = 1.0f;
    Light.Range = 500.0f;
	ms_lpd3dDevice->SetLight(0, &Light);
	ms_lpd3dDevice->LightEnable(0, TRUE);

	Light.Type = D3DLIGHT_POINT;
	Light.Position = D3DXVECTOR3(0.0f, 200.0f, 200.0f);
	Light.Attenuation0 = 0.1f;
	Light.Attenuation1 = 0.01f;
	Light.Attenuation2 = 0.0f;
	ms_lpd3dDevice->SetLight(1, &Light);
	ms_lpd3dDevice->LightEnable(1, TRUE);
}

void CPythonGraphic::SetViewport(float fx, float fy, float fWidth, float fHeight)
{
	ms_lpd3dDevice->GetViewport(&m_backupViewport);

	D3DVIEWPORT9 ViewPort;
	ViewPort.X = fx;
	ViewPort.Y = fy;
	ViewPort.Width = fWidth;
	ViewPort.Height = fHeight;
	ViewPort.MinZ = 0.0f;
	ViewPort.MaxZ = 1.0f;
	if (FAILED(
		ms_lpd3dDevice->SetViewport(&ViewPort)
	))
	{
		Tracef("CPythonGraphic::SetViewport(%d, %d, %d, %d) - Error",
			ViewPort.X, ViewPort.Y,
			ViewPort.Width, ViewPort.Height
		);
	}

	// DX11: Sync viewport
	if (ms_pD3D11Context) {
		D3D11_VIEWPORT vp11 = {};
		vp11.TopLeftX = fx;
		vp11.TopLeftY = fy;
		vp11.Width = fWidth;
		vp11.Height = fHeight;
		vp11.MinDepth = 0.0f;
		vp11.MaxDepth = 1.0f;
		ms_pD3D11Context->RSSetViewports(1, &vp11);
	}
}

void CPythonGraphic::RestoreViewport()
{
	ms_lpd3dDevice->SetViewport(&m_backupViewport);

	// DX11: Restore viewport
	if (ms_pD3D11Context) {
		D3D11_VIEWPORT vp11 = {};
		vp11.TopLeftX = (FLOAT)m_backupViewport.X;
		vp11.TopLeftY = (FLOAT)m_backupViewport.Y;
		vp11.Width = (FLOAT)m_backupViewport.Width;
		vp11.Height = (FLOAT)m_backupViewport.Height;
		vp11.MinDepth = m_backupViewport.MinZ;
		vp11.MaxDepth = m_backupViewport.MaxZ;
		ms_pD3D11Context->RSSetViewports(1, &vp11);
	}
}

void CPythonGraphic::SetGamma(float fGammaFactor)
{
	D3DCAPS9		d3dCaps;
	D3DGAMMARAMP	NewRamp;
	int				ui, val;
	
	ms_lpd3dDevice->GetDeviceCaps(&d3dCaps);

	if (D3DCAPS2_FULLSCREENGAMMA != (d3dCaps.Caps2 & D3DCAPS2_FULLSCREENGAMMA))
		return;

	for (int i = 0; i < 256; ++i)
	{
		val	= (int) (i * fGammaFactor * 255.0f);
		ui = 0;
		
		if (val > 32767)
		{
			val = val - 32767;
			ui = 1;
		}

		if (val > 32767)
			val = 32767;
		
		NewRamp.red[i] = (WORD) (val | (32768 * ui));
		NewRamp.green[i] = (WORD) (val | (32768 * ui));
		NewRamp.blue[i] = (WORD) (val | (32768 * ui));
	}

	ms_lpd3dDevice->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &NewRamp);
}

void GenScreenShotTag(const char* src, DWORD crc32, char* leaf, size_t leafLen)
{
	const char* p = src;
	const char* n = p;
	while (n = strchr(p, '\\'))
		p = n + 1;

	_snprintf(leaf, leafLen, "YMIR_METIN2:%s:0x%.8x", p, crc32);
}

bool CPythonGraphic::SaveJPEG(const char * pszFileName, LPBYTE pbyBuffer, UINT uWidth, UINT uHeight)
{
	return jpeg_save(pbyBuffer, uWidth, uHeight, 100, pszFileName) != 0;
}

bool CPythonGraphic::SaveScreenShot(const char * c_pszFileName)
{
	HRESULT hr;
	LPDIRECT3DSURFACE9 lpSurface;
	D3DSURFACE_DESC stSurfaceDesc;

	if (FAILED(hr = ms_lpd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &lpSurface)))
	{
		TraceError("Failed to get back buffer (0x%08x)", hr);
		return false;
	}

	if (FAILED(hr = lpSurface->GetDesc(&stSurfaceDesc)))
	{
		TraceError("Failed to get surface desc (0x%08x)", hr);
		SAFE_RELEASE(lpSurface);
		return false;
	}

	UINT uWidth = stSurfaceDesc.Width;
	UINT uHeight = stSurfaceDesc.Height;

	switch( stSurfaceDesc.Format ) {
	case D3DFMT_R8G8B8 :
	case D3DFMT_A8R8G8B8 :
	case D3DFMT_X8R8G8B8 :
	case D3DFMT_R5G6B5 :
	case D3DFMT_X1R5G5B5 :
	case D3DFMT_A1R5G5B5 :
		break;
	case D3DFMT_A4R4G4B4 :
	case D3DFMT_R3G3B2 :
	case D3DFMT_A8R3G3B2 :
	case D3DFMT_X4R4G4B4 :
	case D3DFMT_A2B10G10R10 :
		TraceError("Unsupported BackBuffer Format(%d). Please contact Metin 2 Administrator.", stSurfaceDesc.Format);
		SAFE_RELEASE(lpSurface);
		return false;
	}

	D3DLOCKED_RECT lockRect;
	if (FAILED(hr = lpSurface->LockRect(&lockRect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK)))
	{
		TraceError("Failed to lock the surface (0x%08x)", hr);
		SAFE_RELEASE(lpSurface);
		return false;
	}

	uint8_t* pbyBuffer = new uint8_t[uWidth * uHeight * 3];
	if (pbyBuffer == NULL) {
		lpSurface->UnlockRect();
		lpSurface->Release();
		lpSurface = NULL;
		TraceError("Failed to allocate screenshot buffer");
		return false;
	}
	uint8_t* pbySource = (uint8_t*) lockRect.pBits;
	uint8_t* pbyDestination = (uint8_t*) pbyBuffer;
	for(UINT y = 0; y < uHeight; ++y) {
		uint8_t*pRow = pbySource;

		switch( stSurfaceDesc.Format ) {
		case D3DFMT_R8G8B8 :
			for(UINT x = 0; x < uWidth; ++x) {
				*pbyDestination++ = pRow[2];	// Blue
				*pbyDestination++ = pRow[1];	// Green
				*pbyDestination++ = pRow[0];	// Red
				pRow += 3;
			}
			break;
		case D3DFMT_A8R8G8B8 :
		case D3DFMT_X8R8G8B8 :
			for(UINT x = 0; x < uWidth; ++x) {
				*pbyDestination++ = pRow[2];	// Blue
				*pbyDestination++ = pRow[1];	// Green
				*pbyDestination++ = pRow[0];	// Red
				pRow += 4;
			}
			break;
		case D3DFMT_R5G6B5 :
			{
				for(UINT x = 0; x < uWidth; ++x) {
					UINT uColor		= *((UINT *) pRow);
					uint8_t byBlue		= (uColor >> 11) & 0x1F;
					uint8_t byGreen	= (uColor >> 5) & 0x3F;
					uint8_t byRed		= uColor & 0x1F;

					*pbyDestination++ = (byBlue << 3)	| (byBlue >> 2);		// Blue
					*pbyDestination++ = (byGreen << 2)	| (byGreen >> 2);		// Green
					*pbyDestination++ = (byRed << 3)	| (byRed >> 2);			// Red
					pRow += 2;
				}
			}
			break;
		case D3DFMT_X1R5G5B5 :
		case D3DFMT_A1R5G5B5 :
			{
				for(UINT x = 0; x < uWidth; ++x) {
					UINT uColor		= *((UINT *) pRow);
					uint8_t byBlue		= (uColor >> 10) & 0x1F;
					uint8_t byGreen	= (uColor >> 5) & 0x1F;
					uint8_t byRed		= uColor & 0x1F;

					*pbyDestination++ = (byBlue << 3)	| (byBlue >> 2);		// Blue
					*pbyDestination++ = (byGreen << 3)	| (byGreen >> 2);		// Green
					*pbyDestination++ = (byRed << 3)	| (byRed >> 2);			// Red
					pRow += 2;
				}
			}
			break;
		}

		// increase by one line
		pbySource += lockRect.Pitch;
	}

	if(lpSurface) {
		lpSurface->UnlockRect();
		lpSurface->Release();
		lpSurface = NULL;
	}

	bool bSaved = SaveJPEG(c_pszFileName, pbyBuffer, uWidth, uHeight);

	if(pbyBuffer) {
		delete [] pbyBuffer;
		pbyBuffer = NULL;
	}

	if(bSaved == false) {
		TraceError("Failed to save JPEG file. (%s, %d, %d)", c_pszFileName, uWidth, uHeight);
		return false;
	}

	if (g_isScreenShotKey)
	{
		// UTF-8 → UTF-16 conversion for Unicode path support
		std::wstring wFileName = Utf8ToWide(c_pszFileName);
		FILE* srcFilePtr = _wfopen(wFileName.c_str(), L"rb");
		if (srcFilePtr)
		{
			fseek(srcFilePtr, 0, SEEK_END);		
			size_t fileSize = ftell(srcFilePtr);
			fseek(srcFilePtr, 0, SEEK_SET);

			char head[21];
			size_t tailSize = fileSize - sizeof(head);
			char* tail = (char*)malloc(tailSize);
			
			fread(head, sizeof(head), 1, srcFilePtr);
			fread(tail, tailSize, 1, srcFilePtr);
			fclose(srcFilePtr);

			char imgDesc[64];
			GenScreenShotTag(c_pszFileName, GetCRC32(tail, tailSize), imgDesc, sizeof(imgDesc));

			int imgDescLen = strlen(imgDesc) + 1;
			
			unsigned char exifHeader[] = {
				0xe1,
				0, // blockLen[1],
				0, // blockLen[0],
				0x45,
				0x78,
				0x69,
				0x66,
				0x0,
				0x0,
				0x49,
				0x49,
				0x2a,
				0x0,
				0x8,
				0x0,
				0x0,
				0x0,
				0x1,
				0x0,
				0xe,
				0x1,
				0x2,
				0x0,
				static_cast<unsigned char>(imgDescLen), // textLen[0],
				0, // textLen[1],
				0, // textLen[2],
				0, // textLen[3],
				0x1a,
				0x0,
				0x0,
				0x0,
				0x0,
				0x0,
				0x0,
				0x0,
			};

			exifHeader[2] = sizeof(exifHeader) + imgDescLen;

			FILE* dstFilePtr = _wfopen(wFileName.c_str(), L"wb");
			if (dstFilePtr)
			{
				fwrite(head, sizeof(head), 1, dstFilePtr);
				fwrite(exifHeader, sizeof(exifHeader), 1, dstFilePtr);
				fwrite(imgDesc, imgDescLen, 1, dstFilePtr);
				fputc(0x00, dstFilePtr);
				fputc(0xff, dstFilePtr);
				fwrite(tail, tailSize, 1, dstFilePtr);
				fclose(dstFilePtr);
			}

			free(tail);
		}
	}
	return true;
}

void CPythonGraphic::PushState()
{
	TState curState;

	curState.matProj = ms_matProj;
	curState.matView = ms_matView;
	//STATEMANAGER.SaveTransform(D3DTS_WORLD, &m_SaveWorldMatrix);

	m_stateStack.push(curState);
	//CCamera::Instance().PushParams();
}

void CPythonGraphic::PopState()
{
	if (m_stateStack.empty())
	{
		assert(!"PythonGraphic::PopState StateStack is EMPTY");
		return;
	}
	
	TState & rState = m_stateStack.top();

	//STATEMANAGER.RestoreTransform(D3DTS_WORLD);
	ms_matProj = rState.matProj;
	ms_matView = rState.matView;
	
	UpdatePipeLineMatrix();

	m_stateStack.pop();
	//CCamera::Instance().PopParams();
}

void CPythonGraphic::RenderImage(CGraphicImageInstance* pImageInstance, float x, float y)
{
	assert(pImageInstance != NULL);

	//SetColorRenderState();
	const CGraphicTexture * c_pTexture = pImageInstance->GetTexturePointer();

	float width = (float) pImageInstance->GetWidth();
	float height = (float) pImageInstance->GetHeight();

	c_pTexture->SetTextureStage(0);

	RenderTextureBox(x,
					 y,
					 x + width,
					 y + height,
					 0.0f,
					 0.5f / width, 
					 0.5f / height, 
					 (width + 0.5f) / width, 
					 (height + 0.5f) / height);
}

void CPythonGraphic::RenderAlphaImage(CGraphicImageInstance* pImageInstance, float x, float y, float aLeft, float aRight)
{
	assert(pImageInstance != NULL);

	D3DXCOLOR DiffuseColor1 = D3DXCOLOR(1.0f, 1.0f, 1.0f, aLeft);
	D3DXCOLOR DiffuseColor2 = D3DXCOLOR(1.0f, 1.0f, 1.0f, aRight);

	const CGraphicTexture * c_pTexture = pImageInstance->GetTexturePointer();

	float width = (float) pImageInstance->GetWidth();
	float height = (float) pImageInstance->GetHeight();

	c_pTexture->SetTextureStage(0);

	float sx = x;
	float sy = y;
	float ex = x + width;
	float ey = y + height;
	float z = 0.0f;

	float su = 0.0f;
	float sv = 0.0f;
	float eu = 1.0f;
	float ev = 1.0f;

	TPDTVertex vertices[4];
	vertices[0].position = TPosition(sx, sy, z);
	vertices[0].diffuse = DiffuseColor1;
	vertices[0].texCoord = TTextureCoordinate(su, sv);

	vertices[1].position = TPosition(ex, sy, z);
	vertices[1].diffuse = DiffuseColor2;
	vertices[1].texCoord = TTextureCoordinate(eu, sv);

	vertices[2].position = TPosition(sx, ey, z);
	vertices[2].diffuse = DiffuseColor1;
	vertices[2].texCoord = TTextureCoordinate(su, ev);

	vertices[3].position = TPosition(ex, ey, z);
	vertices[3].diffuse = DiffuseColor2;
	vertices[3].texCoord = TTextureCoordinate(eu, ev);

	STATEMANAGER.SetVertexDeclaration(ms_pntVS);
	// 2004.11.18.myevan.DrawIndexPrimitiveUP -> DynamicVertexBuffer
	CGraphicBase::SetDefaultIndexBuffer(DEFAULT_IB_FILL_RECT);
	if (CGraphicBase::SetPDTStream(vertices, 4))
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
}

void CPythonGraphic::RenderCoolTimeBox(float fxCenter, float fyCenter, float fRadius, float fTime)
{
	if (fTime >= 1.0f)
		return;

	fTime = std::max(0.0f, fTime);

	static D3DXCOLOR color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
	static WORD s_wBoxIndicies[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	static D3DXVECTOR2 s_v2BoxPos[8] =
	{
		D3DXVECTOR2( -1.0f, -1.0f ),
		D3DXVECTOR2( -1.0f,  0.0f ),
		D3DXVECTOR2( -1.0f, +1.0f ),
		D3DXVECTOR2(  0.0f, +1.0f ),
		D3DXVECTOR2( +1.0f, +1.0f ),
		D3DXVECTOR2( +1.0f,  0.0f ),
		D3DXVECTOR2( +1.0f, -1.0f ),
		D3DXVECTOR2(  0.0f, -1.0f ),
	};

	int iTriCount = int(8 - 8.0f * fTime);
	float fLastPercentage = (8 - 8.0f * fTime) - iTriCount;

	std::vector<TPDTVertex> vertices;
	TPDTVertex vertex;
	vertex.position.x = fxCenter;
	vertex.position.y = fyCenter;
	vertex.position.z = 0.0f;
	vertex.diffuse = color;
	vertex.texCoord.x = 0.0f;
	vertex.texCoord.x = 0.0f;
	vertices.push_back(vertex);
	vertex.position.x = fxCenter;
	vertex.position.y = fyCenter - fRadius;
	vertex.position.z = 0.0f;
	vertex.diffuse = color;
	vertex.texCoord.x = 0.0f;
	vertex.texCoord.x = 0.0f;
	vertices.push_back(vertex);

	for (int j = 0; j < iTriCount; ++j)
	{
		vertex.position.x = fxCenter + s_v2BoxPos[j].x * fRadius;
		vertex.position.y = fyCenter + s_v2BoxPos[j].y * fRadius;
		vertices.push_back(vertex);
	}

	if (fLastPercentage > 0.0f)
	{
		D3DXVECTOR2 * pv2Pos;
		D3DXVECTOR2 * pv2LastPos;

		assert((iTriCount-1+8)%8 >= 0 && (iTriCount-1+8)%8 < 8);
		assert((iTriCount+8)%8 >= 0 && (iTriCount+8)%8 < 8);
		pv2LastPos = &s_v2BoxPos[(iTriCount-1+8)%8];
		pv2Pos = &s_v2BoxPos[(iTriCount+8)%8];

		vertex.position.x = fxCenter + ((pv2Pos->x-pv2LastPos->x) * fLastPercentage + pv2LastPos->x) * fRadius;
		vertex.position.y = fyCenter + ((pv2Pos->y-pv2LastPos->y) * fLastPercentage + pv2LastPos->y) * fRadius;
		vertices.push_back(vertex);
		++iTriCount;
	}

	if (vertices.empty())
		return;

	if (SetPDTStream(&vertices[0], vertices.size()))
	{
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_DIFFUSE);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);
		STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	}
}

long CPythonGraphic::GenerateColor(float r, float g, float b, float a)
{
	return GetColor(r, g, b, a);
}

void CPythonGraphic::RenderDownButton(float sx, float sy, float ex, float ey)
{
	RenderBox2d(sx, sy, ex, ey);

	SetDiffuseColor(m_darkColor);
	RenderLine2d(sx, sy, ex, sy);
	RenderLine2d(sx, sy, sx, ey);

	SetDiffuseColor(m_lightColor);
	RenderLine2d(sx, ey, ex, ey);
	RenderLine2d(ex, sy, ex, ey);
}

void CPythonGraphic::RenderUpButton(float sx, float sy, float ex, float ey)
{
	RenderBox2d(sx, sy, ex, ey);

	SetDiffuseColor(m_lightColor);
	RenderLine2d(sx, sy, ex, sy);
	RenderLine2d(sx, sy, sx, ey);

	SetDiffuseColor(m_darkColor);
	RenderLine2d(sx, ey, ex, ey);
	RenderLine2d(ex, sy, ex, ey);
}

DWORD CPythonGraphic::GetAvailableMemory()
{
	return ms_lpd3dDevice->GetAvailableTextureMem();
}

CPythonGraphic::CPythonGraphic()
{
	m_lightColor = GetColor(1.0f, 1.0f, 1.0f);
	m_darkColor = GetColor(0.0f, 0.0f, 0.0f);
	
	memset(&m_backupViewport, 0, sizeof(D3DVIEWPORT9));

	m_fOrthoDepth = 1000.0f;
}

CPythonGraphic::~CPythonGraphic()
{
	Tracef("Python Graphic Clear\n");
}
