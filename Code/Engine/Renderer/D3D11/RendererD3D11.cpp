#include <Engine/StdAfx.h>
#include <Engine/Renderer/D3D11/RendererD3D11.h>
#include <Engine/RenderObjects/UIObject.h>
#include <Engine/Foundation/Object.h>
#include <Engine/IEngine.h>
#include <Engine/GlobalEnv.h>
#include <Engine/ICamera.h>
#include <Engine/ILight.h>
#include <Engine/RenderObjects/DebugHud.h>
#include <Engine/Renderer/IRenderState.h>
#include <Engine/Renderer/D3D11/VertexBufferD3D11.h>
#include <Engine/Renderer/D3D11/IndexBufferD3D11.h>
#include <Engine/Renderer/D3D11/ShaderD3D11.h>
#include <Engine/Renderer/D3D11/TextureD3D11.h>
#include <Engine/Renderer/D3D11/ConvertEnumD3D11.h>
#include <Engine/Renderer/D3D11/ConvertStructD3D11.h>
#include <Engine/Renderer/D3D11/InputLayoutD3D11.h>
#include <Engine/Renderer/D3D11/RenderStateD3D11.h>
#include <Engine/Renderer/D3D11/RenderToTextureD3D11.h>
#include <CommonLib/StringUtils.h>
#include <CommonLib/Hammersley.h>
#include <d3d11.h>
#include <D3DX11.h>
#include <d3dcompiler.h>

#define _RENDERER_FRAME_PROFILER_

using namespace fastbird;

//----------------------------------------------------------------------------
IRenderer* IRenderer::CreateD3D11Instance()
{
	return FB_NEW(RendererD3D11);
}

//----------------------------------------------------------------------------
RendererD3D11::RendererD3D11()
: mCurrentDSView(0)
{
	m_pDevice = 0;
	m_pFactory = 0;
	m_pSwapChain = 0;
	m_pImmediateContext = 0;
	m_pRenderTargetView = 0;
	m_pDepthStencil = 0;
	m_pDepthStencilView = 0;
	m_pFrameConstantsBuffer = 0;
	m_pObjectConstantsBuffer = 0;
	m_pMaterialConstantsBuffer = 0;
	m_pMaterialParametersBuffer = 0;
	m_pRareConstantsBuffer = 0;
	m_pBigBuffer = 0;
	m_pImmutableConstantsBuffer = 0;
	m_pWireframeRasterizeState = 0;
	m_pThreadPump = 0;
	mMultiSampleDesc.Count = 1;
	mMultiSampleDesc.Quality = 0;
	mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	mFrameConstants.gViewProj.MakeIdentity();
	mFrameConstants.gView.MakeIdentity();

	mBindedShader = 0;
	mBindedInputLayout = 0;
	mCurrentTopology = PRIMITIVE_TOPOLOGY_POINTLIST;
}

//----------------------------------------------------------------------------
RendererD3D11::~RendererD3D11()
{
	Deinit();	
	IEngine::Log(FB_DEFAULT_DEBUG_ARG, "DirectX11 is destroyed.");
}

//----------------------------------------------------------------------------
void RendererD3D11::Deinit()
{
	if (m_pThreadPump)
	{
		m_pThreadPump->WaitForAllItems();
		SAFE_RELEASE(m_pThreadPump);
	}
	__super::Deinit();
	mRasterizerMap.clear();
	mBlendMap.clear();
	mDepthStencilMap.clear();
	mSamplerMap.clear();

	m_pImmediateContext->ClearState();
	SAFE_RELEASE(m_pWireframeRasterizeState);
	SAFE_RELEASE(m_pMaterialParametersBuffer);
	SAFE_RELEASE(m_pMaterialConstantsBuffer);
	SAFE_RELEASE(m_pObjectConstantsBuffer);
	SAFE_RELEASE(m_pFrameConstantsBuffer);
	SAFE_RELEASE(m_pRareConstantsBuffer);
	SAFE_RELEASE(m_pBigBuffer);
	SAFE_RELEASE(m_pImmutableConstantsBuffer);
	for each(auto r in mRenderTargetTextures)
	{
		SAFE_RELEASE(r);
	}
	
	UIObject::ClearSharedRS();

	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pDepthStencil);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pSwapChain);
	// 0 is already released
	for (size_t i = 1; i<mSwapChains.size(); i++)
	{
		SAFE_RELEASE(mSwapChains[i]);
		SAFE_RELEASE(mRenderTargetViews[i]);
		SAFE_RELEASE(mDepthStencilViews[i]);
	}
	mSwapChains.clear();
	mRenderTargetViews.clear();
	mDepthStencilViews.clear();
	SAFE_RELEASE(m_pImmediateContext);

	if (gFBEnv->pConsole->GetEngineCommand()->r_ReportDeviceObjectLeak)
	{
		ID3D11Debug* pDebug;
		m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pDebug));
		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		SAFE_RELEASE(pDebug);
	}	
	
	SAFE_RELEASE(m_pDevice);

	IEngine::Log(FB_DEFAULT_DEBUG_ARG, "DirectX11 is deinitilized.");
}

//----------------------------------------------------------------------------
bool RendererD3D11::Init(int threadPool)
{
	HRESULT hr;
	try
	{
		hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&m_pFactory);
	}
	catch(...)
	{
	}
	if (FAILED(hr))
	{
		Log(FB_DEFAULT_DEBUG_ARG, "CreateDXGIFactory1() failed!");
		return false;
	}

	
	UINT i = 0; 
	IDXGIAdapter1 * pAdapter; 
	std::vector <IDXGIAdapter1*> vAdapters; 
	while(m_pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) 
	{ 
		vAdapters.push_back(pAdapter); 
		DXGI_ADAPTER_DESC1 adapterDesc;
		pAdapter->GetDesc1(&adapterDesc);
		++i; 
	}

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = 
	{
		D3D_DRIVER_TYPE_UNKNOWN,
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	int numDriverTypes = _ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3
	};
	int numFeatureLevels = _ARRAYSIZE(featureLevels);

	for( int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		mDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(vAdapters[0], mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pDevice, &mFeatureLevel, &m_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED( hr ))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "D3D11CreateDevice() failed!");
		return false;
	}
	
	unsigned msQuality = 0;	
	unsigned msCount = 1;
	hr = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R32G32B32A32_FLOAT, msCount, &msQuality);
	if (SUCCEEDED(hr))
	{
		hr = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R32G32B32A32_FLOAT, msCount, &msQuality);
		if (SUCCEEDED(hr))
		{
			mMultiSampleDesc.Count = msCount;
			mMultiSampleDesc.Quality = std::min(msQuality - 1, (unsigned)4);
		}
	}
	/*RECT rc;
	GetClientRect(gEnv->pEngine->GetWindowHandle(), &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	InitSwapChain(0, width, height);*/
	
	/*
	for( int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		mDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
										D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, &mFeatureLevel, &m_pImmediateContext);
		if( SUCCEEDED(hr))
			break;
	}
	if (FAILED( hr ))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to initialize D3D11!");
		return false;
	}
	*/
	
	//m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//------------------------------------------------------------------------
	// FRAME CONSTANT
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;    
	Desc.ByteWidth = sizeof( FRAME_CONSTANTS );
	hr = m_pDevice->CreateBuffer( &Desc, NULL, &m_pFrameConstantsBuffer );
	if (FAILED( hr ) )
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create constant buffer(FrameConstants)!");
		assert(0);
	}
	mFrameConstants.gView.MakeIdentity();
	mFrameConstants.gInvView.MakeIdentity();
	mFrameConstants.gViewProj.MakeIdentity();
	mFrameConstants.gInvViewProj.MakeIdentity();
	mFrameConstants.gLightViewProj.MakeIdentity();
	mFrameConstants.gDirectionalLightDir_Intensity = Vec4(1.f, -.5f, .5f, 1.f);
	mFrameConstants.gDirectionalLightDiffuse = Vec4(1.f, 1.f, 1.f, 1.f);
	mFrameConstants.gDirectionalLightSpecular = Vec4(1.f, 1.f, 1.f, 1.f);
	mFrameConstants.gCameraPos = Vec3(0.f, 0.f, 2.0f);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = m_pImmediateContext->Map(m_pFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 
		ConvertEnumD3D11(MAP_FLAG_NONE), &mappedResource);
	if (FAILED(hr))
		assert(0);
	memcpy(mappedResource.pData, &mFrameConstants, sizeof(FRAME_CONSTANTS));
	m_pImmediateContext->Unmap(m_pFrameConstantsBuffer, 0);

	//------------------------------------------------------------------------
	// OBJECT CONSTANT
	Desc.ByteWidth = sizeof( OBJECT_CONSTANTS );
	hr = m_pDevice->CreateBuffer( &Desc, NULL, &m_pObjectConstantsBuffer );
	if ( FAILED( hr ) )
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create constant buffer(ObjectConstants)!");
		assert(0);
	}

	//------------------------------------------------------------------------
	// MATERIAL CONSTANT
	Desc.ByteWidth = sizeof( MATERIAL_CONSTANTS );
	hr = m_pDevice->CreateBuffer( &Desc, NULL, &m_pMaterialConstantsBuffer );
	if ( FAILED( hr ) )
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create constant buffer(MaterialConstants)!");
		assert(0);
	}

	//------------------------------------------------------------------------
	// MATERIAL PARAMETERS
	Desc.ByteWidth = sizeof( MATERIAL_PARAMETERS );
	hr = m_pDevice->CreateBuffer( &Desc, NULL, &m_pMaterialParametersBuffer );
	if ( FAILED( hr ) )
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create constant buffer(MaterialParameters)!");
		assert(0);
	}

	Desc.ByteWidth = sizeof(RARE_CONSTANTS);
	hr = m_pDevice->CreateBuffer( &Desc, NULL, &m_pRareConstantsBuffer );
	if ( FAILED( hr ) )
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Failed to create constant buffer(Rare constants)!");
		assert(0);
	}

	Desc.ByteWidth = sizeof(BIG_BUFFER);
	hr = m_pDevice->CreateBuffer(&Desc, NULL, &m_pBigBuffer);
	if (FAILED(hr))
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Failed to create big buffer!");
		assert(0);
	}

	Desc.ByteWidth = sizeof(IMMUTABLE_CONSTANTS);
	hr = m_pDevice->CreateBuffer( &Desc, NULL, &m_pImmutableConstantsBuffer );
	if ( FAILED( hr ) )
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Failed to create constant buffer(Immutable constants)!");
		assert(0);
	}
	else
	{
		std::vector<Vec2> hammersley;
		GenerateHammersley(256, hammersley);
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr;
		hr = m_pImmediateContext->Map( m_pImmutableConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		assert(hr == S_OK);
		IMMUTABLE_CONSTANTS* pConstants = (IMMUTABLE_CONSTANTS*)mappedResource.pData;
		memcpy(pConstants, &hammersley[0], sizeof(Vec2) * 256);
		m_pImmediateContext->Unmap(m_pImmutableConstantsBuffer, 0);
		m_pImmediateContext->PSSetConstantBuffers(5, 1, &m_pImmutableConstantsBuffer);
	}

	//------------------------------------------------------------------------
	// Wireframe Render state
	D3D11_RASTERIZER_DESC RasterizerDesc;
	RasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	RasterizerDesc.CullMode = D3D11_CULL_NONE;
	RasterizerDesc.FrontCounterClockwise = FALSE;
	RasterizerDesc.DepthBias = 0;
	RasterizerDesc.DepthBiasClamp = 0.0f;
	RasterizerDesc.SlopeScaledDepthBias = 0.0f;
	RasterizerDesc.DepthClipEnable = TRUE;
	RasterizerDesc.ScissorEnable = FALSE;
	RasterizerDesc.MultisampleEnable = FALSE;
	RasterizerDesc.AntialiasedLineEnable = FALSE;
	hr = m_pDevice->CreateRasterizerState( &RasterizerDesc, &m_pWireframeRasterizeState );
	if (FAILED(hr))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create an wireframe rasterizer state!");
		assert(0);
	}

	if (threadPool!=0)
	{
		hr = D3DX11CreateThreadPump(1, threadPool, &m_pThreadPump);
		if (FAILED(hr))
		{
			Log(FB_DEFAULT_DEBUG_ARG, "Failed to create thread pump");
		}
	}

	return true;
}

//----------------------------------------------------------------------------
int RendererD3D11::InitSwapChain(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC sd={};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc = mMultiSampleDesc;
	sd.Windowed = true;

	assert(m_pDevice);
	IDXGISwapChain* pSwapChain;
	HRESULT hr = m_pFactory->CreateSwapChain(m_pDevice, &sd, &pSwapChain);
	if (FAILED(hr))
	{
		Error(FB_DEFAULT_DEBUG_ARG, "CreateSwapChain failed!");
		assert(0);
		return false;
	}

	// RenderTargetView
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer);
	if (FAILED(hr))
	{
		Error(FB_DEFAULT_DEBUG_ARG, "Failed to get backbuffer!");
		assert(0);
		SAFE_RELEASE(pSwapChain);
		return false;
	}
	ID3D11RenderTargetView* pRenderTargetView = NULL;
	hr = m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		Error(FB_DEFAULT_DEBUG_ARG, "Failed to create a render target view!");
		assert(0);
		SAFE_RELEASE(pSwapChain);
		return false;
	}

	// DepthStencilView
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc = mMultiSampleDesc;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;	
	ID3D11Texture2D *pDepthStencil = 0;
	hr = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &pDepthStencil);
	mRenderTargetTextures.push_back(pDepthStencil);
	ID3D11DepthStencilView* pDepthStencilView = 0;
	if (FAILED(hr))
	{
		Error(FB_DEFAULT_DEBUG_ARG, "Failed to create the depth stencil texture!");
		assert(0);
	}
	else
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
		viewDesc.Format = mDepthStencilFormat;
		if (mMultiSampleDesc.Count==1)
		{
			viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}
		else
		{
			viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		viewDesc.Flags = 0;
		viewDesc.Texture2D.MipSlice = 0;
		hr = m_pDevice->CreateDepthStencilView(pDepthStencil, &viewDesc, &pDepthStencilView);
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create the depth stencil view!");
		}
		else
		{
			mDepthStencilCreated = true;
		}
	}

	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	if (mSwapChains.empty())
	{
		mWidth = width;
		mHeight = height;
		mCropWidth = width - width % 8;
		mCropHeight = height - height % 8;
		m_pSwapChain = pSwapChain;
		m_pRenderTargetView = pRenderTargetView;
		m_pDepthStencilView = pDepthStencilView;
		mViewPort = vp;
		m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
		mCurrentRTViews.clear();
		mCurrentRTViews.push_back(m_pRenderTargetView);
		mCurrentDSView = m_pDepthStencilView;
		m_pImmediateContext->RSSetViewports(1, &mViewPort);
		ICamera* pCamera = gFBEnv->pEngine->GetCamera(0);
		if (pCamera)
		{
			pCamera->SetWidth((float)width);
			pCamera->SetHeight((float)height);
		}
		OnPrepared();
	}
	mSwapChains.push_back(pSwapChain);
	mRenderTargetViews.push_back(pRenderTargetView);
	mDepthStencilViews.push_back(pDepthStencilView);
	mViewports.push_back(vp);
	return mSwapChains.size()-1;
}

//----------------------------------------------------------------------------
void RendererD3D11::Clear(float r, float g, float b, float a, float z, UINT8 stencil)
{
	float ClearColor[4] = { r, g, b, a }; // red,green,blue,alpha
	for (size_t i=0; i<mCurrentRTViews.size(); i++)
	{
		if (mCurrentRTViews[i])
			m_pImmediateContext->ClearRenderTargetView( mCurrentRTViews[i], ClearColor );
	}
	if (mCurrentDSView)
		m_pImmediateContext->ClearDepthStencilView( mCurrentDSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, z, stencil );
}

//----------------------------------------------------------------------------
void RendererD3D11::Clear()
{
	Clear(mClearColor.r(), mClearColor.g(), mClearColor.b(), mClearColor.a(),
		mDepthClear, mStencilClear);
}

//----------------------------------------------------------------------------
void RendererD3D11::Clear(float r, float g, float b, float a)// only color
{
	float ClearColor[4] = { r, g, b, a }; // red,green,blue,alpha
	for (size_t i = 0; i<mCurrentRTViews.size(); i++)
	{
		if (mCurrentRTViews[i])
			m_pImmediateContext->ClearRenderTargetView(mCurrentRTViews[i], ClearColor);
	}
}

//----------------------------------------------------------------------------
void RendererD3D11::UpdateFrameConstantsBuffer()
{
	if (mCamera)
	{
		mFrameConstants.gView = mCamera->GetViewMat();
		mFrameConstants.gInvView = mCamera->GetInvViewMat();
		mFrameConstants.gViewProj = mCamera->GetViewProjMat();
		mFrameConstants.gInvViewProj = mCamera->GetInvViewProjMat();
		ILight* pLight = GetDirectionalLight();
		ICamera* pLightCam = pLight ? pLight->GetCamera() : 0;
		if (pLightCam)
			mFrameConstants.gLightViewProj = pLightCam->GetViewProjMat();
	}
	else
	{
		assert(0);
	}
	ILight* pLight = mDirectionalLightOverride ? mDirectionalLightOverride : mDirectionalLight;
	mFrameConstants.gDirectionalLightDir_Intensity = float4(pLight->GetPosition(), pLight->GetIntensity());
	mFrameConstants.gDirectionalLightDiffuse = float4(pLight->GetDiffuse(), 1.0f);
	mFrameConstants.gDirectionalLightSpecular = float4(pLight->GetSpecular(), 1.0f);
	mFrameConstants.gCameraPos = mCamera->GetPos();
	mFrameConstants.gTime = gFBEnv->pTimer->GetTime();
	mFrameConstants.gDeltaTime.x = gFBEnv->pTimer->GetDeltaTime();
	long x, y;
	gFBEnv->pEngine->GetMousePos(x, y);
	mFrameConstants.gMousePos.x = (float)x;
	mFrameConstants.gMousePos.y = (float)y;
	bool lbuttonDown = gFBEnv->pEngine->IsMouseLButtonDown();
	mFrameConstants.gMousePos.z = lbuttonDown ? (float)x : 0;
	mFrameConstants.gMousePos.w = lbuttonDown ? (float)y : 0;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pImmediateContext->Map( m_pFrameConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	memcpy(mappedResource.pData, &mFrameConstants, sizeof(FRAME_CONSTANTS));
	m_pImmediateContext->Unmap(m_pFrameConstantsBuffer, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pFrameConstantsBuffer);
	m_pImmediateContext->GSSetConstantBuffers(0, 1, &m_pFrameConstantsBuffer);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pFrameConstantsBuffer);

	if (m_pImmutableConstantsBuffer)
		m_pImmediateContext->PSSetConstantBuffers(6, 1, &m_pImmutableConstantsBuffer);
}

void RendererD3D11::ClearState()
{
	// do not use if possible.
	m_pImmediateContext->ClearState();

	for (int i = 0; i < SAMPLERS::NUM; ++i)
	{
		assert(mDefaultSamplers[i] != 0);
		__super::SetSamplerState((SAMPLERS::Enum)i, BINDING_SHADER_PS, i);
	}

	__super::SetSamplerState(SAMPLERS::POINT, BINDING_SHADER_VS, SAMPLERS::POINT);
}

//----------------------------------------------------------------------------
void RendererD3D11::UpdateObjectConstantsBuffer(void* pData)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pImmediateContext->Map( m_pObjectConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	memcpy(mappedResource.pData, pData, sizeof(OBJECT_CONSTANTS));
	m_pImmediateContext->Unmap(m_pObjectConstantsBuffer, 0);
	m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_pObjectConstantsBuffer);
	m_pImmediateContext->GSSetConstantBuffers(1, 1, &m_pObjectConstantsBuffer);
}

//----------------------------------------------------------------------------
void RendererD3D11::UpdateMaterialConstantsBuffer(void* pData)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr;
	hr = m_pImmediateContext->Map( m_pMaterialConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	assert(hr == S_OK);
	memcpy(mappedResource.pData, pData, sizeof(OBJECT_CONSTANTS));
	m_pImmediateContext->Unmap(m_pMaterialConstantsBuffer, 0);
	m_pImmediateContext->PSSetConstantBuffers(2, 1, &m_pMaterialConstantsBuffer);
}

//----------------------------------------------------------------------------
void RendererD3D11::UpdateRareConstantsBuffer()
{
	static int entered = 0;
	++entered;
	if (entered != 1)
	{
		--entered;
		return;
	}
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr;
	hr = m_pImmediateContext->Map( m_pRareConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	assert(hr == S_OK);
	RARE_CONSTANTS* pRareConstants = (RARE_CONSTANTS*)mappedResource.pData;
	pRareConstants->gProj = mCamera->GetProjMat();
	mCamera->GetNearFar(pRareConstants->gNearFar.x, pRareConstants->gNearFar.y);
	pRareConstants->gScreenSize.x = (float)mWidth;
	pRareConstants->gScreenSize.y = (float)mHeight;
	pRareConstants->gTangentTheta = tan(mCamera->GetFOV()/2.0f);
	pRareConstants->gScreenRatio = mWidth / (float)mHeight;
	pRareConstants->gMiddleGray_Star_Bloom_Empty = Vec4(mMiddleGray, mStarPower, mBloomPower, 0);
	pRareConstants->gFogColor = gFBEnv->pEngine->GetScene()->GetFogColor().GetVec4();
	m_pImmediateContext->Unmap(m_pRareConstantsBuffer, 0);
	m_pImmediateContext->VSSetConstantBuffers(4, 1, &m_pRareConstantsBuffer);
	m_pImmediateContext->GSSetConstantBuffers(4, 1, &m_pRareConstantsBuffer);
	m_pImmediateContext->PSSetConstantBuffers(4, 1, &m_pRareConstantsBuffer);
	--entered;
}

//----------------------------------------------------------------------------
void* RendererD3D11::MapMaterialParameterBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pImmediateContext->Map( m_pMaterialParametersBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	return mappedResource.pData;	
}

//----------------------------------------------------------------------------
void RendererD3D11::UnmapMaterialParameterBuffer()
{
	m_pImmediateContext->Unmap(m_pMaterialParametersBuffer, 0);
	m_pImmediateContext->VSSetConstantBuffers(3, 1, &m_pMaterialParametersBuffer);
	m_pImmediateContext->GSSetConstantBuffers(3, 1, &m_pMaterialParametersBuffer);
	m_pImmediateContext->PSSetConstantBuffers(3, 1, &m_pMaterialParametersBuffer);
}

//----------------------------------------------------------------------------
void* RendererD3D11::MapBigBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_pImmediateContext->Map(m_pBigBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	return mappedResource.pData;
}

//----------------------------------------------------------------------------
void RendererD3D11::UnmapBigBuffer()
{
	m_pImmediateContext->Unmap(m_pBigBuffer, 0);
	m_pImmediateContext->PSSetConstantBuffers(5, 1, &m_pBigBuffer);
}

unsigned RendererD3D11::GetMultiSampleCount() const
{
	return mMultiSampleDesc.Count;
}
//----------------------------------------------------------------------------
IRenderToTexture* RendererD3D11::CreateRenderToTexture(bool everyframe)
{
	if (everyframe)
	{
		mRenderToTextures.push_back(FB_NEW(RenderToTextureD3D11));
		return mRenderToTextures.back();
	}
	else
	{
		return FB_NEW(RenderToTextureD3D11);
	}
}

void RendererD3D11::DeleteRenderToTexture(IRenderToTexture* removeRT)
{
	FB_DELETE(removeRT);
	mRenderToTextures.erase(
		std::remove(mRenderToTextures.begin(), mRenderToTextures.end(), removeRT),
		mRenderToTextures.end());		
}

//----------------------------------------------------------------------------
void RendererD3D11::Present()
{
	HRESULT hr = m_pSwapChain->Present(1, 0);
	assert(!FAILED(hr));

	if (m_pThreadPump)
	{
		//UINT io, process, device;
		//m_pThreadPump->GetQueueStatus(&io, &process, &device);
		m_pThreadPump->ProcessDeviceWorkItems(2);

		for (auto it = mCheckTextures.begin(); it!= mCheckTextures.end(); )
		{
			if ((*it)->mHr==S_OK)
			{
				TextureD3D11* pt = (TextureD3D11*)(*it);
				size_t id = pt->GetTextureID();
				char buf[256];
				sprintf_s(buf, "TextureID(%u)", id);
				FB_SET_DEVICE_DEBUG_NAME(pt, buf);
				it = mCheckTextures.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

//----------------------------------------------------------------------------
void RendererD3D11::SetVertexBuffer(unsigned int startSlot, unsigned int numBuffers,
	IVertexBuffer* pVertexBuffers[], unsigned int strides[], unsigned int offsets[])
{
	if (numBuffers == 0 && pVertexBuffers == 0)
	{
		ID3D11Buffer* pHardwareBuffers[1] = { 0 };
		unsigned strides[1] = { 0 };
		unsigned offsets[1] = { 0 };
		m_pImmediateContext->IASetVertexBuffers(0, 1, pHardwareBuffers, strides, offsets);
		return;
	}
	ID3D11Buffer* pHardwareBuffers[32]={0};
	for (int i=0; i<(int)numBuffers; i++)
	{
		if (pVertexBuffers[i])
		{
			ID3D11Buffer* pBuffer = static_cast<VertexBufferD3D11*>(pVertexBuffers[i])->GetHardwareBuffer();
			pHardwareBuffers[i] = pBuffer;
		}
	}
	m_pImmediateContext->IASetVertexBuffers(startSlot, numBuffers, pHardwareBuffers, strides, offsets);
}

//----------------------------------------------------------------------------
void RendererD3D11::SetIndexBuffer(IIndexBuffer* pIndexBuffer)
{
	IndexBufferD3D11* pIndexBufferD3D11 = static_cast<IndexBufferD3D11*>(pIndexBuffer);
	ID3D11Buffer* pHardwareBuffer = pIndexBufferD3D11->GetHardwareBuffer();
	m_pImmediateContext->IASetIndexBuffer(pHardwareBuffer, pIndexBufferD3D11->GetFormatD3D11(), pIndexBufferD3D11->GetOffset());
}

//----------------------------------------------------------------------------
void RendererD3D11::SetTexture(ITexture* pTexture, BINDING_SHADER shaderType, unsigned int slot)
{
	TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pTexture);
	
	ID3D11ShaderResourceView* pSRV = pTextureD3D11 ? pTextureD3D11->GetHardwareResourceView() : 0;
	switch(shaderType)
	{
	case BINDING_SHADER_VS:
		m_pImmediateContext->VSSetShaderResources(slot, 1, &pSRV);
		break;
	case BINDING_SHADER_PS:
		m_pImmediateContext->PSSetShaderResources(slot, 1, &pSRV);
		break;
	default:
		assert(0);
		break;	
	}
}

void RendererD3D11::SetTextures(ITexture* pTextures[], int num, BINDING_SHADER shaderType, int startSlot)
{
	std::vector<ID3D11ShaderResourceView*> rvs;
	rvs.reserve(num);
	for (int i = 0; i < num; i++)
	{
		TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pTextures[i]);
		if (pTextureD3D11)
		{
			rvs.push_back(pTextureD3D11->GetHardwareResourceView());
		}
		else
		{
			rvs.push_back(0);
		}
	}

	switch (shaderType)
	{
	case BINDING_SHADER_VS:
		m_pImmediateContext->VSSetShaderResources(startSlot, num, &rvs[0]);
		break;
	case BINDING_SHADER_PS:
		m_pImmediateContext->PSSetShaderResources(startSlot, num, &rvs[0]);
		break;
	default:
		assert(0);
		break;
	}
	
}

void RendererD3D11::GenerateMips(ITexture* pTexture)
{
	TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pTexture);
	assert(pTextureD3D11);
	ID3D11ShaderResourceView* pSRV = pTextureD3D11->GetHardwareResourceView();
	assert(pSRV);
	m_pImmediateContext->GenerateMips(pSRV);
}

//----------------------------------------------------------------------------
IVertexBuffer* RendererD3D11::CreateVertexBuffer(void* data, unsigned stride, 
	unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag)
{
	if (usage == BUFFER_USAGE_IMMUTABLE && data==0)
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create vertex buffer! "
			"Immutable needs data pointer.");
		assert(0);
		return 0;		
	}

	VertexBufferD3D11* pVertexBufferD3D11 = VertexBufferD3D11::CreateInstance(stride, numVertices);
	assert(pVertexBufferD3D11);

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = ConvertEnumD3D11(usage);
	bufferDesc.ByteWidth = stride * numVertices;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = ConvertEnumD3D11(accessFlag);
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	ID3D11Buffer* pHardwareBuffer = 0;
	HRESULT hr;
	if (data)
	{
		hr = m_pDevice->CreateBuffer(&bufferDesc, &initData, &pHardwareBuffer);	
	}
	else
	{
		hr = m_pDevice->CreateBuffer(&bufferDesc, 0, &pHardwareBuffer);
	}

	if (FAILED(hr))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create a vertex buffer!");
	}
	else
	{
		static unsigned ID = 0;
		char buf[256];
		sprintf_s(buf, "VertexBuffer ID(%d)", ID++);
		pHardwareBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}
	

	pVertexBufferD3D11->SetHardwareBuffer(pHardwareBuffer);

	return pVertexBufferD3D11;
}

//----------------------------------------------------------------------------
IIndexBuffer* RendererD3D11::CreateIndexBuffer(void* data, unsigned int numIndices, INDEXBUFFER_FORMAT format)
{
	IndexBufferD3D11* pIndexBufferD3D11 = IndexBufferD3D11::CreateInstance(numIndices, format);
	assert(pIndexBufferD3D11);

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = pIndexBufferD3D11->GetSize();
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	ID3D11Buffer* pHardwareBuffer = 0;
	HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, &initData, &pHardwareBuffer);
	if (FAILED(hr))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create a index buffer!");
	}
	
	pIndexBufferD3D11->SetHardwareBuffer(pHardwareBuffer);

	return pIndexBufferD3D11;
}

class IncludeProcessor : public ID3DInclude
{
public:
	IncludeProcessor(const char* filename, IShader* pshader)
		: mShader(pshader)
	{
		assert(mShader);
		mWorkingDirectory = fastbird::GetDirectoryPath(filename) + "/";
	}
	~IncludeProcessor()
	{
		mShader->SetIncludeFiles(mProcessedFiles);
	}
	virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Open(
		D3D10_INCLUDE_TYPE IncludeType,
		LPCSTR pFileName,
		LPCVOID pParentData,
		LPCVOID *ppData,
		UINT *pBytes
		)
	{
		assert(IncludeType == D3D10_INCLUDE_LOCAL);
		FILE* file = 0;
		std::string filepath = mWorkingDirectory + pFileName;
		fopen_s(&file, filepath.c_str(), "rb");
		if (file == 0)
		{
			filepath = "code/engine/renderer/shaders/";
			filepath += pFileName;
			fopen_s(&file, filepath.c_str(), "rb");
			if (file == 0)
			{
				Error("Failed to open include file %s", filepath.c_str());
				return S_OK;
			}
		}

		ToLowerCase(filepath);
		mProcessedFiles.insert(filepath);

		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		rewind(file);
		char* buffer = FB_ARRNEW(char, size);
		int elements = fread(buffer, 1, size, file);
		assert(elements == size);
		*ppData = buffer;
		*pBytes = size;
		fclose(file);
		return S_OK;
	}

	virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Close(LPCVOID pData)
	{
		FB_ARRDELETE((void*)pData);
		return S_OK;
	}

private:
	std::string mWorkingDirectory;
	std::set<std::string> mProcessedFiles;
	IShader* mShader;

};

//----------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const char* filename, const char* entryPoint, 
	const char* shaderModel, ID3DBlob** ppBlobOut, D3D_SHADER_MACRO* pDefines=0, IncludeProcessor* includeProcessor=0)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* pErrorBlob = 0;
	hr = D3DX11CompileFromFile(filename, pDefines, includeProcessor, entryPoint, shaderModel, dwShaderFlags, 0,
		0, ppBlobOut, &pErrorBlob, 0);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			IEngine::Error("[Error] CompileShaderFromFile %s failed!", filename);
			IEngine::Error((const char*)pErrorBlob->GetBufferPointer());
			Beep(100, 50);
		}
		else
		{
			Error("Shader compile error. maybe file not found %s", filename);
			Beep(100, 50);
		}
	}
	if (pErrorBlob)
		pErrorBlob->Release();
	return hr;
}

//----------------------------------------------------------------------------
IShader* RendererD3D11::CreateShader(const char* path, int shaders,
	const IMaterial::SHADER_DEFINES& defines,
	IShader* pReloadingShader/*=0*/)
{
	std::string filepath(path);
	ToLowerCase(filepath);
	ShaderD3D11* pShader;
	if (pReloadingShader)
	{
		pShader = (ShaderD3D11*)pReloadingShader;
		assert(strcmp(filepath.c_str(),pShader->GetName())==0);
	}
	else
	{
		IShader* pShaderFromCache = Shader::FindShader(filepath.c_str(), defines);
		if (pShaderFromCache)
			return pShaderFromCache;

		pShader = ShaderD3D11::CreateInstance(filepath.c_str());
	}
	assert(pShader);
	pShader->SetShaderDefines(defines);
	pShader->SetBindingShaders(shaders);

	char onlyname[MAX_PATH];
	strcpy_s( onlyname, MAX_PATH, StripPath(filepath.c_str()) );
	StripExtension(onlyname);

	std::vector<D3D_SHADER_MACRO> shaderMacros;
	for (DWORD i=0; i<defines.size(); i++)
	{
		shaderMacros.push_back(D3D_SHADER_MACRO());
		shaderMacros.back().Name = defines[i].name.c_str();
		shaderMacros.back().Definition = defines[i].value.c_str();
	}
	D3D_SHADER_MACRO* pShaderMacros = 0;
	if (!shaderMacros.empty())
	{
		shaderMacros.push_back(D3D_SHADER_MACRO());
		shaderMacros.back().Name = 0;
		shaderMacros.back().Definition = 0;
		pShaderMacros = &shaderMacros[0];
	}	

	IncludeProcessor includeProcessor(filepath.c_str(), pShader);
	HRESULT hr;
	// Load VS
	if (shaders & BINDING_SHADER_VS)
	{
		std::string VSName = onlyname;
		VSName+="_VertexShader";
		ID3DBlob* pVSBlob = 0;
		hr = CompileShaderFromFile(filepath.c_str(), VSName.c_str(), "vs_5_0", &pVSBlob,
			pShaderMacros, &includeProcessor);
		if (FAILED(hr))
		{		
			SAFE_RELEASE(pVSBlob);
			pShader->SetCompileFailed(true);
			return pShader;
		}
		// Create VS
		ID3D11VertexShader* pVertexShader = 0;
		hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), 
			pVSBlob->GetBufferSize(), 0, &pVertexShader);
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, pVSBlob->GetBufferPointer());
			SAFE_RELEASE(pVSBlob);
			pShader->SetCompileFailed(true);
			return pShader;
		}
		else
		{
			static unsigned vsID = 0;
			char buf[256];
			sprintf_s(buf, "VS ID(%u)", vsID++);
			pVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
		pShader->SetVertexShader(pVertexShader);
		pShader->SetVertexShaderBytecode(pVSBlob); // pVSBlob will be released here
	}

	// Load GS
	if (shaders & BINDING_SHADER_GS)
	{
		std::string GSName = onlyname;
		GSName+="_GeometryShader";
		ID3DBlob* pGSBlob = 0;
		HRESULT hr = CompileShaderFromFile(filepath.c_str(), GSName.c_str(), "gs_5_0", &pGSBlob,
			pShaderMacros, &includeProcessor);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pGSBlob);
			pShader->SetCompileFailed(true);
			return pShader;
		}
		// Create GS
		ID3D11GeometryShader* pGeometryShader = 0;
		hr = m_pDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), 
			pGSBlob->GetBufferSize(), 0, &pGeometryShader);
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, pGSBlob->GetBufferPointer());
			SAFE_RELEASE(pGSBlob);
			pShader->SetCompileFailed(true);
			return pShader;
		}
		else
		{
			pShader->SetGeometryShader(pGeometryShader);
		}	
		SAFE_RELEASE(pGSBlob);
		
	}

	// Load PS
	if (shaders & BINDING_SHADER_PS)
	{
		std::string PSName = onlyname;
		PSName+="_PixelShader";
		ID3DBlob* pPSBlob = 0;
		hr = CompileShaderFromFile(filepath.c_str(), PSName.c_str(), "ps_5_0", &pPSBlob,
			pShaderMacros, &includeProcessor);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pPSBlob);
			pShader->SetCompileFailed(true);
			return pShader;
		}
		// Create PS
		ID3D11PixelShader* pPixelShader = 0;
		hr = m_pDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), 
			pPSBlob->GetBufferSize(), 0, &pPixelShader);
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, pPSBlob->GetBufferPointer());
			SAFE_RELEASE(pPSBlob);
			pShader->SetCompileFailed(true);
			return pShader;
		}
		pShader->SetPixelShader(pPixelShader);
		SAFE_RELEASE(pPSBlob);
	}
	pShader->SetCompileFailed(false);

	return pShader;
}

//----------------------------------------------------------------------------
ITexture* RendererD3D11::CreateTexture(const Vec2I& size, int mipLevels, int arraySize)
{
	assert(0);
	return 0;
}

//----------------------------------------------------------------------------
ITexture* RendererD3D11::CreateTexture(const char* file, ITexture* pReloadingTexture/*=0*/)
{	
	std::string filepath(file);
	ToLowerCase(filepath);
	if (!pReloadingTexture)
	{
		TextureCache::iterator it = mTextureCache.Find(filepath);
		if (it!=mTextureCache.end())
		{
			return it->second->Clone();
		}
	}

	ID3D11ShaderResourceView* pSRView = 0;
	D3DX11_IMAGE_INFO imageInfo;
	HRESULT hr = D3DX11GetImageInfoFromFile(filepath.c_str(), 0, &imageInfo, 0);
	if (FAILED(hr))
	{
		Error("[Error] Failed to get image info (%s)", filepath.c_str());
	}

	TextureD3D11* pTexture = 0;
	if (pReloadingTexture)
	{
		pTexture = (TextureD3D11*)pReloadingTexture;

		bool found = false;
		FB_FOREACH(it, mCheckTextures)
		{
			if ((*it)->GetName() == file)
				return pTexture;
		}
	}
	else
	{
		pTexture = TextureD3D11::CreateInstance();
		mTextureCache.Insert( std::make_pair(filepath, pTexture) );
		pTexture->SetName(filepath.c_str());
	}

	SAFE_RELEASE(pTexture->mSRView);
	if (m_pThreadPump)
	{
		pTexture->mHr = S_FALSE;
		hr = D3DX11CreateShaderResourceViewFromFile(m_pDevice, filepath.c_str(), 
			&pTexture->mLoadInfo, m_pThreadPump, 
			&pTexture->mSRView, &pTexture->mHr);
		mCheckTextures.push_back(pTexture);
	}
	else
	{
		hr = D3DX11CreateShaderResourceViewFromFile(m_pDevice, filepath.c_str(), 
			&pTexture->mLoadInfo, nullptr, 
			&pTexture->mSRView, nullptr);

		size_t tid = pTexture->GetTextureID();
		char buf[256];
		sprintf_s(buf, "Texture(%u)", tid);
		FB_SET_DEVICE_DEBUG_NAME(pTexture, buf);
	}
	pTexture->SetSize(Vec2I(imageInfo.Width, imageInfo.Height));
	
	if (FAILED(hr))
	{
		Error("[Error] Failed to load a texture (%s).", filepath.c_str());		
	}

	if (pReloadingTexture)
	{
		TextureD3D11* pT = (TextureD3D11*)pReloadingTexture;
		pT->OnReloaded();
		return pReloadingTexture;
	}
	else
	{
		return pTexture->Clone();
	}
	
	
	/*
	FIBITMAP* pImage = LoadImage(file, 0);
	if (!pImage)
	{
		assert(pImage);
		return pTexture;
	}
	
	FREE_IMAGE_TYPE type = FreeImage_GetImageType(pImage);

	unsigned bpp = FreeImage_GetBPP(pImage);
	unsigned width = FreeImage_GetWidth(pImage);
	unsigned height = FreeImage_GetHeight(pImage);
	unsigned line = FreeImage_GetLine(pImage); // width in bytes
	unsigned pitch = FreeImage_GetPitch(pImage);
	ID3D11Texture2D *pTextureD3D11 = NULL;

	switch(type)
	{
	case FIT_BITMAP:
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;

			RGBQUAD* pSrc;
			if ( bpp == 32)
			{				
				pSrc = (RGBQUAD*)FreeImage_GetBits(pImage);
			}
			else if (bpp == 24)
			{
				pSrc = new RGBQUAD[width*height];
				RGBQUAD* pSrcIt = pSrc;
				for(unsigned y = 0; y < height; y++) 
				{
					RGBTRIPLE* bits = (RGBTRIPLE*)FreeImage_GetScanLine(pImage, y);
					for(unsigned x = 0; x < width; x++) 
					{
						pSrcIt->rgbBlue = bits->rgbtBlue;
						pSrcIt->rgbGreen = bits->rgbtGreen;
						pSrcIt->rgbRed = bits->rgbtRed;
						pSrcIt->rgbReserved = 255;
						// jump to next pixel
						pSrcIt++;
						bits++;						
					}
				}
			}

			D3D11_SUBRESOURCE_DATA sd;
			sd.pSysMem  = pSrc;
			sd.SysMemPitch = pitch;
			sd.SysMemSlicePitch = 0;
			m_pDevice->CreateTexture2D( &desc, &sd, &pTextureD3D11 );
		}
		break;

	default:
		assert(0);
	}
	pTexture->SetHardwareTexture(pTextureD3D11);

	FreeImage_Unload(pImage);
	*/
}

//----------------------------------------------------------------------------
// type : TEXTURE_TYPE
ITexture* RendererD3D11::CreateTexture(void* data, int width, int height, PIXEL_FORMAT format,
	BUFFER_USAGE usage, int  buffer_cpu_access, int type)
{
	bool engineUsingMS = mMultiSampleDesc.Count != 1;
	bool cubeMap = (type & TEXTURE_TYPE_CUBE_MAP) != 0;

	if (type & TEXTURE_TYPE_DEPTH_STENCIL_SRV)
	{
		assert(format == PIXEL_FORMAT_D32_FLOAT);
		if (format != PIXEL_FORMAT_R32_TYPELESS)
		{
			Log("Forcing shader bindable depth buffer format to PIXEL_FORMAT_R32_TYPELESS");
			format = PIXEL_FORMAT_R32_TYPELESS;
		}
	}
	ID3D11Texture2D *pTextureD3D11 = NULL;
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	if (type & TEXTURE_TYPE_MIPS)
	{
		assert(!(type & TEXTURE_TYPE_MULTISAMPLE));
		desc.MipLevels = GetMipLevels((float)std::min(width, height));
	}
		
	desc.ArraySize = cubeMap ? 6 : 1;
	desc.Format = ConvertEnumD3D11(format);
	//bool renderTarget = (type & TEXTURE_TYPE_RENDER_TARGET) || (type & TEXTURE_TYPE_RENDER_TARGET_SRV);
	//bool depthStencil = (type & TEXTURE_TYPE_DEPTH_STENCIL) || (type & TEXTURE_TYPE_DEPTH_STENCIL_SRV);
	if (engineUsingMS && type & TEXTURE_TYPE_MULTISAMPLE)
	{
		desc.SampleDesc = mMultiSampleDesc;
	}
	else
	{
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
	}
	desc.Usage = ConvertEnumD3D11(usage);
	desc.BindFlags = 0;
	if ( usage != BUFFER_USAGE_STAGING && 
		!(type & TEXTURE_TYPE_RENDER_TARGET) && !(type & TEXTURE_TYPE_DEPTH_STENCIL))
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if ((type & TEXTURE_TYPE_RENDER_TARGET) || (type & TEXTURE_TYPE_RENDER_TARGET_SRV))
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	else if ( (type & TEXTURE_TYPE_DEPTH_STENCIL) || (type & TEXTURE_TYPE_DEPTH_STENCIL_SRV) )
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	if (buffer_cpu_access & BUFFER_CPU_ACCESS_READ)
		desc.CPUAccessFlags = ConvertEnumD3D11(BUFFER_CPU_ACCESS_READ);
	if (buffer_cpu_access & BUFFER_CPU_ACCESS_WRITE)
		desc.CPUAccessFlags |= ConvertEnumD3D11(BUFFER_CPU_ACCESS_WRITE);
	desc.MiscFlags = cubeMap ? 
		D3D11_RESOURCE_MISC_TEXTURECUBE: 
		0;
	if (type & TEXTURE_TYPE_MIPS)
	{
		assert(!(type & TEXTURE_TYPE_MULTISAMPLE));
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	std::vector<D3D11_SUBRESOURCE_DATA> sds;
	if (data)
	{
		if (cubeMap)
		{
			sds.reserve(6);
			for (int i=0; i<6; i++)
			{
				sds.push_back(D3D11_SUBRESOURCE_DATA());
				D3D11_SUBRESOURCE_DATA& sd = sds.back();
				sd.pSysMem = ((DWORD*)data) + (i * width*height);
				sd.SysMemPitch = width * PixelFormat2Bytes(format);
				sd.SysMemSlicePitch = 0;
			}
		}
		else
		{
			sds.push_back(D3D11_SUBRESOURCE_DATA());
			D3D11_SUBRESOURCE_DATA& sd = sds.back();
			sd.pSysMem  = data;
			sd.SysMemPitch = width * PixelFormat2Bytes(format);
			sd.SysMemSlicePitch = 0;
		}
	}
	HRESULT hr;
	if (FAILED(hr = m_pDevice->CreateTexture2D( &desc, data ? &sds[0]: 0, &pTextureD3D11 )))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to CreateTexture from memory!");
		assert(0);
		return 0;
	}

	TextureD3D11* pTexture = TextureD3D11::CreateInstance();
	pTexture->SetHardwareTexture(pTextureD3D11);
	pTexture->SetSize(Vec2I(width, height));

	if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = ConvertEnumD3D11(format);
		if (type & TEXTURE_TYPE_DEPTH_STENCIL_SRV)
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		if (engineUsingMS && type & TEXTURE_TYPE_MULTISAMPLE)
		{
			srvDesc.ViewDimension = cubeMap ? D3D11_SRV_DIMENSION_TEXTURECUBE :
				D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			srvDesc.ViewDimension = cubeMap ? D3D11_SRV_DIMENSION_TEXTURECUBE :
				D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		

		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels=1;
		if (type & TEXTURE_TYPE_MIPS)
		{
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
		}		

		ID3D11ShaderResourceView* pResourceView;
		m_pDevice->CreateShaderResourceView(pTextureD3D11, &srvDesc, &pResourceView);
		pTexture->SetHardwareResourceView(pResourceView);
	}
	static size_t RTV_ID = 0;
	if ((type & TEXTURE_TYPE_RENDER_TARGET) ||
		(type & TEXTURE_TYPE_RENDER_TARGET_SRV))
	{
		int view = cubeMap ? 6 : 1;
		for (int i=0; i<view; i++)
		{
			// Create the render target view.
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = desc.Format;
			if (engineUsingMS && type & TEXTURE_TYPE_MULTISAMPLE)
			{
				renderTargetViewDesc.ViewDimension = cubeMap ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				renderTargetViewDesc.ViewDimension = cubeMap ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;
			}
			if (cubeMap)
			{
				renderTargetViewDesc.Texture2DArray.ArraySize = 1;
				renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
				renderTargetViewDesc.Texture2DArray.MipSlice = 0;
			}
			else
			{
				renderTargetViewDesc.Texture2D.MipSlice = 0;
			}

			ID3D11RenderTargetView* pRenderTargetView;		
			hr = m_pDevice->CreateRenderTargetView(pTextureD3D11, &renderTargetViewDesc, &pRenderTargetView);
			if(FAILED(hr))
			{
				Error("Cannot create RenderTargetView!");
			}
			else
			{
				char buf[256];
				sprintf_s(buf, "RenderTargetView(%u)", RTV_ID++);
				pRenderTargetView->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
			}
			pTexture->AddRenderTargetView(pRenderTargetView);
		}
	}
	else if ((type & TEXTURE_TYPE_DEPTH_STENCIL) ||
		(type & TEXTURE_TYPE_DEPTH_STENCIL_SRV)	)
	{
		int view = cubeMap ? 6 : 1;
		for (int i=0; i<view; i++)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC  dsvd = 
			{
				desc.Format,
				cubeMap ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D,
				0
			};
			if (cubeMap)
			{
				dsvd.Texture2DArray.ArraySize = 6;
				dsvd.Texture2DArray.FirstArraySlice = i;
				dsvd.Texture2DArray.MipSlice = 0;
			}
			if (type&TEXTURE_TYPE_DEPTH_STENCIL_SRV)
				dsvd.Format = DXGI_FORMAT_D32_FLOAT;
			ID3D11DepthStencilView* pDepthStencilView;
			hr = m_pDevice->CreateDepthStencilView(pTextureD3D11, &dsvd, &pDepthStencilView);
			if(FAILED(hr))
			{
				Error("Cannot create DepthStencilView!");
			}
			pTexture->AddDepthStencilView(pDepthStencilView);
		}
		
	}

	return pTexture;

}

//-----------------------------------------------------------------------------
void RendererD3D11::SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num, ITexture* pDepthStencil, size_t dsIndex)
{
	std::vector<ID3D11RenderTargetView*> rtviews;
	if (pRenderTargets)
	{
		for (int i=0; i<num; i++)
		{
			TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pRenderTargets[i]);
			rtviews.push_back(pTextureD3D11 ? pTextureD3D11->GetRenderTargetView(rtIndex[i]) : 0);
		}
	}
	else
	{
		rtviews.push_back(0);
	}
	ID3D11DepthStencilView* pDepthStencilView = 0;
	if (pDepthStencil)
	{
		TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pDepthStencil);
		pDepthStencilView = pTextureD3D11->GetDepthStencilView(dsIndex);
	}
	
	m_pImmediateContext->OMSetRenderTargets(rtviews.size(), &rtviews[0], pDepthStencilView);
	mCurrentRTViews = rtviews;
	mCurrentDSView = pDepthStencilView;
}

//----------------------------------------------------------------------------
void RendererD3D11::SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num)
{
	std::vector<ID3D11RenderTargetView*> rtviews;
	if (pRenderTargets)
	{
		for (int i = 0; i<num; i++)
		{
			TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pRenderTargets[i]);
			rtviews.push_back(pTextureD3D11 ? pTextureD3D11->GetRenderTargetView(rtIndex[i]) : 0);
		}
	}
	else
	{
		rtviews.push_back(0);
	}
	m_pImmediateContext->OMSetRenderTargets(rtviews.size(), &rtviews[0], mDepthStencilViews[0]);
	mCurrentRTViews = rtviews;
	mCurrentDSView = mDepthStencilViews[0];
}

//----------------------------------------------------------------------------
void RendererD3D11::SetGlowRenderTarget()
{
	__super::SetGlowRenderTarget();

	mRTViewsBeforeGlow = mCurrentRTViews;	
	TextureD3D11* rt = static_cast<TextureD3D11*>(mGlowTarget.get());
	if (mCurrentRTViews.size() < 2)
	{
		assert(mCurrentRTViews.size() == 1);
		mCurrentRTViews.push_back(rt->GetRenderTargetView(0));
	}
	else
	{
		mCurrentRTViews[1] = rt->GetRenderTargetView(0);
	}
	m_pImmediateContext->OMSetRenderTargets(mCurrentRTViews.size(), &mCurrentRTViews[0], mCurrentDSView);
}

//----------------------------------------------------------------------------
void RendererD3D11::UnSetGlowRenderTarget()
{
	mCurrentRTViews = mRTViewsBeforeGlow;
	m_pImmediateContext->OMSetRenderTargets(mCurrentRTViews.size(), &mCurrentRTViews[0], mCurrentDSView);
}

//----------------------------------------------------------------------------
void RendererD3D11::RestoreRenderTarget()
{
	m_pImmediateContext->OMSetRenderTargets(mRenderTargetViews.size(), &mRenderTargetViews[0], mDepthStencilViews[0]);
	mCurrentRTViews = mRenderTargetViews;
	mCurrentDSView = mDepthStencilViews[0];
}

//----------------------------------------------------------------------------
void RendererD3D11::OnReleaseRenderTarget(ID3D11RenderTargetView* pRTView)
{
	if (mCurrentRTViews.end() !=
		std::find(mCurrentRTViews.begin(), mCurrentRTViews.end(), pRTView))
	{
		RestoreRenderTarget();
	}
}

//----------------------------------------------------------------------------
void RendererD3D11::OnReleaseDepthStencil(ID3D11DepthStencilView* pDSView)
{
	if (mCurrentDSView == pDSView)
	{
		RestoreRenderTarget();
	}
}

void RendererD3D11::SetViewports(Viewport viewports[], int num)
{
	std::vector<D3D11_VIEWPORT> d3d11_viewports;
	for (int i=0; i<num; i++)
	{
		d3d11_viewports.push_back(ConvertStructD3D11(viewports[i]));
	}
	if (!d3d11_viewports.empty())
		m_pImmediateContext->RSSetViewports(d3d11_viewports.size(), &d3d11_viewports[0]);
	else
		RestoreViewports();

}

void RendererD3D11::RestoreViewports()
{
	m_pImmediateContext->RSSetViewports(mViewports.size(), &mViewports[0]);
}

void RendererD3D11::SetScissorRects(RECT rects[], int num)
{
	m_pImmediateContext->RSSetScissorRects(num, rects);
}

void RendererD3D11::RestoreScissorRects()
{
	RECT rect = {0, 0, 0, 0};
	m_pImmediateContext->RSSetScissorRects(1, &rect);
}

IInputLayout* RendererD3D11::GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
			IMaterial* material)
{
	IInputLayout* ret = __super::GetInputLayout(descs);
	if (ret)
		return ret;

	if (!material)
		return 0;

	unsigned int byteLength = 0;
	void* pShaderBytecodeWithInputSignature = material->GetShaderByteCode(byteLength);
	if (!pShaderBytecodeWithInputSignature)
		return 0;	

	ret = CreateInputLayout(descs, pShaderBytecodeWithInputSignature, byteLength);
	if (ret)
	{
		mInputLayouts[descs] = ret;
		return ret;
	}
	else
	{
		gFBEnv->pEngine->Error("Cannot create InputLayout!");
		return 0;
	}
	
}

IInputLayout* RendererD3D11::GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
			IShader* shader)
{
	IInputLayout* ret = __super::GetInputLayout(descs);
	if (ret)
		return ret;

	if (!shader)
		return 0;

	unsigned int byteLength = 0;
	void* pShaderBytecodeWithInputSignature = shader->GetVSByteCode(byteLength);
	assert(pShaderBytecodeWithInputSignature);
	if (!pShaderBytecodeWithInputSignature)
		return 0;	

	ret = CreateInputLayout(descs, pShaderBytecodeWithInputSignature, byteLength);
	if (ret)
	{
		mInputLayouts.insert(INPUTLAYOUT_MAP::value_type(descs, ret));
		return ret;
	}
	else
	{
		gFBEnv->pEngine->Error("Cannot create InputLayout!");
		return 0;
	}
}

IInputLayout* RendererD3D11::CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,
	void* byteCode, int byteLength)
{
	InputLayoutD3D11* pInputLayoutD3D11 = InputLayoutD3D11::CreateInstance();
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d11Descs;
	d3d11Descs.resize(descs.size());
	for (unsigned i=0; i<descs.size(); i++)
	{
		d3d11Descs[i] = ConvertStructD3D11(descs[i]);
	}
	ID3D11InputLayout* pHardwareInputLayout=0;
	
	HRESULT hr = m_pDevice->CreateInputLayout(&d3d11Descs[0], descs.size(), 
		byteCode, byteLength, &pHardwareInputLayout);
	if (FAILED(hr))
	{
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create input layout!");
	}
	else
	{
		static unsigned ID = 0;
		char buf[256];
		sprintf_s(buf, "InputLayout ID(%u)", ID++);
		pHardwareInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
	}
	pInputLayoutD3D11->SetHardwareInputLayout(pHardwareInputLayout);
	pInputLayoutD3D11->SetDescs(descs);

	return pInputLayoutD3D11;
}

//----------------------------------------------------------------------------
void RendererD3D11::SetShaders(IShader* pShader)
{
	if (!pShader->IsValid())
	{
		return;
	}
	if (mBindedShader == pShader)
		return;

	ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);	
	ID3D11VertexShader* pVS = pShaderD3D11->GetVertexShader();
	m_pImmediateContext->VSSetShader(pVS, 0, 0);
	ID3D11GeometryShader* pGS = pShaderD3D11->GetGeometryShader();
	m_pImmediateContext->GSSetShader(pGS, 0, 0);
	ID3D11HullShader* pHS = pShaderD3D11->GetHullShader();
	m_pImmediateContext->HSSetShader(pHS, 0, 0);
	ID3D11DomainShader* pDS = pShaderD3D11->GetDomainShader();
	m_pImmediateContext->DSSetShader(pDS, 0, 0);
	ID3D11PixelShader* pPS = pShaderD3D11->GetPixelShader();
	m_pImmediateContext->PSSetShader(pPS, 0, 0);
	mBindedShader = pShader;
}

void RendererD3D11::SetVSShader(IShader* pShader)
{
	if (pShader == 0)
	{
		m_pImmediateContext->VSSetShader(0, 0, 0);
		return;
	}

	if (!pShader->IsValid())
	{
		return;
	}

	ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
	ID3D11VertexShader* pVS = pShaderD3D11->GetVertexShader();
	m_pImmediateContext->VSSetShader(pVS, 0, 0);
	mBindedShader = 0;
}
void RendererD3D11::SetPSShader(IShader* pShader)
{
	if (pShader == 0)
	{
		m_pImmediateContext->PSSetShader(0, 0, 0);
		return;
	}

	if (!pShader->IsValid())
	{
		return;
	}

	ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
	ID3D11PixelShader* pPS = pShaderD3D11->GetPixelShader();
	m_pImmediateContext->PSSetShader(pPS, 0, 0);
	mBindedShader = 0;
}

void RendererD3D11::SetGSShader(IShader* pShader)
{
	if (pShader == 0)
	{
		m_pImmediateContext->GSSetShader(0, 0, 0);
		return;
	}

	if (!pShader->IsValid())
	{
		return;
	}

	ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
	ID3D11GeometryShader* pGS = pShaderD3D11->GetGeometryShader();
	m_pImmediateContext->GSSetShader(pGS, 0, 0);
	mBindedShader = 0;
}

void RendererD3D11::SetHSShader(IShader* pShader)
{
	if (pShader == 0)
	{
		m_pImmediateContext->HSSetShader(0, 0, 0);
		return;
	}

	if (!pShader->IsValid())
	{
		return;
	}

	ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
	ID3D11HullShader* pHS = pShaderD3D11->GetHullShader();
	m_pImmediateContext->HSSetShader(pHS, 0, 0);
	mBindedShader = 0;
}

void RendererD3D11::SetDSShader(IShader* pShader)
{
	if (pShader == 0)
	{
		m_pImmediateContext->DSSetShader(0, 0, 0);
		return;
	}

	if (!pShader->IsValid())
	{
		return;
	}

	ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
	ID3D11DomainShader* pDS = pShaderD3D11->GetDomainShader();
	m_pImmediateContext->DSSetShader(pDS, 0, 0);
	mBindedShader = 0;
}

//----------------------------------------------------------------------------
void RendererD3D11::SetInputLayout(IInputLayout* pInputLayout)
{
	if (pInputLayout == mBindedInputLayout)
		return;
	if (!pInputLayout)
	{
		m_pImmediateContext->IASetInputLayout(0);
		mBindedInputLayout = 0;
		return;
	}

	InputLayoutD3D11* pInputLayoutD3D11 = static_cast<InputLayoutD3D11*>(pInputLayout);
	ID3D11InputLayout* pLayout = pInputLayoutD3D11->GetHardwareInputLayout();
	assert(pLayout);
	m_pImmediateContext->IASetInputLayout(pLayout);
	mBindedInputLayout = pInputLayout;
}

//----------------------------------------------------------------------------
void RendererD3D11::SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt)
{
	if (pt == mCurrentTopology)
		return;
	D3D11_PRIMITIVE_TOPOLOGY d3d11PT;
	switch(pt)
	{
	case PRIMITIVE_TOPOLOGY_POINTLIST:
		d3d11PT = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case PRIMITIVE_TOPOLOGY_LINELIST:
		d3d11PT = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case PRIMITIVE_TOPOLOGY_LINESTRIP:
		d3d11PT = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		d3d11PT = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		d3d11PT = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	default:
		IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Undefined primitive topology!");
		assert(0);
		d3d11PT = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}

	m_pImmediateContext->IASetPrimitiveTopology(d3d11PT);
	mCurrentTopology = pt;
}

//----------------------------------------------------------------------------
void RendererD3D11::DrawIndexed(unsigned indexCount, 
	unsigned startIndexLocation, unsigned startVertexLocation)
{
	m_pImmediateContext->DrawIndexed(indexCount, startIndexLocation, startVertexLocation);

#ifdef _RENDERER_FRAME_PROFILER_
	mFrameProfiler.NumDrawIndexedCall++;
	mFrameProfiler.NumIndexCount += indexCount;
#endif
}

//----------------------------------------------------------------------------
void RendererD3D11::Draw(unsigned int vertexCount, unsigned int startVertexLocation)
{
	m_pImmediateContext->Draw(vertexCount, startVertexLocation);

#ifdef _RENDERER_FRAME_PROFILER_
	mFrameProfiler.NumDrawCall++;
	mFrameProfiler.NumVertexCount += vertexCount;
#endif
}

//----------------------------------------------------------------------------
void RendererD3D11::SetWireframe(bool enable)
{
	assert(m_pWireframeRasterizeState);
	if(mForcedWireframe != enable)
	{
		mForcedWireframe = enable;
		if (mForcedWireframe)
		{
			m_pImmediateContext->RSSetState(m_pWireframeRasterizeState);
		}
		else
		{
			m_pImmediateContext->RSSetState(0);
		}
	}
}

//----------------------------------------------------------------------------
MapData RendererD3D11::MapBuffer(ID3D11Resource* pResource, 
			UINT subResource, MAP_TYPE type, MAP_FLAG flag)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	D3D11_MAP maptype = ConvertEnumD3D11(type);
	unsigned int flagd3d11 = ConvertEnumD3D11(flag);
	HRESULT hr = m_pImmediateContext->Map(pResource, subResource, maptype,
		flagd3d11, &mappedSubresource);
	if (FAILED(hr))
	{
		assert(0);
		MapData data;
		data.pData = 0;
		return data;
	}

	MapData data;
	data.pData = mappedSubresource.pData;
	data.RowPitch = mappedSubresource.RowPitch;
	data.DepthPitch = mappedSubresource.DepthPitch;

	return data;
}

//----------------------------------------------------------------------------
MapData RendererD3D11::MapVertexBuffer(IVertexBuffer* pBuffer, UINT subResource, 
	MAP_TYPE type, MAP_FLAG flag)
{
	
	VertexBufferD3D11* pD3D11VB = dynamic_cast<VertexBufferD3D11*>(pBuffer);
	assert(pD3D11VB);
	return MapBuffer(pD3D11VB->GetHardwareBuffer(), subResource, type, flag);
}
void RendererD3D11::UnmapVertexBuffer(IVertexBuffer* pBuffer, unsigned int subResource)
{
	VertexBufferD3D11* pBufferD3D11 = dynamic_cast<VertexBufferD3D11*>(pBuffer);
	assert(pBufferD3D11);
	m_pImmediateContext->Unmap(pBufferD3D11->GetHardwareBuffer(), subResource);
}

//----------------------------------------------------------------------------
MapData RendererD3D11::MapTexture(ITexture* pTexture, UINT subResource, 
			MAP_TYPE type, MAP_FLAG flag)
{
	TextureD3D11* pTextureD3D11 = dynamic_cast<TextureD3D11*>(pTexture);
	assert(pTextureD3D11);
	return MapBuffer(pTextureD3D11->GetHardwareTexture(), subResource, type, flag);
}
void RendererD3D11::UnmapTexture(ITexture* pTexture, UINT subResource)
{
	TextureD3D11* pTextureD3D11 = dynamic_cast<TextureD3D11*>(pTexture);
	assert(pTextureD3D11);
	m_pImmediateContext->Unmap(pTextureD3D11->GetHardwareTexture(), subResource);
}

void RendererD3D11::CopyToStaging(ITexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
			ITexture* src, UINT srcSubresource, Box3D* pBox)
{
	assert(dst && src && dst!=src);
	TextureD3D11* pDstD3D11 = static_cast<TextureD3D11*>(dst);
	TextureD3D11* pSrcD3D11 = static_cast<TextureD3D11*>(src);
	m_pImmediateContext->CopySubresourceRegion(pDstD3D11->mTexture, dstSubresource,
		dstx, dsty, dstz, pSrcD3D11->mTexture, srcSubresource, (D3D11_BOX*)pBox);
}

//----------------------------------------------------------------------------
void RendererD3D11::SaveTextureToFile(ITexture* texture, const char* filename)
{
	TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(texture);
	if (pTextureD3D11)
	{
		const char* ext = GetExtension(filename);
		D3DX11_IMAGE_FILE_FORMAT format = D3DX11_IFF_FORCE_DWORD;
		if (stricmp(ext, "bmp")==0)
		{
			format = D3DX11_IFF_BMP;
		}
		else if (stricmp(ext, "jpg")==0)
		{
			format = D3DX11_IFF_JPG;
		}
		else if (stricmp(ext, "png")==0)
		{
			format = D3DX11_IFF_PNG;
		}
		else if (stricmp(ext, "dds")==0)
		{
			format = D3DX11_IFF_DDS;
		}
		else if (stricmp(ext, "tif")==0)
		{
			format = D3DX11_IFF_TIFF;
		}
		else if (stricmp(ext, "gif")==0)
		{
			format = D3DX11_IFF_GIF;
		}
		if (format == D3DX11_IFF_FORCE_DWORD)
		{
			Error("Unsupported file format!");
			return;
		}
		HRESULT hr = D3DX11SaveTextureToFile(m_pImmediateContext, pTextureD3D11->mTexture, format, filename);
		if (FAILED(hr))
		{
			Error("Save texture to file is failed.");
		}

	}
	
}

//----------------------------------------------------------------------------
IRasterizerState* RendererD3D11::CreateRasterizerState(const RASTERIZER_DESC& desc)
{
	size_t numRS = 0;
	RasterizerStateD3D11* pRS = 0;
	auto find = mRasterizerMap.find(desc);
	if (find == mRasterizerMap.end())
	{
		ID3D11RasterizerState* pRasterizerState = 0;
		D3D11_RASTERIZER_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = m_pDevice->CreateRasterizerState( &d3d11desc, &pRasterizerState );
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create an rasterizer state!");
			assert(0);
			return 0;
		}
		pRS = FB_NEW(RasterizerStateD3D11);
		mRasterizerMap[desc] = pRS;
		pRS->SetHardwareRasterizerState(pRasterizerState);		
		char buff[255];
		sprintf_s(buff, "RasterizerState(%u)", numRS++);
		FB_SET_DEVICE_DEBUG_NAME(pRS, buff);
	}
	else
	{
		pRS = find->second;
	}
	return pRS;
}

//----------------------------------------------------------------------------
IBlendState* RendererD3D11::CreateBlendState(const BLEND_DESC& desc)
{
	static size_t numBlendStates = 0;
	BlendStateD3D11* pBlendStateD3D11 = 0;

	auto find = mBlendMap.find(desc);
	if (find == mBlendMap.end())
	{
		ID3D11BlendState* pBlendState = 0;
		D3D11_BLEND_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = m_pDevice->CreateBlendState( &d3d11desc, &pBlendState );
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create a blend state!");
			assert(0);
			return 0;
		}
		pBlendStateD3D11 = FB_NEW(BlendStateD3D11);
		pBlendStateD3D11->SetHardwareBlendState(pBlendState);
		mBlendMap[desc] = pBlendStateD3D11;
		char buff[255];
		sprintf_s(buff, "BlendState(%u)", numBlendStates++);
		FB_SET_DEVICE_DEBUG_NAME(pBlendStateD3D11, buff);
		return pBlendStateD3D11;
	}
	else
	{
		pBlendStateD3D11 = find->second;
	}
	return pBlendStateD3D11;
}

//----------------------------------------------------------------------------
IDepthStencilState* RendererD3D11::CreateDepthStencilState( const DEPTH_STENCIL_DESC& desc )
{
	static size_t numDepthStencilStates = 0;
	DepthStencilStateD3D11* pDSSD3D11 = 0;
	auto find = mDepthStencilMap.find(desc);
	if (find == mDepthStencilMap.end())
	{
		ID3D11DepthStencilState* pHardwareState = 0;
		D3D11_DEPTH_STENCIL_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = m_pDevice->CreateDepthStencilState( &d3d11desc, &pHardwareState );
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create a depth stencil state!");
			assert(0);
			return 0;
		}
		pDSSD3D11 = FB_NEW(DepthStencilStateD3D11);
		mDepthStencilMap[desc] = pDSSD3D11;
		pDSSD3D11->SetHardwareDSState(pHardwareState);
		char buff[255];
		sprintf_s(buff, "DepthStencilState(%u)", numDepthStencilStates++);
		FB_SET_DEVICE_DEBUG_NAME(pDSSD3D11, buff);
	}
	else
	{
		pDSSD3D11 = find->second;
	}
	return pDSSD3D11;
}

//----------------------------------------------------------------------------
ISamplerState* RendererD3D11::CreateSamplerState(const SAMPLER_DESC& desc)
{
	static size_t numSamplerStates = 0;
	SamplerStateD3D11* pSSD3D11 = 0;
	auto find = mSamplerMap.find(desc);
	if (find == mSamplerMap.end())
	{
		ID3D11SamplerState* pSamplerState = 0;
		D3D11_SAMPLER_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = m_pDevice->CreateSamplerState(&d3d11desc, &pSamplerState);
		if (FAILED(hr))
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Failed to create sampler state!");
			assert(0);
		}
		pSSD3D11 = FB_NEW(SamplerStateD3D11);
		mSamplerMap[desc] = pSSD3D11;
		pSSD3D11->SetHardwareSamplerState(pSamplerState);
		char buff[255];
		sprintf_s(buff, "SamplerState(%u)", numSamplerStates++);
		FB_SET_DEVICE_DEBUG_NAME(pSSD3D11, buff);
	}
	else
	{
		pSSD3D11 = find->second;
	}
	return pSSD3D11;
}

//----------------------------------------------------------------------------
void RendererD3D11::SetRasterizerState(IRasterizerState* pRasterizerState)
{	
	if (!mForcedWireframe)
	{
		RasterizerStateD3D11* pRasterizerStateD3D11 = (RasterizerStateD3D11*)pRasterizerState;
		m_pImmediateContext->RSSetState(pRasterizerStateD3D11->GetHardwareRasterizerState());
	}
}

//----------------------------------------------------------------------------
void RendererD3D11::SetBlendState(IBlendState* pBlendState)
{
	if (mLockBlendState)
		return;
	BlendStateD3D11* pBlendStateD3D11 = (BlendStateD3D11*)pBlendState;
	m_pImmediateContext->OMSetBlendState(pBlendStateD3D11->GetHardwareBlendState(), 
		pBlendStateD3D11->GetBlendFactor(), pBlendStateD3D11->GetBlendMask());
}

//----------------------------------------------------------------------------
void RendererD3D11::SetDepthStencilState(IDepthStencilState* pDepthStencilState,
	unsigned stencilRef)
{
	if (mLockDepthStencil)
		return;
	DepthStencilStateD3D11* pDSSD11 = (DepthStencilStateD3D11*)pDepthStencilState;
	m_pImmediateContext->OMSetDepthStencilState(pDSSD11->GetHardwareDSState(),
		stencilRef);
}

//----------------------------------------------------------------------------
void RendererD3D11::SetSamplerState(ISamplerState* pSamplerState, BINDING_SHADER shader, int slot)
{
	SamplerStateD3D11* pSamplerStateD3D11 = (SamplerStateD3D11*)pSamplerState;
	ID3D11SamplerState* pSS = pSamplerStateD3D11->GetHardwareSamplerState();
	switch (shader)
	{
	case BINDING_SHADER_VS:
		m_pImmediateContext->VSSetSamplers(slot, 1, &pSS);
		break;
	case BINDING_SHADER_PS:
		m_pImmediateContext->PSSetSamplers(slot, 1, &pSS);
		break;
	default:
		assert(0);
	}
	
}

void RendererD3D11::DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color)
{
	static OBJECT_CONSTANTS constants = 
		{ 
			Mat44(2.f / mWidth, 0, 0, -1.f,
							0.f, -2.f / mHeight, 0, 1.f,
							0, 0, 1.f, 0.f,
							0, 0, 0, 1.f),
			Mat44()
		};

	// vertex buffer
	MapData mapped = mDynVBs[DEFAULT_INPUTS::POSITION_COLOR]->Map(
		MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
	DEFAULT_INPUTS::V_PC data[4] = {
		DEFAULT_INPUTS::V_PC(Vec3((float)pos.x,		(float)pos.y,		0.f), color.Get4Byte()),
		DEFAULT_INPUTS::V_PC(Vec3((float)pos.x+size.x,	(float)pos.y,		0.f), color.Get4Byte()),
		DEFAULT_INPUTS::V_PC(Vec3((float)pos.x,		(float)pos.y+size.y,0.f), color.Get4Byte()),
		DEFAULT_INPUTS::V_PC(Vec3((float)pos.x+size.x,	(float)pos.y+size.y,0.f), color.Get4Byte()),
	};
	if (mapped.pData)
	{
		memcpy(mapped.pData, data, sizeof(data));
		mDynVBs[DEFAULT_INPUTS::POSITION_COLOR]->Unmap();
	}
	

	UpdateObjectConstantsBuffer(&constants);

	// set primitive topology
	SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// set material
	mMaterials[DEFAULT_MATERIALS::QUAD]->Bind(true);
	// set vertex buffer
	mDynVBs[DEFAULT_INPUTS::POSITION_COLOR]->Bind();
	// draw
	Draw(4, 0);
}

void RendererD3D11::DrawFullscreenQuad(IShader* pixelShader, bool farside)
{
	// vertex buffer
	
	if (farside)
		mFullscreenQuadVSFar->BindVS();
	else
		mFullscreenQuadVSNear->BindVS();
	
	if (pixelShader)
		pixelShader->BindPS();

	SetInputLayout(0);
	SetGSShader(0);
	// draw
	// using full screen triangle : http://blog.naver.com/jungwan82/220108100698
	SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Draw(3, 0);
}

void RendererD3D11::DrawBillboardWorldQuad(const Vec3& pos, const Vec2& size, const Vec2& offset, 
	DWORD color, IMaterial* pMat)
{
	IVertexBuffer* pVB = mDynVBs[DEFAULT_INPUTS::POSITION_VEC4_COLOR];
	assert(pVB);
	MapData mapped = pVB->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
	DEFAULT_INPUTS::POSITION_VEC4_COLOR_V* data = (DEFAULT_INPUTS::POSITION_VEC4_COLOR_V*)mapped.pData;
	data->p = pos;
	data->v4 = Vec4(size.x, size.y, offset.x, offset.y);
	data->color = color;
	pVB->Unmap();
	pVB->Bind();
	SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
	pMat->Bind(true);
	Draw(1, 0);

	// vertexBuffer



}