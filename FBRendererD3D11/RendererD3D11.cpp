/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "RendererD3D11.h"
#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "ShaderD3D11.h"
#include "TextureD3D11.h"
#include "RenderStatesD3D11.h"
#include "InputLayoutD3D11.h"
#include "ConvertEnumD3D11.h"
#include "ConvertStructD3D11.h"
#include "D3D11Types.h"
#include "IUnknownDeleter.h"
#include "FBStringLib/StringLib.h"
#include "EssentialEngineData/shaders/CommonDefines.h"
#include "FBFileSystem/FileSystem.h"
#include "FBCommonHeaders/Helpers.h"
#include <set>

#define _RENDERER_FRAME_PROFILER_
using namespace fb;

static void Error(const char* szFmt, ...){
	char buf[2048];
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);
	Logger::Log(FB_ERROR_LOG_ARG, buf);
}

static void Log(const char* szFmt, ...){
	char buf[2048];
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);
	Logger::Log(FB_DEFAULT_LOG_ARG, buf);
}

//----------------------------------------------------------------------------
class RendererD3D11::Impl
{
public:
	HWindow INVALID_HWND = (HWindow)-1;

	IDXGIFactory1Ptr		mDXGIFactory;
	ID3D11DevicePtr			mDevice; // free-threaded
	ID3D11DeviceContextPtr	mImmediateContext; //  not free-threaded
	VectorMap<HWindowId, IDXGISwapChainPtr> mSwapChains;
	VectorMap<HWindowId, std::pair<TextureD3D11Ptr, TextureD3D11Ptr> > mRenderTargetTextures;
	DXGI_SAMPLE_DESC		mMultiSampleDesc;
	D3D_DRIVER_TYPE			mDriverType;
	D3D_FEATURE_LEVEL		mFeatureLevel;
	ID3DX11ThreadPumpPtr	mThreadPump;
	PIXEL_FORMAT			mDepthStencilFormat;

	// constant buffers
	ID3D11Buffer*			mShaderConstants[ShaderConstants::Num];	
	ID3D11RasterizerStatePtr mWireframeRasterizeState;
	std::vector<TextureD3D11Ptr>		 mCheckTextures;

	std::vector<DXGI_OUTPUT_DESC> mOutputInfos;
	VectorMap<HMONITOR, std::vector<DXGI_MODE_DESC>> mDisplayModes;

	std::vector<ID3D11RenderTargetView*> mCurrentRTViews;
	ID3D11DepthStencilView* mCurrentDSView;

	bool mStandBy;
	bool mUseShaderCache;
	bool mGenerateShaderCache;
	std::string mTakeScreenshot;
	

	//-------------------------------------------------------------------
	Impl()
		: mStandBy(false)
		, mUseShaderCache(true)
		, mGenerateShaderCache(true)
		, mDepthStencilFormat(PIXEL_FORMAT_D24_UNORM_S8_UINT)
	{
		IDXGIFactory1* dxgiFactory;
		HRESULT hr;
		try
		{
			hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
		}
		catch (...)
		{
		}
		if (FAILED(hr))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "CreateDXGIFactory1() failed!");
			return;
		}
		mDXGIFactory = IDXGIFactory1Ptr(dxgiFactory, IUnknownDeleter());

		UINT i = 0;
		IDXGIAdapter1* pAdapter;
		std::vector <IDXGIAdapter1Ptr> vAdapters;
		while (mDXGIFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(IDXGIAdapter1Ptr(pAdapter, IUnknownDeleter()));
			++i;
		}

		if (vAdapters.empty()){
			Error("No graphics adapter found!");
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

		ID3D11Device* device = 0;
		ID3D11DeviceContext* immediateContext = 0;
		for (int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			mDriverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(vAdapters[0].get(), mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &device, &mFeatureLevel, &immediateContext);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
		{
			Error("D3D11CreateDevice() failed!");
			return;
		}

		mDevice = ID3D11DevicePtr(device, IUnknownDeleter());
		mImmediateContext = ID3D11DeviceContextPtr(immediateContext, IUnknownDeleter());

		// Get OuputInformation
		GetOutputInformationFor(vAdapters[0].get());		

		//check multithreaded is supported by the hardware
		D3D11_FEATURE_DATA_THREADING dataThreading;
		if (SUCCEEDED(mDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &dataThreading, sizeof(dataThreading)))){
			if (dataThreading.DriverConcurrentCreates){
				Log("Hardware supports 'Concurrent Creates'");
			}
			else {
				Log("Hardware doen't support 'Concurrent Creates'");
			}

			if (dataThreading.DriverCommandLists){
				Log("Hardware supports multi threaded command lists.");
			}
			else{
				Log("Hardware doen't support 'Concurrent Creates'");
			}
		}
		else{
			Log("Hardware doen't support multithreading.");
		}


		unsigned msQuality = 0;
		unsigned msCount = 1;
		hr = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R32G32B32A32_FLOAT, msCount, &msQuality);
		if (SUCCEEDED(hr))
		{
			hr = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R32G32B32A32_FLOAT, msCount, &msQuality);
			if (SUCCEEDED(hr))
			{
				mMultiSampleDesc.Count = msCount;
				mMultiSampleDesc.Quality = std::min(msQuality - 1, (unsigned)4);
			}
		}
		unsigned shaderBufferSizes[] = {
			sizeof(FRAME_CONSTANTS),
			sizeof(OBJECT_CONSTANTS),
			sizeof(MATERIAL_CONSTANTS),
			sizeof(MATERIAL_PARAMETERS),
			sizeof(RARE_CONSTANTS),
			sizeof(BIG_BUFFER),
			sizeof(IMMUTABLE_CONSTANTS),
			sizeof(POINT_LIGHT_CONSTANTS),
			sizeof(CAMERA_CONSTANTS),
			sizeof(RENDERTARGET_CONSTANTS),
			sizeof(SCENE_CONSTANTS),
		};
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.MiscFlags = 0;		
		for (int i = 0; i < ShaderConstants::Num; ++i){
			Desc.ByteWidth = shaderBufferSizes[i];
			ID3D11Buffer* buffer = 0;
			hr = mDevice->CreateBuffer(&Desc, NULL, &buffer);
			if (FAILED(hr))
			{
				Error(FB_ERROR_LOG_ARG, "Failed to create constant buffer(FrameConstants)!");
			}
			else{
				mShaderConstants[i] = buffer;
			}
		}
		BindConstants();

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
		ID3D11RasterizerState* state;
		hr = mDevice->CreateRasterizerState(&RasterizerDesc, &state);
		if (FAILED(hr))
		{
			Log(FB_ERROR_LOG_ARG, "Failed to create an wireframe rasterizer state!");
		}
		else{
			mWireframeRasterizeState = ID3D11RasterizerStatePtr(state, IUnknownDeleter());
		}

		ID3DX11ThreadPump* pump;
		hr = D3DX11CreateThreadPump(1, 4, &pump);
		if (FAILED(hr))
		{
			Log(FB_ERROR_LOG_ARG, "Failed to create thread pump");
		}
		else{
			mThreadPump = ID3DX11ThreadPumpPtr(pump, IUnknownDeleter());
		}
	}

	~Impl(){
		for (int i = 0; i < ShaderConstants::Num; ++i){
			SAFE_RELEASE(mShaderConstants[i]);
		}
		mCurrentRTViews.clear();
		mCurrentDSView = 0;
		mCheckTextures.clear();
		mSwapChains.clear();
		mImmediateContext = 0;
		mDevice = 0;
		mDXGIFactory = 0;
	}

	// Device features
	Vec2ITuple FindClosestSize(HWindowId id, const Vec2ITuple& input){
		Vec2I closest = input;
		auto it = mSwapChains.Find(id);
		if (it == mSwapChains.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("RendererD3D11::FindClosestSize : swap chain %d is not found", id).c_str());
			return closest;
		}

		IDXGIOutput* output;
		if (FAILED(it->second->GetContainingOutput(&output))){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("RendererD3D11::FindClosestSize : failed to get containing ouput for swap chain %d", id).c_str());
			return closest;
		}
		auto outputPtr = IDXGIOutputPtr(output, IUnknownDeleter());
		UINT num = 0;
		if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, 0))){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("RendererD3D11::FindClosestSize : GetDisplayModeList #1 is failed for swap chain %d", id).c_str());			
			return closest;
		}

		Real shortestDist = FLT_MAX;
		auto descsPtr = std::shared_ptr<DXGI_MODE_DESC>(FB_ARRAY_NEW(DXGI_MODE_DESC, num), [](DXGI_MODE_DESC* obj){ FB_ARRAY_DELETE(obj); });
		DXGI_MODE_DESC* descs = descsPtr.get();
		if (SUCCEEDED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, descs))){
			for (UINT i = 0; i < num; ++i){
				auto curSize = Vec2I(descs[i].Width, descs[i].Height);
				Real dist = curSize.DistanceTo(input);
				if (dist < shortestDist){
					shortestDist = dist;
					closest = curSize;
				}

			}
		}
		return closest;
	}

	bool GetResolutionList(unsigned& outNum, Vec2ITuple* list){
		auto it = mSwapChains.Find(1);
		if (it == mSwapChains.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "RendererD3D11::GetResolutionList : swap chain 1 is not found");
			return false;
		}

		IDXGIOutput* output;
		if (FAILED(it->second->GetContainingOutput(&output))){
			Logger::Log(FB_ERROR_LOG_ARG, "RendererD3D11::GetResolutionList : failed to get containing ouput for swap chain 1");
			return false;
		}
		IDXGIOutputPtr outputPtr(output, IUnknownDeleter());
		UINT num = 0;
		if (list == 0){
			if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, 0))){
				Logger::Log(FB_ERROR_LOG_ARG, "RendererD3D11::GetResolutionList : GetDisplayModeList #1 is failed for swap chain 1");
				return false;
			}
			outNum = num;
			return true;
		}
		else{
			num = outNum;
			std::shared_ptr<DXGI_MODE_DESC> descsPtr(FB_ARRAY_NEW(DXGI_MODE_DESC, num), [](DXGI_MODE_DESC* obj){FB_ARRAY_DELETE(obj); });
			DXGI_MODE_DESC* descs = descsPtr.get();
			if (SUCCEEDED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, descs))){
				for (UINT i = 0; i < num; ++i){
					list[i] = Vec2ITuple{ descs[i].Width, descs[i].Height };
				}
			}
			else{
				return false;
			}
			return true;
		}
	}

	bool InitCanvas(HWindowId id, HWindow window, int width, int height, int fullscreen,
		IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture){				
		if (window == INVALID_HWND)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No valid window.");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = 0;

		DXGI_MODE_DESC findingMode;
		findingMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		findingMode.Width = width;
		findingMode.Height = height;
		findingMode.RefreshRate.Numerator = 60;
		findingMode.RefreshRate.Denominator = 1;
		findingMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		findingMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		DXGI_MODE_DESC bestMatch;
		if (id == 1){
			HMONITOR monitorHandle = MonitorFromWindow((HWND)window, MONITOR_DEFAULTTONEAREST);
			if (monitorHandle){
				bool found = FindClosestMatchingMode(&findingMode, &bestMatch, monitorHandle);
				if (found){
					sd.BufferDesc.Width = bestMatch.Width;
					sd.BufferDesc.Height = bestMatch.Height;
					sd.BufferDesc.Format = bestMatch.Format;
					sd.BufferDesc.RefreshRate.Numerator = bestMatch.RefreshRate.Numerator;
					sd.BufferDesc.RefreshRate.Denominator = bestMatch.RefreshRate.Denominator;
					sd.BufferDesc.Scaling = bestMatch.Scaling;
					sd.BufferDesc.ScanlineOrdering = bestMatch.ScanlineOrdering;
				}
			}
		}

		/*
		DXGI_SWAP_EFFECT_DISCARD
		Use this flag to indicate that the contents of the back buffer are
		discarded after calling IDXGISwapChain::Present. This flag is valid for
		a swap chain with more than one back buffer, although, an application only
		has read and write access to buffer 0. Use this flag to enable the display
		driver to select the most efficient presentation technique for the swap chain.
		*/
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = (HWND)window;
		sd.SampleDesc = mMultiSampleDesc;
		auto r_fullscreen = fullscreen;
		/*
		Since the target output cannot be chosen explicitly when the swap-chain is created,
		you should not create a full-screen swap chain. This can reduce presentation performance
		if the swap chain size and the output window size do not match.
		*/
		sd.Windowed = true;
		
		IDXGISwapChain* pSwapChain;
		HRESULT hr = mDXGIFactory->CreateSwapChain(mDevice.get(), &sd, &pSwapChain);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "CreateSwapChain failed!");
			assert(0);
			return false;
		}
		mSwapChains[id] = IDXGISwapChainPtr(pSwapChain, IUnknownDeleter());

		if (r_fullscreen == 2){
			// faked fullscreen
			IDXGIOutput* output;
			if (SUCCEEDED(pSwapChain->GetContainingOutput(&output))){
				DXGI_OUTPUT_DESC desc;
				if (SUCCEEDED(output->GetDesc(&desc))){
					ChangeWindowStyle((HWND)window, 0);
					ChangeWindowRect((HWND)window, desc.DesktopCoordinates);
				}
				SAFE_RELEASE(output);
			}
			return true;
		}
		else if (r_fullscreen == 1){
			if (SUCCEEDED(pSwapChain->SetFullscreenState(TRUE, NULL))){
				return true;
			}
		}

		TextureD3D11Ptr color, depth;
		auto successful = CreateTargetTexturesFor(pSwapChain, Vec2I(sd.BufferDesc.Width, sd.BufferDesc.Height), color, depth);
		if (!successful){
			Logger::Log(FB_ERROR_LOG_ARG, "Create swap-chain target texture is failed!");
			return false;
		}
		mRenderTargetTextures[id] = { color, depth };
		outColorTexture = color;
		outDepthTexture = depth;
		return true;

	}

	void DeinitCanvas(HWindowId id, HWindow window){
		auto it = mRenderTargetTextures.Find(id);
		if (it != mRenderTargetTextures.end()){
			mRenderTargetTextures.erase(it);
		}

		auto itSwapChain = mSwapChains.Find(id);
		if (itSwapChain != mSwapChains.end())
		{
			BOOL fullscreen = TRUE;
			itSwapChain->second->GetFullscreenState(&fullscreen, 0);			
			if (fullscreen){
				/*
				Destroying a Swap Chain
				You may not release a swap chain in full-screen mode because doing so
				may create thread contention (which will cause DXGI to raise a non-continuable
				exception). Before releasing a swap chain, first switch to windowed mode
				(using IDXGISwapChain::SetFullscreenState( FALSE, NULL )) and then call IUnknown::Release.
				*/
				itSwapChain->second->SetFullscreenState(FALSE, NULL);
			}
			mSwapChains.erase(itSwapChain);
		}
	}

	bool ChangeResolution(HWindowId id, HWindow window, const Vec2ITuple& newResol,
		IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture){
		auto swIt = mSwapChains.Find(id);
		if (swIt == mSwapChains.end())
			return false;
		Vec2I resol = newResol;
		Vec2I originalResol(0, 0);
		{
			auto rtIt = mRenderTargetTextures.Find(id);
			if (rtIt != mRenderTargetTextures.end()){
				originalResol = rtIt->second.first->GetSize();
				if (originalResol == resol)
					return true;
			}

		}
		BOOL fullscreen;
		IDXGIOutput* output = 0;
		swIt->second->GetFullscreenState(&fullscreen, &output);
		if (output){
			SAFE_RELEASE(output);
		}
		TextureD3D11Ptr color, depth;
		bool suc = ResizeSwapChain(id, resol, color, depth);
		outColorTexture = color;
		outDepthTexture = depth;
		if (!suc){
			Error("ResizeSwapChain failed!");
			return false;			
		}
		else{
			return true;
		}
	}

	bool ChangeFullscreenMode(HWindowId id, HWindow window, int mode){
		auto it = mSwapChains.Find(id);
		if (it == mSwapChains.end())
			return false;

		auto swapChain = it->second.get();
		if (mode == 0){
			swapChain->SetFullscreenState(FALSE, NULL);
			ChangeWindowStyle((HWND)window, WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX);			
		}
		else if (mode == 1){
			swapChain->SetFullscreenState(TRUE, NULL);
		}
		else if (mode == 2){
			swapChain->SetFullscreenState(FALSE, NULL);
			// faked fullscreen
			IDXGIOutput* output;
			if (SUCCEEDED(swapChain->GetContainingOutput(&output))){
				DXGI_OUTPUT_DESC desc;
				if (SUCCEEDED(output->GetDesc(&desc))){
					ChangeWindowStyle((HWND)window, 0);
					ChangeWindowRect((HWND)window, desc.DesktopCoordinates);
				}
				SAFE_RELEASE(output);
			}

		}
		return true;
	}

	unsigned GetMultiSampleCount() const{
		return 1;
	}

	// Resource creation
	void SetShaderCacheOption(bool useShaderCache, bool generateCache){
		mUseShaderCache = useShaderCache;
		mGenerateShaderCache = generateCache;
	}

	IPlatformTexturePtr CreateTexture(const char* path, bool async){
		if (!ValidCStringLength(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}
		std::string filepath(path);
		ToLowerCase(filepath);
		if (!FileSystem::Exists(path))
		{
			Error("File not found while loading a texture! %s", path);
			return 0;
		}

		ID3D11ShaderResourceView* pSRView = 0;
		D3DX11_IMAGE_INFO imageInfo;
		HRESULT hr = D3DX11GetImageInfoFromFile(path, 0, &imageInfo, 0);
		if (FAILED(hr))
		{
			Error("[Error] Failed to get image info (%s)", path);
		}

		auto texture = TextureD3D11::Create();
		texture->SetPath(path);
		if (imageInfo.Format == DXGI_FORMAT_R8G8B8A8_UNORM || imageInfo.Format == DXGI_FORMAT_BC1_UNORM ||
			imageInfo.Format == DXGI_FORMAT_BC3_UNORM || imageInfo.Format == DXGI_FORMAT_BC5_UNORM)
		{
			texture->SetLoadInfoTextureFormat((DXGI_FORMAT)(imageInfo.Format + 1));
		}

		if (mThreadPump && async)
		{
			hr = D3DX11CreateShaderResourceViewFromFile(mDevice.get(), path,
				texture->GetLoadInfoPtr(), mThreadPump.get(),
				texture->GetSRViewSyncPtr(), texture->GetHRPtr());
			mCheckTextures.push_back(texture);
		}
		else
		{
			hr = D3DX11CreateShaderResourceViewFromFile(mDevice.get(), path,
				texture->GetLoadInfoPtr(), nullptr,
				texture->GetSRViewSyncPtr(), nullptr);

			unsigned tid = texture->GetTextureId();
			char buf[256];
			sprintf_s(buf, "Texture(%u)", tid);
			texture->SetDebugName(buf);
		}
		texture->SetSize(Vec2I(imageInfo.Width, imageInfo.Height));

		if (FAILED(hr))
		{
			Error("[Error] Failed to load a texture (%s).", path);
		}

		return texture;
	}

	IPlatformTexturePtr CreateTexture(void* data, int width, int height,
		PIXEL_FORMAT format, BUFFER_USAGE usage, int  buffer_cpu_access,
		int type) {
		if (width == 0 || height == 0)
		{
			Error(FB_ERROR_LOG_ARG, "width and height cannot be 0.");
			return 0;
		}
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
			desc.MipLevels = 0;// GetMipLevels((float)std::min(width, height));
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
		if (usage != BUFFER_USAGE_STAGING &&
			!(type & TEXTURE_TYPE_RENDER_TARGET) && !(type & TEXTURE_TYPE_DEPTH_STENCIL))
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if ((type & TEXTURE_TYPE_RENDER_TARGET) || (type & TEXTURE_TYPE_RENDER_TARGET_SRV))
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		else if ((type & TEXTURE_TYPE_DEPTH_STENCIL) || (type & TEXTURE_TYPE_DEPTH_STENCIL_SRV))
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		if (buffer_cpu_access & BUFFER_CPU_ACCESS_READ)
			desc.CPUAccessFlags = ConvertEnumD3D11(BUFFER_CPU_ACCESS_READ);
		if (buffer_cpu_access & BUFFER_CPU_ACCESS_WRITE)
			desc.CPUAccessFlags |= ConvertEnumD3D11(BUFFER_CPU_ACCESS_WRITE);
		desc.MiscFlags = cubeMap ?
		D3D11_RESOURCE_MISC_TEXTURECUBE :
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
				for (int i = 0; i<6; i++)
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
				sd.pSysMem = data;
				sd.SysMemPitch = width * PixelFormat2Bytes(format);
				sd.SysMemSlicePitch = 0;
			}
		}
		HRESULT hr;
		if (FAILED(hr = mDevice->CreateTexture2D(&desc, data ? &sds[0] : 0, &pTextureD3D11)))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to CreateTexture from memory!");			
			return 0;
		}

		auto texture = TextureD3D11::Create();
		texture->SetHardwareTexture(pTextureD3D11);
		texture->SetSize(Vec2I(width, height));

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

			D3D11_TEXTURE2D_DESC tempDest;
			pTextureD3D11->GetDesc(&tempDest);
			srvDesc.Texture2D.MipLevels = tempDest.MipLevels;

			ID3D11ShaderResourceView* pResourceView;
			mDevice->CreateShaderResourceView(pTextureD3D11, &srvDesc, &pResourceView);
			texture->SetHardwareResourceView(pResourceView);
		}
		static size_t RTV_ID = 0;
		if ((type & TEXTURE_TYPE_RENDER_TARGET) ||
			(type & TEXTURE_TYPE_RENDER_TARGET_SRV))
		{
			int view = cubeMap ? 6 : 1;
			for (int i = 0; i<view; i++)
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

				ID3D11RenderTargetView* pRenderTargetView = 0;
				hr = mDevice->CreateRenderTargetView(pTextureD3D11, &renderTargetViewDesc, &pRenderTargetView);
				if (FAILED(hr))
				{
					Error("Cannot create RenderTargetView!");
				}
				else
				{
					char buf[256];
					sprintf_s(buf, "RenderTargetView(%u)", RTV_ID++);
					pRenderTargetView->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
				}
				texture->AddRenderTargetView(pRenderTargetView);
			}
		}
		else if ((type & TEXTURE_TYPE_DEPTH_STENCIL) ||
			(type & TEXTURE_TYPE_DEPTH_STENCIL_SRV))
		{
			int view = cubeMap ? 6 : 1;
			for (int i = 0; i<view; i++)
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
				ID3D11DepthStencilView* pDepthStencilView = 0;
				hr = mDevice->CreateDepthStencilView(pTextureD3D11, &dsvd, &pDepthStencilView);
				if (FAILED(hr))
				{
					Error("Cannot create DepthStencilView!");
				}
				texture->AddDepthStencilView(pDepthStencilView);
			}
		}

		return texture;
	}

	IPlatformVertexBufferPtr CreateVertexBuffer(void* data, unsigned stride,
		unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) {
		if (usage == BUFFER_USAGE_IMMUTABLE && data == 0)
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create vertex buffer! Immutable needs data pointer.");
			return 0;
		}

		
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
		HRESULT hr = mDevice->CreateBuffer(&bufferDesc, data ? &initData : 0, &pHardwareBuffer);

		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create a vertex buffer!");
		}
		else
		{
			static unsigned ID = 0;
			char buf[256];
			sprintf_s(buf, "VertexBuffer ID(%d)", ID++);
			pHardwareBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
		auto pVertexBufferD3D11 = VertexBufferD3D11::Create(pHardwareBuffer, stride);
		return pVertexBufferD3D11;
	}

	IPlatformIndexBufferPtr CreateIndexBuffer(void* data, unsigned int numIndices,
		INDEXBUFFER_FORMAT format) {
		auto pIndexBufferD3D11 = IndexBufferD3D11::Create();	

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = numIndices * (format == INDEXBUFFER_FORMAT_16BIT ? 2 : 4);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		ID3D11Buffer* pHardwareBuffer = 0;
		HRESULT hr = mDevice->CreateBuffer(&bufferDesc, &initData, &pHardwareBuffer);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create a index buffer!");
			return 0;
		}

		pIndexBufferD3D11->SetHardwareBuffer(pHardwareBuffer);
		return pIndexBufferD3D11;
		
	}

	class IncludeProcessor : public ID3DInclude
	{
	public:
		IncludeProcessor(const char* filename, ShaderD3D11Ptr pshader)
			: mShader(pshader)
		{
			assert(mShader);
			mWorkingDirectory = FileSystem::GetParentPath(filename) + "/";
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
				const char* paths[] = { "EssentialEnginedata/shaders/"
				};
				for (int i = 0; i < ARRAYCOUNT(paths); i++)
				{
					std::string filepath = paths[i];
					filepath += pFileName;
					fopen_s(&file, filepath.c_str(), "rb");
					if (file)
					{
						break;
					}
				}
				if (!file)
				{
					Error("Failed to open include file %s", pFileName);
					return S_OK;
				}

			}

			ToLowerCase(filepath);
			mProcessedFiles.insert(filepath);

			fseek(file, 0, SEEK_END);
			long size = ftell(file);
			rewind(file);
			char* buffer = FB_ARRAY_NEW(char, size);
			int elements = fread(buffer, 1, size, file);
			assert(elements == size);
			*ppData = buffer;
			*pBytes = size;
			fclose(file);
			return S_OK;
		}

		virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Close(LPCVOID pData)
		{
			FB_ARRAY_DELETE((char*)pData);
			return S_OK;
		}

	private:
		std::string mWorkingDirectory;
		std::set<std::string> mProcessedFiles;
		ShaderD3D11Ptr mShader;

	};

	HRESULT CompileShaderFromFile(const char* filename, const char* entryPoint,
		const char* shaderModel, ID3DBlob** ppBlobOut, D3D_SHADER_MACRO* pDefines = 0, IncludeProcessor* includeProcessor = 0)
	{
		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* pErrorBlob = 0;
		hr = D3DX11CompileFromFile(filename, pDefines, includeProcessor, entryPoint, shaderModel, dwShaderFlags, 0,
			0, ppBlobOut, &pErrorBlob, 0);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				Error("[Error] CompileShaderFromFile %s failed!", filename);
				Error((const char*)pErrorBlob->GetBufferPointer());
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

	IPlatformShaderPtr CreateShader(const char* path, int shaders,
		const SHADER_DEFINES& defines) {		
		std::string filepath(path);
		ToLowerCase(filepath);

		// build cache key
		std::string cachekey = filepath;
		if (!defines.empty())
		{
			std::string namevalue;
			for (const auto& define : defines)
			{
				namevalue += define.name;
				namevalue += define.value;
			}
			uLong crc = crc32(0L, Z_NULL, 0);
			crc = crc32(crc, (const Bytef*)namevalue.c_str(), namevalue.size());
			char buf[255];
			sprintf_s(buf, "%u", crc);
			cachekey += buf;
		}

		auto pShader = ShaderD3D11::Create();
		auto onlyname = FileSystem::GetName(filepath.c_str());
		std::vector<D3D_SHADER_MACRO> shaderMacros;
		for (DWORD i = 0; i<defines.size(); i++)
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
		BinaryData dataHolder;
		// Load VS
		if (shaders & BINDING_SHADER_VS)
		{
			std::string VSName = onlyname;
			VSName += "_VertexShader";
			ID3DBlob* pVSBlob = 0;
			// check vs cache
			std::string vs_cachekey = cachekey + ".vscache";
			void* shaderByteCode = 0;
			bool usingCache = false;
			std::streamoff length = 0;
			// use cache
			if (mUseShaderCache && FileSystem::CompareFileModifiedTime(filepath.c_str(), vs_cachekey.c_str()) == -1)
			{
				usingCache = true;
				dataHolder = FileSystem::ReadBinaryFile(vs_cachekey.c_str(), length);
				shaderByteCode = dataHolder.get();
			}
			// don't use cache, and make the cache.
			else
			{
				hr = CompileShaderFromFile(filepath.c_str(), VSName.c_str(), "vs_5_0", &pVSBlob,
					pShaderMacros, &includeProcessor);
				if (FAILED(hr))
				{
					SAFE_RELEASE(pVSBlob);
					pShader->SetCompileFailed(true);
					return pShader;
				}
				if (mGenerateShaderCache)
					FileSystem::WriteBinaryFile(vs_cachekey.c_str(), (char*)pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize());
				shaderByteCode = pVSBlob->GetBufferPointer();
				length = pVSBlob->GetBufferSize();
			}
			// Create VS
			ID3D11VertexShader* pVertexShader = 0;
			hr = mDevice->CreateVertexShader(shaderByteCode, (size_t)length, 0, &pVertexShader);
			if (FAILED(hr))
			{
				if (pVSBlob)
				{
					Logger::Log(FB_ERROR_LOG_ARG, "pVSBlob->GetBufferPointer()");
					SAFE_RELEASE(pVSBlob);
				}
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
			if (pVSBlob)
			{
				pShader->SetVertexShaderBytecode(pVSBlob); // pVSBlob will be released here
				SAFE_RELEASE(pVSBlob);
			}
			else if (shaderByteCode)
			{
				pShader->SetVertexShaderBytecode(shaderByteCode, (size_t)length);
			}
			else
				assert(0);
		}

		// Load GS
		if (shaders & BINDING_SHADER_GS)
		{
			std::string GSName = onlyname;
			GSName += "_GeometryShader";
			ID3DBlob* pGSBlob = 0;
			// check vs cache
			std::string gs_cachekey = cachekey + ".gscache";
			void* shaderByteCode = 0;
			bool usingCache = false;
			std::streamoff length = 0;
			// use cache
			if (mUseShaderCache && FileSystem::CompareFileModifiedTime(filepath.c_str(), gs_cachekey.c_str()) == -1)
			{
				usingCache = true;
				dataHolder = FileSystem::ReadBinaryFile(gs_cachekey.c_str(), length);
				shaderByteCode = dataHolder.get();
			}
			// don't use cache, and make the cache.
			else
			{

				HRESULT hr = CompileShaderFromFile(filepath.c_str(), GSName.c_str(), "gs_5_0", &pGSBlob,
					pShaderMacros, &includeProcessor);
				if (FAILED(hr))
				{
					SAFE_RELEASE(pGSBlob);
					pShader->SetCompileFailed(true);
					return pShader;
				}
				if (mGenerateShaderCache)
					FileSystem::WriteBinaryFile(gs_cachekey.c_str(), (char*)pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize());
				shaderByteCode = pGSBlob->GetBufferPointer();
				length = pGSBlob->GetBufferSize();
			}
			// Create GS
			ID3D11GeometryShader* pGeometryShader = 0;
			hr = mDevice->CreateGeometryShader(shaderByteCode, (size_t)length, 0, &pGeometryShader);
			if (FAILED(hr))
			{
				if (pGSBlob)
				{
					Error(FB_ERROR_LOG_ARG, pGSBlob->GetBufferPointer());
					SAFE_RELEASE(pGSBlob);
				}
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
			PSName += "_PixelShader";
			ID3DBlob* pPSBlob = 0;
			// check vs cache
			std::string ps_cachekey = cachekey + ".pscache";
			void* shaderByteCode = 0;
			bool usingCache = false;
			std::streamoff length = 0;
			// use cache
			if (mUseShaderCache && FileSystem::CompareFileModifiedTime(filepath.c_str(), ps_cachekey.c_str()) == -1)
			{
				usingCache = true;
				dataHolder = FileSystem::ReadBinaryFile(ps_cachekey.c_str(), length);
				shaderByteCode = dataHolder.get();
			}
			// don't use cache, and make the cache.
			else
			{
				hr = CompileShaderFromFile(filepath.c_str(), PSName.c_str(), "ps_5_0", &pPSBlob,
					pShaderMacros, &includeProcessor);
				if (FAILED(hr))
				{
					SAFE_RELEASE(pPSBlob);
					pShader->SetCompileFailed(true);
					return pShader;
				}
				if (mGenerateShaderCache)
					FileSystem::WriteBinaryFile(ps_cachekey.c_str(), (char*)pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize());
				shaderByteCode = pPSBlob->GetBufferPointer();
				length = pPSBlob->GetBufferSize();
			}
			// Create PS
			ID3D11PixelShader* pPixelShader = 0;
			hr = mDevice->CreatePixelShader(shaderByteCode, (size_t)length, 0, &pPixelShader);
			if (FAILED(hr))
			{
				if (pPSBlob)
				{
					Error(FB_ERROR_LOG_ARG, pPSBlob->GetBufferPointer());
					SAFE_RELEASE(pPSBlob);
				}
				pShader->SetCompileFailed(true);
				return pShader;
			}
			pShader->SetPixelShader(pPixelShader);
			SAFE_RELEASE(pPSBlob);
		}
		pShader->SetCompileFailed(false);

		return pShader;
	}

	
	IPlatformInputLayoutPtr CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,
		void* shaderByteCode, unsigned size) {
		if (descs.empty())
			return 0;

		auto pInputLayoutD3D11 = InputLayoutD3D11::Create();
		std::vector<D3D11_INPUT_ELEMENT_DESC> d3d11Descs;
		d3d11Descs.resize(descs.size());
		for (unsigned i = 0; i<descs.size(); i++)
		{
			d3d11Descs[i] = ConvertStructD3D11(descs[i]);
		}
		ID3D11InputLayout* pHardwareInputLayout = 0;

		HRESULT hr = mDevice->CreateInputLayout(&d3d11Descs[0], descs.size(),
			shaderByteCode, size, &pHardwareInputLayout);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create input layout!");
		}
		else
		{
			static unsigned ID = 0;
			char buf[256];
			sprintf_s(buf, "InputLayout ID(%u)", ID++);
			pHardwareInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		}
		pInputLayoutD3D11->SetHardwareInputLayout(pHardwareInputLayout);
		return pInputLayoutD3D11;
	}

	IPlatformBlendStatePtr CreateBlendState(const BLEND_DESC& desc) {
		static size_t numBlendStates = 0;
		ID3D11BlendState* pBlendState = 0;
		D3D11_BLEND_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = mDevice->CreateBlendState(&d3d11desc, &pBlendState);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create a blend state!");			
			return 0;
		}
		auto pBlendStateD3D11 = BlendStateD3D11::Create(pBlendState);				
		char buff[255];
		sprintf_s(buff, "BlendState(%u)", numBlendStates++);
		pBlendStateD3D11->SetDebugName(buff);
		return pBlendStateD3D11;
	}

	IPlatformDepthStencilStatePtr CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc) {
		static size_t numDepthStencilStates = 0;
		ID3D11DepthStencilState* pHardwareState = 0;
		D3D11_DEPTH_STENCIL_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = mDevice->CreateDepthStencilState(&d3d11desc, &pHardwareState);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create a depth stencil state!");
			return 0;
		}
		auto pDSSD3D11 = DepthStencilStateD3D11::Create(pHardwareState);				
		char buff[255];
		sprintf_s(buff, "DepthStencilState(%u)", numDepthStencilStates++);
		pDSSD3D11->SetDebugName(buff);		
		
		return pDSSD3D11;
	}

	IPlatformRasterizerStatePtr CreateRasterizerState(const RASTERIZER_DESC& desc) {
		static size_t numRS = 0;
		ID3D11RasterizerState* pRasterizerState = 0;
		D3D11_RASTERIZER_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = mDevice->CreateRasterizerState(&d3d11desc, &pRasterizerState);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create an rasterizer state!");
			return 0;
		}
		auto pRS = RasterizerStateD3D11::Create(pRasterizerState);
		char buff[255];
		sprintf_s(buff, "RasterizerState(%u)", numRS++);
		pRS->SetDebugName(buff);		
		return pRS;
	}

	IPlatformSamplerStatePtr CreateSamplerState(const SAMPLER_DESC& desc) {
		static size_t numSamplerStates = 0;		
		ID3D11SamplerState* pSamplerState = 0;
		D3D11_SAMPLER_DESC d3d11desc;
		ConvertStructD3D11(d3d11desc, desc);
		HRESULT hr = mDevice->CreateSamplerState(&d3d11desc, &pSamplerState);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create sampler state!");
			assert(0);
		}
		auto pSSD3D11 = SamplerStateD3D11::Create(pSamplerState);
		char buff[255];
		sprintf_s(buff, "SamplerState(%u)", numSamplerStates++);
		pSSD3D11->SetDebugName(buff);
		
		return pSSD3D11;
	}

	unsigned GetNumLoadingTexture() const {
		return mCheckTextures.size();
	}

	// Resource Binding
	void SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
		IPlatformTexturePtr pDepthStencil, size_t dsViewIndex){		
		mCurrentRTViews.clear();
		if (pRenderTargets)
		{
			for (int i = 0; i < num; i++)
			{
				TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pRenderTargets[i].get());
				auto renderTargetView = pTextureD3D11 ? pTextureD3D11->GetRenderTargetView(rtViewIndex[i]) : 0;
				mCurrentRTViews.push_back(renderTargetView);				
			}
		}
		else
		{
			mCurrentRTViews.push_back(0);			
		}

		mCurrentDSView = 0;
		if (pDepthStencil)
		{
			TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pDepthStencil.get());
			mCurrentDSView = pTextureD3D11->GetDepthStencilView(dsViewIndex);			
		}

		try
		{
			mImmediateContext->OMSetRenderTargets(mCurrentRTViews.size(), &mCurrentRTViews[0], mCurrentDSView);
		}
		catch (...)
		{
			Error(FB_ERROR_LOG_ARG, "OMSetRenderTargets failed!");
		}
	}

	void SetViewports(const Viewport viewports[], int num){
		assert(num > 0);
		std::vector<D3D11_VIEWPORT> d3d11_viewports;
		for (int i = 0; i<num; i++)
		{
			d3d11_viewports.push_back(ConvertStructD3D11(viewports[i]));
		}
		if (!d3d11_viewports.empty())
			mImmediateContext->RSSetViewports(d3d11_viewports.size(), &d3d11_viewports[0]);
	}

	void SetScissorRects(const Rect rects[], int num){
		mImmediateContext->RSSetScissorRects(num, (D3D11_RECT*)rects);
	}

	void SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,
		IPlatformVertexBuffer const * pVertexBuffers[], unsigned int const strides[], unsigned int offsets[]){
		if (numBuffers == 0 && pVertexBuffers == 0)
		{
			ID3D11Buffer* pHardwareBuffers[1] = { 0 };
			unsigned strides[1] = { 0 };
			unsigned offsets[1] = { 0 };
			mImmediateContext->IASetVertexBuffers(0, 1, pHardwareBuffers, strides, offsets);
			return;
		}
		ID3D11Buffer* pHardwareBuffers[32] = { 0 };
		for (int i = 0; i<(int)numBuffers; i++)
		{
			if (pVertexBuffers[i])
			{
				ID3D11Buffer* pBuffer = static_cast<VertexBufferD3D11 const *>(pVertexBuffers[i])->GetHardwareBuffer();
				pHardwareBuffers[i] = pBuffer;
			}
		}
		mImmediateContext->IASetVertexBuffers(startSlot, numBuffers, pHardwareBuffers, strides, offsets);
	}

	void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt){
		D3D11_PRIMITIVE_TOPOLOGY d3d11PT = ConvertEnumD3D11(pt);
		mImmediateContext->IASetPrimitiveTopology(d3d11PT);
	}

	void SetTextures(IPlatformTexturePtr pTextures[], int num, BINDING_SHADER shaderType, int startSlot){
		std::vector<ID3D11ShaderResourceView*> rvs;
		rvs.reserve(num);
		for (int i = 0; i < num; i++)
		{
			TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pTextures[i].get());
			if (pTextureD3D11)
			{
				rvs.push_back(pTextureD3D11->GetHardwareResourceView());
			}
			else
			{
				rvs.push_back(0);
			}
		}

		try
		{
			switch (shaderType)
			{
			case BINDING_SHADER_VS:
				mImmediateContext->VSSetShaderResources(startSlot, num, &rvs[0]);
				break;
			case BINDING_SHADER_PS:
				mImmediateContext->PSSetShaderResources(startSlot, num, &rvs[0]);
				break;
			default:
				assert(0);
				break;
			}
		}
		catch (...)
		{
		}
	}

	void BindConstants(){
		mImmediateContext->VSSetConstantBuffers(0, 11, mShaderConstants);
		mImmediateContext->GSSetConstantBuffers(0, 11, mShaderConstants);
		mImmediateContext->PSSetConstantBuffers(0, 11, mShaderConstants);
	}

	// Data
	void UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size){
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		auto buffer = mShaderConstants[type];
		mImmediateContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (mappedResource.pData){
			memcpy(mappedResource.pData, data, size);
			mImmediateContext->Unmap(buffer, 0);
		}
	}

	void* MapMaterialParameterBuffer() const{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mImmediateContext->Map(mShaderConstants[ShaderConstants::MaterialParam], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		return mappedResource.pData;
	}

	void UnmapMaterialParameterBuffer() const{
		mImmediateContext->Unmap(mShaderConstants[ShaderConstants::MaterialParam], 0);
	}

	void* MapBigBuffer() const{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mImmediateContext->Map(mShaderConstants[ShaderConstants::BigData], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		return mappedResource.pData;
	}

	void UnmapBigBuffer() const{
		mImmediateContext->Unmap(mShaderConstants[ShaderConstants::BigData], 0);
	}

	void UnbindInputLayout(){
		mImmediateContext->IASetInputLayout(0);
	}

	void UnbindShader(BINDING_SHADER shader){
		switch (shader){
		case BINDING_SHADER_VS:
			mImmediateContext->VSSetShader(0, 0, 0);
			break;
		case BINDING_SHADER_HS:
			mImmediateContext->HSSetShader(0, 0, 0);
			break;
		case BINDING_SHADER_DS:
			mImmediateContext->DSSetShader(0, 0, 0);
			break;
		case BINDING_SHADER_GS:
			mImmediateContext->GSSetShader(0, 0, 0);
			break;
		case BINDING_SHADER_PS:
			mImmediateContext->PSSetShader(0, 0, 0);
			break;
		}
	}

	void UnbindTexture(BINDING_SHADER shader, int slot){
		ID3D11ShaderResourceView* srv = 0;
		switch (shader){
		case BINDING_SHADER_VS:
			mImmediateContext->VSSetShaderResources(slot, 1, &srv);
			break;
		case BINDING_SHADER_HS:
			mImmediateContext->HSSetShaderResources(slot, 1, &srv);
			break;
		case BINDING_SHADER_DS:
			mImmediateContext->DSSetShaderResources(slot, 1, &srv);
			break;
		case BINDING_SHADER_GS:
			mImmediateContext->GSSetShaderResources(slot, 1, &srv);
			break;
		case BINDING_SHADER_PS:
			mImmediateContext->PSSetShaderResources(slot, 1, &srv);
			break;
		}
	}

	void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
		IPlatformTexture* src, UINT srcSubresource, Box3D* pBox){
		assert(dst && src && dst != src);
		TextureD3D11* pDstD3D11 = static_cast<TextureD3D11*>(dst);
		TextureD3D11* pSrcD3D11 = static_cast<TextureD3D11*>(src);
		auto hardwareTextureDest = pDstD3D11->GetHardwareTexture();
		auto hardwareTextureSrc = pSrcD3D11->GetHardwareTexture();
		if (hardwareTextureDest && hardwareTextureSrc){
			mImmediateContext->CopySubresourceRegion(hardwareTextureDest, dstSubresource,
				dstx, dsty, dstz, hardwareTextureSrc, srcSubresource, (D3D11_BOX*)pBox);
		}
		else{
			Error(FB_ERROR_LOG_ARG, "No hardware texture found.");
		}
	}

	// Drawing
	void Draw(unsigned int vertexCount, unsigned int startVertexLocation) {
		mImmediateContext->Draw(vertexCount, startVertexLocation);
	}

	void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) {
		mImmediateContext->DrawIndexed(indexCount, startIndexLocation, startVertexLocation);
	}

	void Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil) {
		Clear(r, g, b, a);		
		if (mCurrentDSView)
			mImmediateContext->ClearDepthStencilView(mCurrentDSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, (float)z, stencil);
	}

	void Clear(Real r, Real g, Real b, Real a) {
		float ClearColor[4] = { (float)r, (float)g, (float)b, (float)a }; // red,green,blue,alpha
		for (size_t i = 0; i<mCurrentRTViews.size(); i++)
		{
			if (mCurrentRTViews[i])
				mImmediateContext->ClearRenderTargetView(mCurrentRTViews[i], ClearColor);
		}
	}

	void ClearState() {
		mImmediateContext->ClearState();		
	}

	void Present() {
		for (auto& it : mSwapChains)
		{
			HRESULT hr = it.second->Present(0, mStandBy ? DXGI_PRESENT_TEST : 0);
			assert(!FAILED(hr));
			if (hr == DXGI_STATUS_OCCLUDED){
				mStandBy = true;
			}
			else{
				mStandBy = false;
			}
			if (!mTakeScreenshot.empty()){
				if (!mRenderTargetTextures.empty()){
					auto srcTexture = mRenderTargetTextures.begin()->second.first;
					Vec2I size = srcTexture->GetSize();
					auto pStaging = CreateTexture(0, size.x, size.y, srcTexture->GetPixelFormat(),
						BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);
					CopyToStaging(pStaging.get(), 0, 0, 0, 0, srcTexture.get(), 0, 0);
					pStaging->SaveToFile(mTakeScreenshot.c_str());
				}
				mTakeScreenshot.clear();
			}
		}

		if (mThreadPump)
		{
			//UINT io, process, device;
			//m_pThreadPump->GetQueueStatus(&io, &process, &device);
			mThreadPump->ProcessDeviceWorkItems(2);

			for (auto it = mCheckTextures.begin(); it != mCheckTextures.end();)
			{
				if ((*(*it)->GetHRPtr()) == S_OK)
				{
					TextureD3D11* pt = it->get();
					unsigned id = pt->GetTextureId();
					char buf[256];
					sprintf_s(buf, "TextureID(%u)", id);
					pt->SetDebugName(buf);
					it = mCheckTextures.erase(it);
				}
				else
				{
					if ((*(*it)->GetHRPtr()) == E_FAIL)
					{
						auto path = (*it)->GetPath();
						if (path){
							Error("Failed to load texture %s", path);
						}
						else{
							Error("Failed to load texture %u", (*it)->GetTextureId());
						}
						it = mCheckTextures.erase(it);
					}
					else
					{
						it++;
					}
				}
			}
		}
	}

	// Debugging & Profiling
	void BeginEvent(const char* name){
		D3DPERF_BeginEvent(0xffffffff, AnsiToWide(name));
	}

	void EndEvent(){
		D3DPERF_EndEvent();
	}

	void TakeScreenshot(const char* filename){
		if (ValidCStringLength(filename))
			mTakeScreenshot = filename;
	}

	//-------------------------------------------------------------------
	// Platform Specific
	//-------------------------------------------------------------------
	// Resource Manipulations
	MapData MapBuffer(ID3D11Resource* pResource,
		UINT subResource, MAP_TYPE type, MAP_FLAG flag) const{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		D3D11_MAP maptype = ConvertEnumD3D11(type);
		unsigned int flagd3d11 = ConvertEnumD3D11(flag);
		HRESULT hr = mImmediateContext->Map(pResource, subResource, maptype,
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

	void UnmapBuffer(ID3D11Resource* pResource, UINT subResource) const{
		mImmediateContext->Unmap(pResource, subResource);
	}

	void SaveTextureToFile(TextureD3D11* texture, const char* filename){
		TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(texture);
		if (pTextureD3D11)
		{
			const char* ext = FileSystem::GetExtension(filename);
			D3DX11_IMAGE_FILE_FORMAT format = D3DX11_IFF_FORCE_DWORD;
			if (_stricmp(ext, ".bmp") == 0)
			{
				format = D3DX11_IFF_BMP;
			}
			else if (_stricmp(ext, ".jpg") == 0)
			{
				format = D3DX11_IFF_JPG;
			}
			else if (_stricmp(ext, ".png") == 0)
			{
				format = D3DX11_IFF_PNG;
			}
			else if (_stricmp(ext, ".dds") == 0)
			{
				format = D3DX11_IFF_DDS;
			}
			else if (_stricmp(ext, ".tif") == 0)
			{
				format = D3DX11_IFF_TIFF;
			}
			else if (_stricmp(ext, ".gif") == 0)
			{
				format = D3DX11_IFF_GIF;
			}
			if (format == D3DX11_IFF_FORCE_DWORD)
			{
				Error("Unsupported file format!");
				return;
			}
			HRESULT hr = D3DX11SaveTextureToFile(mImmediateContext.get(), pTextureD3D11->GetHardwareTexture(), format, filename);
			if (FAILED(hr))
			{
				Error("Save texture to file is failed.");
			}

		}
	}
	void GenerateMips(TextureD3D11* pTexture){		
		ID3D11ShaderResourceView* pSRV = pTexture->GetHardwareResourceView();
		if (pSRV){
			mImmediateContext->GenerateMips(pSRV);
		}
		else{
			Error(FB_ERROR_LOG_ARG, "No shader resource view found.");
		}
	}

	// Resource Bindings
	void SetIndexBuffer(IndexBufferD3D11* pIndexBuffer, unsigned offset){		
		ID3D11Buffer* pHardwareBuffer = pIndexBuffer->GetHardwareBuffer();
		mImmediateContext->IASetIndexBuffer(pHardwareBuffer, pIndexBuffer->GetFormatD3D11(), offset);
	}

	void SetTexture(TextureD3D11* pTexture, BINDING_SHADER shaderType, unsigned int slot){
		ID3D11ShaderResourceView* const pSRV = pTexture ? pTexture->GetHardwareResourceView() : 0;
		try
		{
			switch (shaderType)
			{
			case BINDING_SHADER_VS:
				mImmediateContext->VSSetShaderResources(slot, 1, &pSRV);
				break;
			case BINDING_SHADER_GS:
				mImmediateContext->GSSetShaderResources(slot, 1, &pSRV);
				break;
			case BINDING_SHADER_PS:
				mImmediateContext->PSSetShaderResources(slot, 1, &pSRV);
				break;
			default:
				assert(0);
				break;
			}
		}
		catch (...)
		{
			Error(FB_ERROR_LOG_ARG, "Exception caught.");
		}
	}

	void SetShaders(ShaderD3D11* pShader){
		ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
		ID3D11VertexShader* pVS = pShaderD3D11->GetVertexShader();
		mImmediateContext->VSSetShader(pVS, 0, 0);
		ID3D11GeometryShader* pGS = pShaderD3D11->GetGeometryShader();
		mImmediateContext->GSSetShader(pGS, 0, 0);
		ID3D11HullShader* pHS = pShaderD3D11->GetHullShader();
		mImmediateContext->HSSetShader(pHS, 0, 0);
		ID3D11DomainShader* pDS = pShaderD3D11->GetDomainShader();
		mImmediateContext->DSSetShader(pDS, 0, 0);
		ID3D11PixelShader* pPS = pShaderD3D11->GetPixelShader();
		mImmediateContext->PSSetShader(pPS, 0, 0);
	}

	void SetVSShader(ShaderD3D11* pShader){
		if (pShader == 0)
		{
			mImmediateContext->VSSetShader(0, 0, 0);
			return;
		}

		if (pShader->GetCompileFailed())
		{
			return;
		}

		ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
		ID3D11VertexShader* pVS = pShaderD3D11->GetVertexShader();
		mImmediateContext->VSSetShader(pVS, 0, 0);		
	}

	void SetHSShader(ShaderD3D11* pShader){
		if (pShader == 0)
		{
			mImmediateContext->HSSetShader(0, 0, 0);
			return;
		}

		if (pShader->GetCompileFailed())
		{
			return;
		}

		ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
		ID3D11HullShader* pHS = pShaderD3D11->GetHullShader();
		mImmediateContext->HSSetShader(pHS, 0, 0);		
	}

	void SetDSShader(ShaderD3D11* pShader){
		if (pShader == 0)
		{
			mImmediateContext->DSSetShader(0, 0, 0);
			return;
		}

		if (pShader->GetCompileFailed())
		{
			return;
		}

		ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
		ID3D11DomainShader* pDS = pShaderD3D11->GetDomainShader();
		mImmediateContext->DSSetShader(pDS, 0, 0);
	}

	void SetGSShader(ShaderD3D11* pShader){
		if (pShader == 0)
		{
			mImmediateContext->GSSetShader(0, 0, 0);
			return;
		}

		if (pShader->GetCompileFailed())
		{
			return;
		}

		ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
		ID3D11GeometryShader* pGS = pShaderD3D11->GetGeometryShader();
		mImmediateContext->GSSetShader(pGS, 0, 0);		
	}

	void SetPSShader(ShaderD3D11* pShader){
		if (pShader == 0)
		{
			mImmediateContext->PSSetShader(0, 0, 0);
			return;
		}

		if (pShader->GetCompileFailed())
		{
			return;
		}

		ShaderD3D11* pShaderD3D11 = static_cast<ShaderD3D11*>(pShader);
		ID3D11PixelShader* pPS = pShaderD3D11->GetPixelShader();
		mImmediateContext->PSSetShader(pPS, 0, 0);
	}

	void SetInputLayout(InputLayoutD3D11* pInputLayout){
		ID3D11InputLayout* pLayout = pInputLayout->GetHardwareInputLayout();
		assert(pLayout);
		mImmediateContext->IASetInputLayout(pLayout);
	}

	void SetRasterizerState(RasterizerStateD3D11* pRasterizerState){
		mImmediateContext->RSSetState(pRasterizerState->GetHardwareRasterizerState());
	}

	void SetBlendState(BlendStateD3D11* pBlendState){		
		mImmediateContext->OMSetBlendState(pBlendState->GetHardwareBlendState(),
			pBlendState->GetBlendFactor(), pBlendState->GetSampleMask());
	}

	void SetDepthStencilState(DepthStencilStateD3D11* pDepthStencilState, unsigned stencilRef){
		mImmediateContext->OMSetDepthStencilState(pDepthStencilState->GetHardwareDSState(),
			stencilRef);
	}

	void SetSamplerState(SamplerStateD3D11* pSamplerState, BINDING_SHADER shader, int slot){
		ID3D11SamplerState* pSS = pSamplerState->GetHardwareSamplerState();
		switch (shader)
		{
		case BINDING_SHADER_VS:
			mImmediateContext->VSSetSamplers(slot, 1, &pSS);
			break;
		case BINDING_SHADER_PS:
			mImmediateContext->PSSetSamplers(slot, 1, &pSS);
			break;
		default:
			Log("Setting a sampler state into the undefined shader stage.");
			assert(0);
		}
	}

	// Privates
	bool FindClosestMatchingMode(const DXGI_MODE_DESC* finding, DXGI_MODE_DESC* best, HMONITOR monitor){
		IDXGIAdapter1 * adapter;
		UINT i = 0;
		while (mDXGIFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			IDXGIAdapter1Ptr adapterPtr(adapter, IUnknownDeleter());
			UINT j = 0;
			IDXGIOutput* output;
			while (adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND)
			{
				IDXGIOutputPtr outputPtr(output, IUnknownDeleter());
				DXGI_OUTPUT_DESC desc;
				if (SUCCEEDED(output->GetDesc(&desc))){
					if (desc.Monitor == monitor){
						if (SUCCEEDED(output->FindClosestMatchingMode(finding, best, mDevice.get()))){							
							return true;
						}
					}
				}
				++j;
			}
			++i;
		}

		return false;
	}

	void GetOutputInformationFor(IDXGIAdapter1* adapter){
		UINT i = 0;
		IDXGIOutput* pOutput;
		std::vector<IDXGIOutput*> vOutputs;
		while (adapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			vOutputs.push_back(pOutput);
			++i;
		}
		mOutputInfos.clear();
		for (auto output : vOutputs){
			DXGI_OUTPUT_DESC desc;
			auto hr = output->GetDesc(&desc);
			if (SUCCEEDED(hr)){
				mOutputInfos.push_back(desc);
				auto& modes = mDisplayModes[desc.Monitor];
				/* do something */
			}
			SAFE_RELEASE(output);
		}
	}

	void ChangeWindowStyle(HWND window, LONG_PTR newstyle){
		auto ret = SetWindowLongPtr(window, GWL_STYLE, newstyle);
		if (ret == 0){
			Error("ChangeStyle is failed for window: value %u", newstyle);
		}
		else{
			SetWindowPos(window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		}
	}

	void ChangeWindowRect(HWND window, const RECT& rect){
		SetWindowPos(window, 0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
	}

	bool CreateTargetTexturesFor(IDXGISwapChain* pSwapChain, const Vec2I& size, TextureD3D11Ptr& color, TextureD3D11Ptr& depth){
		// RenderTargetView
		ID3D11Texture2D* pBackBuffer = NULL;
		auto hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to get backbuffer!");
			return false;
		}
		ID3D11RenderTargetView* pRenderTargetView = NULL;
		hr = mDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
		if (FAILED(hr))
		{
			Error(FB_ERROR_LOG_ARG, "Failed to create a render target view!");
			return false;
		}
		color = TextureD3D11::Create();
		color->SetHardwareTexture(pBackBuffer);
		color->AddRenderTargetView(pRenderTargetView);
		color->SetSize(size);

		depth = std::dynamic_pointer_cast<TextureD3D11>(
			CreateTexture(0, size.x, size.y, mDepthStencilFormat, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL));
		return true;
	}

	bool ResizeSwapChain(HWindowId hwndId, const Vec2I& resol, TextureD3D11Ptr& outColor, TextureD3D11Ptr& outDepth){
		mImmediateContext->OMSetRenderTargets(0, 0, 0);
		Vec2I originalSize;
		// release render target textures
		auto it = mRenderTargetTextures.Find(hwndId);
		if (it == mRenderTargetTextures.end())
		{
			Error(FB_ERROR_LOG_ARG, FormatString("Cannot find the swap chain render target with id %u", hwndId).c_str());
			return false;
		}
		else
		{
			Log(FormatString("Releasing render target %u.", hwndId).c_str());
			originalSize = it->second.first->GetSize();
			mRenderTargetTextures.erase(it);
		}

		// resize swap chain
		auto itSwapChain = mSwapChains.Find(hwndId);
		TextureD3D11Ptr color, depth;
		if (itSwapChain != mSwapChains.end()) {
			auto hr = itSwapChain->second->ResizeBuffers(1, resol.x, resol.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
			if (!SUCCEEDED(hr)){
				Error("Resizing swapchain to %dx%d is failed(0x%x", resol.x, resol.y, hr);	
				bool successful = CreateTargetTexturesFor(itSwapChain->second.get(), originalSize, color, depth);
				if (!successful){
					Error("Failed to recover original size of render target textures.");
					return false;
				}
				mRenderTargetTextures[hwndId] = { color, depth };
				outColor = color;
				outDepth = depth;
				return false;
			}
			bool successful = CreateTargetTexturesFor(itSwapChain->second.get(), resol, color, depth);
			if (!successful){
				Error("Failed to create Render target for swap chain %d", hwndId);
				return false;
			}
			mRenderTargetTextures[hwndId] = { color, depth };
			outColor = color;
			outDepth = depth;
			return true;
		}
		else{
			Error("No swap chain found for %d", hwndId);
			return false;
		}
	}	
};

//---------------------------------------------------------------------------
static RendererD3D11* sRenderer = 0;
IPlatformRenderer* RendererD3D11::Create(){
	if (sRenderer)
		return sRenderer;
	sRenderer = new RendererD3D11();
	return sRenderer;
}
void RendererD3D11::Destroy(){
	delete sRenderer;
	sRenderer = 0;
}

RendererD3D11& RendererD3D11::GetInstance(){
	if (!sRenderer)
	{
		Logger::Log(FB_ERROR_LOG_ARG, "RendererD3D11 is destroyed abnormally. Program will crash..");
	}

	return *sRenderer;
}

RendererD3D11::RendererD3D11()
	: mImpl(new Impl)
{

}

RendererD3D11::~RendererD3D11(){

}

//-------------------------------------------------------------------
// IPlatformRenderer interface
//-------------------------------------------------------------------		
// Device features
Vec2ITuple RendererD3D11::FindClosestSize(HWindowId id, const Vec2ITuple& input) {
	return mImpl->FindClosestSize(id, input);
}

bool RendererD3D11::GetResolutionList(unsigned& outNum, Vec2ITuple* list) {
	return mImpl->GetResolutionList(outNum, list);
}

bool RendererD3D11::InitCanvas(HWindowId id, HWindow window, int width, int height, int fullscreen,	IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) {
	return mImpl->InitCanvas(id, window, width, height, fullscreen, outColorTexture, outDepthTexture);
}

void RendererD3D11::DeinitCanvas(HWindowId id, HWindow window) {
	mImpl->DeinitCanvas(id, window);
}

bool RendererD3D11::ChangeResolution(HWindowId id, HWindow window, const Vec2ITuple& resol,	IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) {
	return mImpl->ChangeResolution(id, window, resol, outColorTexture, outDepthTexture);
}

bool RendererD3D11::ChangeFullscreenMode(HWindowId id, HWindow window, int mode) {
	return mImpl->ChangeFullscreenMode(id, window, mode);
}

unsigned RendererD3D11::GetMultiSampleCount() const {
	return mImpl->GetMultiSampleCount();
}

// Resource creation
void RendererD3D11::SetShaderCacheOption(bool useShaderCache, bool generateCache) {
	mImpl->SetShaderCacheOption(useShaderCache, generateCache);
}

IPlatformTexturePtr RendererD3D11::CreateTexture(const char* path, bool async) {
	return mImpl->CreateTexture(path, async);
}

IPlatformTexturePtr RendererD3D11::CreateTexture(void* data, int width, int height,	PIXEL_FORMAT format, BUFFER_USAGE usage, int  buffer_cpu_access,	int texture_type) {
	return mImpl->CreateTexture(data, width, height, format, usage, buffer_cpu_access, texture_type);
}

IPlatformVertexBufferPtr RendererD3D11::CreateVertexBuffer(void* data, unsigned stride,	unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) {
	return mImpl->CreateVertexBuffer(data, stride, numVertices, usage, accessFlag);
}

IPlatformIndexBufferPtr RendererD3D11::CreateIndexBuffer(void* data, unsigned int numIndices,	INDEXBUFFER_FORMAT format) {
	return mImpl->CreateIndexBuffer(data, numIndices, format);
}

IPlatformShaderPtr RendererD3D11::CreateShader(const char* path, int shaders,	const SHADER_DEFINES& defines) {
	return mImpl->CreateShader(path, shaders, defines);
}

IPlatformInputLayoutPtr RendererD3D11::CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,	void* shaderByteCode, unsigned size) {
	return mImpl->CreateInputLayout(descs, shaderByteCode, size);
}

IPlatformBlendStatePtr RendererD3D11::CreateBlendState(const BLEND_DESC& desc) {
	return mImpl->CreateBlendState(desc);
}

IPlatformDepthStencilStatePtr RendererD3D11::CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc) {
	return mImpl->CreateDepthStencilState(desc);
}

IPlatformRasterizerStatePtr RendererD3D11::CreateRasterizerState(const RASTERIZER_DESC& desc) {
	return mImpl->CreateRasterizerState(desc);
}

IPlatformSamplerStatePtr RendererD3D11::CreateSamplerState(const SAMPLER_DESC& desc) {
	return mImpl->CreateSamplerState(desc);
}

unsigned RendererD3D11::GetNumLoadingTexture() const {
	return mImpl->GetNumLoadingTexture();
}

// Resource Binding
void RendererD3D11::SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,	IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) {
	mImpl->SetRenderTarget(pRenderTargets, rtViewIndex, num, pDepthStencil, dsViewIndex);
}

void RendererD3D11::SetViewports(const Viewport viewports[], int num) {
	mImpl->SetViewports(viewports, num);
}

void RendererD3D11::SetScissorRects(const Rect rects[], int num) {
	mImpl->SetScissorRects(rects, num);
}

void RendererD3D11::SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,	IPlatformVertexBuffer const * pVertexBuffers[], unsigned int const strides[], unsigned int offsets[]) {
	mImpl->SetVertexBuffers(startSlot, numBuffers, pVertexBuffers, strides, offsets);
}

void RendererD3D11::SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt) {
	mImpl->SetPrimitiveTopology(pt);
}

void RendererD3D11::SetTextures(IPlatformTexturePtr pTextures[], int num, BINDING_SHADER shaderType, int startSlot) {
	mImpl->SetTextures(pTextures, num, shaderType, startSlot);
}

void RendererD3D11::UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size) {
	mImpl->UpdateShaderConstants(type, data, size);
}

void* RendererD3D11::MapMaterialParameterBuffer() const {
	return mImpl->MapMaterialParameterBuffer();
}

void RendererD3D11::UnmapMaterialParameterBuffer() const {
	mImpl->UnmapMaterialParameterBuffer();
}

void* RendererD3D11::MapBigBuffer() const {
	return mImpl->MapBigBuffer();
}

void RendererD3D11::UnmapBigBuffer() const {
	return mImpl->UnmapBigBuffer();
}

void RendererD3D11::UnbindInputLayout() {
	mImpl->UnbindInputLayout();
}

void RendererD3D11::UnbindShader(BINDING_SHADER shader) {
	mImpl->UnbindShader(shader);
}

void RendererD3D11::UnbindTexture(BINDING_SHADER shader, int slot) {
	mImpl->UnbindTexture(shader, slot);
}

void RendererD3D11::CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,	IPlatformTexture* src, UINT srcSubresource, Box3D* pBox) {
	mImpl->CopyToStaging(dst, dstSubresource, dstx, dsty, dstz, src, srcSubresource, pBox);
}

// Drawing
void RendererD3D11::Draw(unsigned int vertexCount, unsigned int startVertexLocation) {
	mImpl->Draw(vertexCount, startVertexLocation);
}

void RendererD3D11::DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) {
	mImpl->DrawIndexed(indexCount, startIndexLocation, startVertexLocation);
}

void RendererD3D11::Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil) {
	mImpl->Clear(r, g, b, a, z, stencil);
}

void RendererD3D11::Clear(Real r, Real g, Real b, Real a) {
	mImpl->Clear(r, g, b, a);
}

void RendererD3D11::ClearState() {
	mImpl->ClearState();
}

void RendererD3D11::Present() {
	mImpl->Present();
}

// Debugging & Profiling
void RendererD3D11::BeginEvent(const char* name) {
	mImpl->BeginEvent(name);
}

void RendererD3D11::EndEvent() {
	mImpl->EndEvent();
}

void RendererD3D11::TakeScreenshot(const char* filename) {
	mImpl->TakeScreenshot(filename);
}

//-------------------------------------------------------------------
// Platform Specific
//-------------------------------------------------------------------
// Resource Manipulations
MapData RendererD3D11::MapBuffer(ID3D11Resource* pResource, UINT subResource, MAP_TYPE type, MAP_FLAG flag) const {
	return mImpl->MapBuffer(pResource, subResource, type, flag);
}

void RendererD3D11::UnmapBuffer(ID3D11Resource* pResource, UINT subResource) const {
	mImpl->UnmapBuffer(pResource, subResource);
}

void RendererD3D11::SaveTextureToFile(TextureD3D11* texture, const char* filename) {
	mImpl->SaveTextureToFile(texture, filename);
}

void RendererD3D11::GenerateMips(TextureD3D11* pTexture) {
	mImpl->GenerateMips(pTexture);
}

// Resource Bindings
void RendererD3D11::SetIndexBuffer(IndexBufferD3D11* pIndexBuffer, unsigned offset) {
	mImpl->SetIndexBuffer(pIndexBuffer, offset);
}

void RendererD3D11::SetTexture(TextureD3D11* pTexture, BINDING_SHADER shaderType, unsigned int slot) {
	mImpl->SetTexture(pTexture, shaderType, slot);
}

void RendererD3D11::SetShaders(ShaderD3D11* pShader) {
	mImpl->SetShaders(pShader);
}

void RendererD3D11::SetVSShader(ShaderD3D11* pShader) {
	mImpl->SetVSShader(pShader);
}

void RendererD3D11::SetHSShader(ShaderD3D11* pShader) {
	mImpl->SetHSShader(pShader);
}

void RendererD3D11::SetDSShader(ShaderD3D11* pShader) {
	mImpl->SetDSShader(pShader);
}

void RendererD3D11::SetGSShader(ShaderD3D11* pShader) {
	mImpl->SetGSShader(pShader);
}

void RendererD3D11::SetPSShader(ShaderD3D11* pShader) {
	mImpl->SetPSShader(pShader);
}

void RendererD3D11::SetInputLayout(InputLayoutD3D11* pInputLayout) {
	mImpl->SetInputLayout(pInputLayout);
}

void RendererD3D11::SetRasterizerState(RasterizerStateD3D11* pRasterizerState) {
	mImpl->SetRasterizerState(pRasterizerState);
}

void RendererD3D11::SetBlendState(BlendStateD3D11* pBlendState) {
	mImpl->SetBlendState(pBlendState);
}

void RendererD3D11::SetDepthStencilState(DepthStencilStateD3D11* pDepthStencilState, unsigned stencilRef) {
	mImpl->SetDepthStencilState(pDepthStencilState, stencilRef);
}

void RendererD3D11::SetSamplerState(SamplerStateD3D11* pSamplerState, BINDING_SHADER shader, int slot) {
	mImpl->SetSamplerState(pSamplerState, shader, slot);
}

