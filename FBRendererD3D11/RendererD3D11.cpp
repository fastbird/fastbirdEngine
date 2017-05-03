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
#include "FBSerializationLib/Serialization.h"
#include "FBStringLib/StringLib.h"
#include "EssentialEngineData/shaders/CommonDefines.h"
#include "FBFileSystem/FileSystem.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBThread/TaskScheduler.h"
#include "FBThread/Invoker.h"
#include "FBCommonHeaders/SpinLock.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <set>

#define _RENDERER_FRAME_PROFILER_
//#define MAIN_THREAD_CHECK if (!IsMainThread()) {__debugbreak();}
#define MAIN_THREAD_CHECK
using namespace fb;

#define SEPERATED_THREAD_FOR_COMPUTE_SHADER 1

static void Error(const char* szFmt, ...){
	static const size_t BufferSize = 2048;
	std::vector<char> buffer(BufferSize, 0);
	va_list args;
	va_start(args, szFmt);
	auto len = (size_t)_vscprintf(szFmt, args) + 1;
	if (len > BufferSize) {
		buffer.resize(len, 0);
	}
	auto s = buffer.size();
	vsprintf_s((char*)&buffer[0], buffer.size(), szFmt, args);
	va_end(args);

	Logger::Log(FB_ERROR_LOG_ARG, &buffer[0]);
}

static void Log(const char* szFmt, ...){
	static const size_t BufferSize = 2048;
	std::vector<char> buffer(BufferSize, 0);
	va_list args;
	va_start(args, szFmt);
	auto len = (size_t)_vscprintf(szFmt, args) + 1;
	if (len > BufferSize) {
		buffer.resize(len, 0);
	}
	auto s = buffer.size();
	vsprintf_s((char*)&buffer[0], buffer.size(), szFmt, args);
	va_end(args);

	Logger::Log(FB_DEFAULT_LOG_ARG, &buffer[0]);
}

using ScratchImagePtr = std::shared_ptr<DirectX::ScratchImage>;
static DirectX::TexMetadata GetMetadata(const char* path);
static ScratchImagePtr LoadScratchImage(const char* path, bool generateMip, DirectX::TexMetadata& metadata);
static ScratchImagePtr ConvertScratchImage(const ScratchImagePtr& srcImage);
//----------------------------------------------------------------------------
class RendererD3D11::Impl
{
public:
	HWindow INVALID_HWND = (HWindow)-1;
	bool mQuit = false;
	IDXGIFactory1Ptr		mDXGIFactory;
	ID3D11DevicePtr			mDevice; // free-threaded	
	ID3D11DeviceContextPtr	mImmediateContext; //  not free-threaded
	/// Device2 is for DirectCompute.
	ID3D11DevicePtr			mDevice2; // free-threaded
	// ImmediateContext2 is for DirectComput.
	ID3D11DeviceContextPtr	mImmediateContext2; //  not free-threaded
	VectorMap<HWindowId, IDXGISwapChainPtr> mSwapChains;
	VectorMap<HWindowId, std::pair<TextureD3D11Ptr, TextureD3D11Ptr> > mRenderTargetTextures;
	DXGI_SAMPLE_DESC		mMultiSampleDesc;
	D3D_DRIVER_TYPE			mDriverType;
	D3D_FEATURE_LEVEL		mFeatureLevel;
	PIXEL_FORMAT			mColorFormat;
	PIXEL_FORMAT			mDepthStencilFormat;

	// constant buffers
	ID3D11Buffer*			mShaderConstants[ShaderConstants::Num];	
	ID3D11Buffer*			mComputeShaderConstants;
	ID3D11BufferPtr		mComputeShaderResult;
	ID3D11UnorderedAccessViewPtr mComputeShaderResultUAV;
	ID3D11BufferPtr		mComputeShaderResultStage;
	size_t						mComputeShaderResultSize;
	ID3D11RasterizerStatePtr mWireframeRasterizeState;

	std::vector<DXGI_OUTPUT_DESC> mOutputInfos;
	VectorMap<HMONITOR, std::vector<DXGI_MODE_DESC>> mDisplayModes;

	std::vector<ID3D11RenderTargetView*> mCurrentRTViews;
	ID3D11DepthStencilView* mCurrentDSView;

	std::atomic<unsigned> mNumLoadingTexture = 0;
	bool mStandBy;
	bool mUseShaderCache;
	bool mGenerateShaderCache;
	std::string mTakeScreenshot;
	/// The main thread is the thread in which the device is created.
	/// Currently we are assuming the device is always created in the game update thread.
	std::thread::id mMainThreadId;
	std::thread::id mMainThreadId2;

	// only for debugging.
	ShaderD3D11* mCurrentShaders[SHADER_TYPE_COUNT];
	bool mDeviceRemoved;
	

	//-------------------------------------------------------------------
	Impl()
		: mStandBy(false)
		, mUseShaderCache(true)
		, mGenerateShaderCache(true)
		, mColorFormat(PIXEL_FORMAT_R8G8B8A8_UNORM)
		, mDepthStencilFormat(PIXEL_FORMAT_D24_UNORM_S8_UINT)
		, mComputeShaderResultSize(0)
		, mDeviceRemoved(false)
	{
		mMainThreadId = std::this_thread::get_id();
		mMainThreadId2 = mMainThreadId;
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
		ID3D11Device* device2 = 0;
		ID3D11DeviceContext* immediateContext2 = 0;
		for (int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			mDriverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(vAdapters[0].get(), mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &device, &mFeatureLevel, &immediateContext);
			if (SUCCEEDED(hr)) {
#if SEPERATED_THREAD_FOR_COMPUTE_SHADER
				auto hr2 = D3D11CreateDevice(vAdapters[0].get(), mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
					D3D11_SDK_VERSION, &device2, NULL, &immediateContext2);
				if (FAILED(hr2)) {
					Logger::Log(FB_ERROR_LOG_ARG, "Failed to create the second d3d11 device. DirectCompute will not be available.");
				}
#endif
				break;
			}
		}
		if (FAILED(hr))
		{
			Error("D3D11CreateDevice() failed!");
			return;
		}

		mDevice = ID3D11DevicePtr(device, IUnknownDeleter());
		mImmediateContext = ID3D11DeviceContextPtr(immediateContext, IUnknownDeleter());
#if SEPERATED_THREAD_FOR_COMPUTE_SHADER
		if (device2) {
			mDevice2 = ID3D11DevicePtr(device2, IUnknownDeleter());
			mImmediateContext2 = ID3D11DeviceContextPtr(immediateContext2, IUnknownDeleter());
		}
#else
		mDevice2 = mDevice;
		mImmediateContext2 = mImmediateContext;
#endif

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
			sizeof(SHADER_CONSTANTS),
			sizeof(RARE_CONSTANTS),
			sizeof(BIG_BUFFER),
			sizeof(RAD_CONSTANTS),
			sizeof(POINT_LIGHT_CONSTANTS),
			sizeof(CAMERA_CONSTANTS),
			sizeof(RENDERTARGET_CONSTANTS),
			sizeof(SCENE_CONSTANTS),
			sizeof(SHADOW_CONSTANTS),
			sizeof(IMMUTABLE_CONSTANTS),
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
				Error(FB_ERROR_LOG_ARG, "Failed to create constant buffer!");
			}
			else{
				mShaderConstants[i] = buffer;
			}
		}
#if SEPERATED_THREAD_FOR_COMPUTE_SHADER
		Desc.ByteWidth = sizeof(SHADER_CONSTANTS);
		ID3D11Buffer* buffer = 0;
		hr = mDevice2->CreateBuffer(&Desc, NULL, &buffer);
		if (FAILED(hr)) {
			Logger::Log(FB_ERROR_LOG_ARG, "Failed to create a constant buffer for Device2.");
		}
		else {
			mComputeShaderConstants = buffer;
		}
#else
		mComputeShaderConstants = mShaderConstants[ShaderConstants::MaterialParam];
#endif
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
	}

	~Impl(){
		for (int i = 0; i < ShaderConstants::Num; ++i){
			SAFE_RELEASE(mShaderConstants[i]);
		}		
#if SEPERATED_THREAD_FOR_COMPUTE_SHADER
		SAFE_RELEASE(mComputeShaderConstants);
#endif
	}

	void PrepareQuit() {
		ENTER_CRITICAL_SECTION lock(ComputeShaderQueueLock);
		mQuit = true;
		mQueuedComputeShaders.clear();
	}

	// Device features
	Vec2ITuple FindClosestSize(HWindowId id, const Vec2ITuple& input){
		Vec2I closest = input;
		auto it = mSwapChains.find(id);
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
		if (FAILED(output->GetDisplayModeList(ConvertEnumD3D11(mColorFormat), 0, &num, 0))){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("RendererD3D11::FindClosestSize : GetDisplayModeList #1 is failed for swap chain %d", id).c_str());			
			return closest;
		}

		Real shortestDist = FLT_MAX;
		auto descsPtr = std::shared_ptr<DXGI_MODE_DESC>(FB_ARRAY_NEW(DXGI_MODE_DESC, num), [](DXGI_MODE_DESC* obj){ FB_ARRAY_DELETE(obj); });
		DXGI_MODE_DESC* descs = descsPtr.get();
		if (SUCCEEDED(output->GetDisplayModeList(ConvertEnumD3D11(mColorFormat), 0, &num, descs))){
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
		auto it = mSwapChains.find(1);
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
			if (FAILED(output->GetDisplayModeList(ConvertEnumD3D11(mColorFormat), 0, &num, 0))){
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
			if (SUCCEEDED(output->GetDisplayModeList(ConvertEnumD3D11(mColorFormat), 0, &num, descs))){
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

	bool InitCanvas(const CanvasInitInfo& info,
		IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture){		
		if (info.mWindow == INVALID_HWND)
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No valid window.");
			return false;
		}

		mDepthStencilFormat = info.mDepthFormat;
		mColorFormat = info.mColorFormat;
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = info.mWidth;
		sd.BufferDesc.Height = info.mHeight;
		sd.BufferDesc.Format = ConvertEnumD3D11(info.mColorFormat);
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = 0;

		DXGI_MODE_DESC findingMode;		
		findingMode.Width = info.mWidth;
		findingMode.Height = info.mHeight;
		findingMode.Format = ConvertEnumD3D11(info.mColorFormat);
		findingMode.RefreshRate.Numerator = 60;
		findingMode.RefreshRate.Denominator = 1;
		findingMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		findingMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		DXGI_MODE_DESC bestMatch;
		if (info.mId == 1){
			HMONITOR monitorHandle = MonitorFromWindow((HWND)info.mWindow, MONITOR_DEFAULTTONEAREST);
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
		sd.OutputWindow = (HWND)info.mWindow;
		sd.SampleDesc = mMultiSampleDesc;
		auto r_fullscreen = info.mFullscreen;
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
		mSwapChains[info.mId] = IDXGISwapChainPtr(pSwapChain, IUnknownDeleter());

		if (r_fullscreen == 2){
			// faked fullscreen
			IDXGIOutput* output;
			if (SUCCEEDED(pSwapChain->GetContainingOutput(&output))){
				DXGI_OUTPUT_DESC desc;
				if (SUCCEEDED(output->GetDesc(&desc))){
					ChangeWindowStyle((HWND)info.mWindow, 0);
					ChangeWindowRect((HWND)info.mWindow, desc.DesktopCoordinates);
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
		mRenderTargetTextures[info.mId] = { color, depth };
		outColorTexture = color;
		outDepthTexture = depth;
		return true;

	}

	void DeinitCanvas(HWindowId id, HWindow window){
		auto it = mRenderTargetTextures.find(id);
		if (it != mRenderTargetTextures.end()){
			mRenderTargetTextures.erase(it);
		}

		auto itSwapChain = mSwapChains.find(id);
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
		auto swIt = mSwapChains.find(id);
		if (swIt == mSwapChains.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "No swap chain found.");
			return false;
		}
		Vec2I resol = newResol;
		if (resol.x == 0 || resol.y == 0){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid resolution");
			return false;
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
		auto it = mSwapChains.find(id);
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

	bool IsFullscreen() const {
		auto it = mSwapChains.begin();
		if (it == mSwapChains.end())
			return false;
		BOOL fullscreen;
		it->second->GetFullscreenState(&fullscreen, 0);
		return fullscreen;
	}

	bool IsMainThread() const {
		auto threadId = std::this_thread::get_id();
		return threadId == mMainThreadId;// || threadId == mMainThreadId2;
	}
	// Resource creation
	void SetShaderCacheOption(bool useShaderCache, bool generateCache){
		mUseShaderCache = useShaderCache;
		mGenerateShaderCache = generateCache;
	}

	IPlatformTexturePtr CreateTexture(const char* path, const TextureCreationOption& options){
		if (!ValidCString(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}
		
		std::string filepath(path);		
		if (!FileSystem::ResourceExists(filepath.c_str(), &filepath))
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString(
				"File(%s) is not found.", path).c_str());
			return 0;
		}		

		auto texture = TextureD3D11::Create();
		texture->SetPath(path);
		if (options.async) {			
			auto metadata = GetMetadata(path);
			texture->SetSize(Vec2I((int)metadata.width, (int)metadata.height));
			FB_DECLARE_SMART_PTR(TextureLoadTask);
			// Create load task - when finish 'with loaded data, create hardware texture and set to the texture'			
			class TextureLoadTask : public Task
			{
				TextureD3D11WeakPtr mTexture;				
				TextureCreationOption mOptions;
				Impl* mImpl; // Impl is guaranteed to be valid while TaskSchedular is alive.

			public:
				TextureLoadTask(TextureD3D11Ptr& texture, const TextureCreationOption& options, Impl* impl)
					: mTexture(texture)					
					, mOptions(options)
					, mImpl(impl)
				{
				}

				void Execute(TaskScheduler* Scheduler) OVERRIDE
				{
					
					auto texture = mTexture.lock();
					if (texture) {
						auto path = texture->GetPath();						
						DirectX::TexMetadata metadata;
						auto scratchTexture = LoadScratchImage(path, mOptions.generateMip, metadata);
						if (!scratchTexture) {
							Logger::Log(FB_ERROR_LOG_ARG, FormatString(
								"Cannot create ScratchImage for %s", path).c_str());
							--mImpl->mNumLoadingTexture;
							return;
						}				
					
						// set size
						texture->SetSize(Vec2I(metadata.width, metadata.height));
						texture->SetSizeInBytes(scratchTexture->GetPixelsSize());
						// 'with loaded data create hardware texture and set to the texture.'						
						unsigned int type = mOptions.textureType;
						auto pimpl = mImpl;						
						// delay invoke
						Invoker::GetInstance().InvokeAtEnd([pimpl, texture, scratchTexture, type]() {
							auto& metadata = scratchTexture->GetMetadata();
							if (DirectX::IsCompressed(metadata.format)) {
								if (!MultipliesOfFour(metadata.width) || !MultipliesOfFour(metadata.height)) {
									Logger::Log(FB_ERROR_LOG_ARG, FormatString(
										"Texture(%s) format is compressed but size is not mulpiflies of four. Will not displayed.", 
										texture->GetPath()).c_str());

									--pimpl->mNumLoadingTexture;
									return;
								}
							}
							
							pimpl->CreateHardwareTextureFor(texture, scratchTexture,
								BUFFER_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, BUFFER_CPU_ACCESS_NONE, type);
							--pimpl->mNumLoadingTexture;
						});
					}
					else {
						//Logger::Log(FB_ERROR_LOG_ARG, "Texture is null.");
					}
				}
			};
			
			auto task = TextureLoadTaskPtr(new TextureLoadTask(texture, options, this),
				[](TextureLoadTask* obj) {delete obj; });			
			++mNumLoadingTexture;
			TaskScheduler::GetInstance().AddTask(task);			
		}
		else {
			// load
			DirectX::TexMetadata metadata;			
			auto scratchTexture = LoadScratchImage(path, options.generateMip, metadata);
			if (!scratchTexture) {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Cannot create ScratchImage for %s", path).c_str());
				return nullptr;
			}			
			texture->SetSize(Vec2I(metadata.width, metadata.height));
			texture->SetSizeInBytes(scratchTexture->GetPixelsSize());
			// 'with loaded data create hardware texture and set to the texture.'			
			unsigned int type = options.textureType;			
			CreateHardwareTextureFor(texture, scratchTexture,
				BUFFER_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, BUFFER_CPU_ACCESS_NONE, type);
		}

		return texture;
	}

	struct TextureD3D11Hasher {
		size_t operator()(const TextureD3D11WeakPtr& obj) const {
			auto texture = obj.lock();
			if (texture)
				return (size_t)texture.get();
			else
				return 0;
		}
	};
	struct TextureD3D11EqualTo {
		size_t operator()(const TextureD3D11WeakPtr& a, const TextureD3D11WeakPtr& b) const {
			return a == b;
		}
	};
	mutable SpinLockWaitSleep sPendingTextureInitLock;
	std::unordered_set<TextureD3D11WeakPtr, TextureD3D11Hasher, TextureD3D11EqualTo> sPendingTextureInit;
	void CreateHardwareTextureFor(TextureD3D11Ptr texture, ScratchImagePtr scratchImage,
		BUFFER_USAGE usage, unsigned int bindFlag, unsigned int buffer_cpu_access, unsigned int texture_type)
	{
		if (!IsMainThread()) {
			{
				EnterSpinLock<SpinLockWaitSleep> lock(sPendingTextureInitLock);
				sPendingTextureInit.insert(texture);
			}
			auto pimpl = this;
			Invoker::GetInstance().InvokeAtEnd([pimpl, texture, scratchImage, usage, bindFlag, buffer_cpu_access,
				texture_type]() {
				pimpl->CreateHardwareTextureFor(texture, scratchImage,
					usage, bindFlag, buffer_cpu_access, texture_type);
			});			
			return;
		}

		{
			EnterSpinLock<SpinLockWaitSleep> lock(sPendingTextureInitLock);
			auto it = sPendingTextureInit.find(texture);
			if (it != sPendingTextureInit.end()) {
				sPendingTextureInit.erase(it);
			}
		}

		auto& metadata = scratchImage->GetMetadata();
		if (DirectX::IsCompressed(metadata.format)) {
			if (!MultipliesOfFour(metadata.width) || !MultipliesOfFour(metadata.height)) {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Texture(%s) format is compressed but size is not mulpiflies of four. Will not displayed.",
					texture->GetPath()).c_str());			
				return;
			}
		}
		texture->SetSize(Vec2I(metadata.width, metadata.height));
		texture->SetPixelFormat(metadata.format);		

		bool cubeMap = (texture_type & TEXTURE_TYPE_CUBE_MAP) != 0;
		unsigned int miscFlags = cubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;				

		ID3D11ShaderResourceView* pSRV = 0;
		auto ret = DirectX::CreateShaderResourceViewEx(mDevice.get(), scratchImage->GetImages(), scratchImage->GetImageCount(),
			scratchImage->GetMetadata(), ConvertEnumD3D11(usage), bindFlag, buffer_cpu_access, miscFlags,
			false, &pSRV);
		if (FAILED(ret)) {
			Logger::Log(FB_ERROR_LOG_ARG, 
				FormatString("Failed to create shader resource view for texture (%s)", texture->GetPath()).c_str());
			return;
		}
		texture->SetHardwareResourceView(pSRV);		

#ifdef _DEBUG
		auto id = texture->GetTextureId();
		char buf[256];
		sprintf_s(buf, "TextureID(%llu)", id);
		texture->SetDebugName(buf);
#endif
	}

	void DecideBindFlags(UINT& bindFlag, BUFFER_USAGE usage, int type) {
		bindFlag = 0;
		if (usage != BUFFER_USAGE_STAGING &&
			!(type & TEXTURE_TYPE_RENDER_TARGET) &&
			!(type & TEXTURE_TYPE_DEPTH_STENCIL))
		{
			bindFlag = D3D11_BIND_SHADER_RESOURCE;
		}
		if ((type & TEXTURE_TYPE_RENDER_TARGET) || (type & TEXTURE_TYPE_RENDER_TARGET_SRV))
		{
			bindFlag |= D3D11_BIND_RENDER_TARGET;
		}
		else if ((type & TEXTURE_TYPE_DEPTH_STENCIL) || (type & TEXTURE_TYPE_DEPTH_STENCIL_SRV))
		{
			bindFlag |= D3D11_BIND_DEPTH_STENCIL;
		}		
	}

	void DecideCPUAccessFlags(UINT& CPUAccessFlags, int buffer_cpu_access) {
		CPUAccessFlags = 0;
		if (buffer_cpu_access & BUFFER_CPU_ACCESS_READ)
			CPUAccessFlags = ConvertEnumD3D11(BUFFER_CPU_ACCESS_READ);
		if (buffer_cpu_access & BUFFER_CPU_ACCESS_WRITE)
			CPUAccessFlags |= ConvertEnumD3D11(BUFFER_CPU_ACCESS_WRITE);
	}

	void DecideMiscFlags(UINT& MiscFlags, bool cubeMap, int type) {
		MiscFlags = cubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
		if (type & TEXTURE_TYPE_MIPS)		{
			assert(!(type & TEXTURE_TYPE_MULTISAMPLE));
			MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}

	int PixelFormat2Bytes(PIXEL_FORMAT format)
	{
		return DirectX::BitsPerPixel(ConvertEnumD3D11(format)) / 8;
	}

	std::vector<D3D11_SUBRESOURCE_DATA> BuildSubresourceData(void* data, int width, int height, PIXEL_FORMAT format, bool cubeMap) {
		std::vector<D3D11_SUBRESOURCE_DATA> sds;
		if (cubeMap)
		{
			sds.resize(6);
			for (int i = 0; i<6; i++)
			{				
				D3D11_SUBRESOURCE_DATA& sd = sds[i];
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
		return sds;
	}

	IPlatformTexturePtr CreateTexture(void* data, int width, int height,
		PIXEL_FORMAT format, int numMips, BUFFER_USAGE usage, int  buffer_cpu_access, int type)
	{
		auto device = mDevice;
		if (type&TEXTURE_TYPE_SECOND_DEVICE) {
			if (mDevice == mDevice2)
				return nullptr;

			device = mDevice2;
		}

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
		
		auto texture = TextureD3D11::Create();
		texture->SetSize(Vec2I(width, height));
		texture->SetSizeInBytes(PixelFormat2Bytes(format) * width * height * (cubeMap ? 6 : 1));
		texture->SetPixelFormat(ConvertEnumD3D11(format));		

		if (height == 1 && type & TEXTURE_TYPE_1D) {
			ID3D11Texture1D *pTextureD3D11 = NULL;
			D3D11_TEXTURE1D_DESC desc;
			desc.Width = width;
			desc.MipLevels = numMips;			
			desc.ArraySize = cubeMap ? 6 : 1;
			desc.Format = ConvertEnumD3D11(format);
			desc.Usage = ConvertEnumD3D11(usage);			
			DecideBindFlags(desc.BindFlags, usage, type);
			DecideCPUAccessFlags(desc.CPUAccessFlags, buffer_cpu_access);
			DecideMiscFlags(desc.MiscFlags, cubeMap, type);

			if (data) {
				auto sds = BuildSubresourceData(data, width, height, format, cubeMap);
				if (FAILED(device->CreateTexture1D(&desc, &sds[0], &pTextureD3D11))) {
					Error(FB_ERROR_LOG_ARG, "Failed to CreateTexture from memory!");
					return 0;
				}
			}
			else {
				if (FAILED(device->CreateTexture1D(&desc, 0, &pTextureD3D11))) {
					Error(FB_ERROR_LOG_ARG, "Failed to CreateTexture from memory!");
					return 0;
				}
			}
			texture->SetHardwareTexture(pTextureD3D11);

			if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
				srvDesc.Format = ConvertEnumD3D11(format);
				assert(!(type&TEXTURE_TYPE_DEPTH_STENCIL_SRV));
				srvDesc.ViewDimension = cubeMap ? D3D_SRV_DIMENSION_TEXTURE1DARRAY :
					D3D11_SRV_DIMENSION_TEXTURE1D;
				srvDesc.Texture1D.MostDetailedMip = 0;

				D3D11_TEXTURE1D_DESC tempDest;
				pTextureD3D11->GetDesc(&tempDest);
				srvDesc.Texture1D.MipLevels = tempDest.MipLevels;
				ID3D11ShaderResourceView* pResourceView;
				if (FAILED(device->CreateShaderResourceView(pTextureD3D11, &srvDesc, &pResourceView))) {
					Logger::Log(FB_ERROR_LOG_ARG, "Failed to create shader resource view.");
				}
				else {
					texture->SetHardwareResourceView(pResourceView);
				}
			}
		}
		else {
			ID3D11Texture2D *pTextureD3D11 = NULL;
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = numMips;
			if (type & TEXTURE_TYPE_MIPS && numMips != 0) {
				Logger::Log(FB_ERROR_LOG_ARG, "Forcing TEXTURE_TYPE_MIPS texture mip levels to 0");
				assert(!(type & TEXTURE_TYPE_MULTISAMPLE));
				desc.MipLevels = 0;
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
			DecideBindFlags(desc.BindFlags, usage, type);
			DecideCPUAccessFlags(desc.CPUAccessFlags, buffer_cpu_access);
			DecideMiscFlags(desc.MiscFlags, cubeMap, type);

			if (data) {
				auto sds = BuildSubresourceData(data, width, height, format, cubeMap);
				if (FAILED(device->CreateTexture2D(&desc, &sds[0], &pTextureD3D11))) {
					Error(FB_ERROR_LOG_ARG, "Failed to CreateTexture from memory!");
					return 0;
				}
			}
			else {
				if (FAILED(device->CreateTexture2D(&desc, 0, &pTextureD3D11))) {
					Error(FB_ERROR_LOG_ARG, "Failed to CreateTexture from memory!");
					return 0;
				}
			}
			texture->SetHardwareTexture(pTextureD3D11);

			if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
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
				if (FAILED(device->CreateShaderResourceView(pTextureD3D11, &srvDesc, &pResourceView))) {
					Logger::Log(FB_ERROR_LOG_ARG, "Failed to create shader resource view.");
				}
				else {
					texture->SetHardwareResourceView(pResourceView);
				}
			}

			static std::atomic<size_t> RTV_ID = 0;
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
					auto hr = device->CreateRenderTargetView(pTextureD3D11, &renderTargetViewDesc, &pRenderTargetView);
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
						dsvd.Texture2DArray.ArraySize = 1;
						dsvd.Texture2DArray.FirstArraySlice = i;
						dsvd.Texture2DArray.MipSlice = 0;
					}
					if (type&TEXTURE_TYPE_DEPTH_STENCIL_SRV)
						dsvd.Format = DXGI_FORMAT_D32_FLOAT;
					ID3D11DepthStencilView* pDepthStencilView = 0;
					auto hr = device->CreateDepthStencilView(pTextureD3D11, &dsvd, &pDepthStencilView);
					if (FAILED(hr))
					{
						Error("Cannot create DepthStencilView!");
					}
					texture->AddDepthStencilView(pDepthStencilView);
				}
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
			static std::atomic<unsigned> ID = 0;
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
		pIndexBufferD3D11->SetFormatD3D11(ConvertEnumD3D11(format));
		return pIndexBufferD3D11;
		
	}

	class IncludeProcessor : public ID3DInclude
	{
	public:
		IncludeProcessor(ShaderD3D11* pShader, const char* path)
			: mShader(pShader)			
		{
			mProcessedFiles.push_back(path);
			assert(mShader);
		}
		~IncludeProcessor()
		{
			mShader->SetRelatedFiles(mProcessedFiles);
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
			std::string filepath = pFileName;
			StringVector triedPath;
			triedPath.push_back(filepath.c_str());
			FileSystem::Open file(filepath.c_str(), "rb", FileSystem::ErrorMode::SkipErrorMsg);
			auto err = file.Error();			
			if (err)
			{
				if (!mCurrentFolder.empty()) {
					filepath = FileSystem::ConcatPath(mCurrentFolder.c_str(), pFileName);
					triedPath.push_back(filepath.c_str());
					err = file.Reset(filepath.c_str(), "rb", FileSystem::ErrorMode::SkipErrorMsg);
				}
				if (err) {					
					Error("Failed to open include file %s", pFileName);
					for (auto& p : triedPath) {
						Logger::Log(FB_ERROR_LOG_ARG, FormatString("	Tried Path: %s",
							p.c_str()).c_str());
					}
					return S_OK;
				}
			}

			if (!ValueExistsInVector(mProcessedFiles, filepath))
				mProcessedFiles.push_back(filepath);

			fseek(file, 0, SEEK_END);
			long size = ftell(file);
			rewind(file);
			char* buffer = FB_ARRAY_NEW(char, size);
			int elements = fread(buffer, 1, size, file);
			assert(elements == size);
			*ppData = buffer;
			*pBytes = size;			
			return S_OK;
		}

		StringVector GetIncludesSV() const {
			return mProcessedFiles;
		}

		void SetCurrentFolder(std::string&& curFolder) {
			mCurrentFolder = curFolder;
		}

		void SetRelatedFiles(StringVector&& relatedFiles) {
			mProcessedFiles = relatedFiles;
		}

		virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Close(LPCVOID pData)
		{
			FB_ARRAY_DELETE((char*)pData);
			return S_OK;
		}

	private:
		StringVector mProcessedFiles;
		ShaderD3D11* mShader;
		std::string mCurrentFolder;

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
		FileSystem::Open file(filename, "r");
		if (!file.IsOpen()) {
			Logger::Log(FB_ERROR_LOG_ARG, 
				FormatString("Shader compile failed. File not found. %s", filename).c_str());
			return 0x80070002;
		}
		auto& shader_code = file.GetTextData();		
		hr = D3DCompile(&shader_code[0], shader_code.size(), filename, pDefines, includeProcessor, entryPoint, shaderModel, dwShaderFlags, 0,
			ppBlobOut, &pErrorBlob);
			/*D3DX11CompileFromFile(filename, pDefines, includeProcessor, entryPoint, shaderModel, dwShaderFlags, 0,
			0, ppBlobOut, &pErrorBlob, 0);*/
		if (FAILED(hr))
		{
			Beep(100, 50);
			if (pErrorBlob)
			{
				Error("[Error] CompileShaderFromFile %s failed!", filename);
				Logger::Log(FB_DEFAULT_LOG_ARG, (const char*)pErrorBlob->GetBufferPointer());				
				
			}
			else
			{
				Error("Shader compile error. maybe file not found %s", filename);				
			}
		}
		if (pErrorBlob)
			pErrorBlob->Release();
		return hr;
	}
	std::string GetIncludeCacheFile(const char* cachefile) const {
		if (ValidCString(cachefile)) {
			return std::string(cachefile) + ".includes";
		}
		else {
			return{};
		}
	}

	ByteArray CompileShaderFunc(const char* filepath, const char* functionName, const char* shaderModel,
		IncludeProcessor* includeProcessor, D3D_SHADER_MACRO* shaderMacros, const char* cachefile, bool ignoreCache)
	{
		FileSystem::CreateDirectory(cachefile);
		// use cache
		if (!ignoreCache && ValidCString(cachefile) && mUseShaderCache && FileSystem::CompareResourceFileModifiedTime(filepath, cachefile) == -1)
		{
			std::string includeCacheFile = GetIncludeCacheFile(cachefile);			
			// include files also containg self main shader path.
			std::ifstream includeFile(includeCacheFile, std::ios_base::binary);			
			bool includesAreNotModified = true;
			if (includeFile) {				
				StringVector includes;
				boost::archive::binary_iarchive ar(includeFile);
				ar >> includes;
				for (auto header : includes) {
					if (FileSystem::CompareResourceFileModifiedTime(header.c_str(), cachefile) != -1) {
						includesAreNotModified = false;
						break;
					}
				}
				includeProcessor->SetRelatedFiles(std::move(includes));
			}			
			if (includesAreNotModified) {
				return FileSystem::ReadBinaryFile(cachefile);
			}
		}

		// not using the cache. Compile and make the cache.		
		ID3DBlob* pBlob = 0;
		auto hr = CompileShaderFromFile(filepath, functionName, shaderModel, &pBlob, shaderMacros, includeProcessor);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pBlob);
			return{};
		}
		ByteArray data(pBlob->GetBufferSize());
		memcpy(&data[0], pBlob->GetBufferPointer(), pBlob->GetBufferSize());
		SAFE_RELEASE(pBlob);
		if (mGenerateShaderCache) {
			FileSystem::WriteBinaryFile(cachefile, data);
			auto includes = includeProcessor->GetIncludesSV();
			if (!includes.empty()) {
				std::ofstream includeFile(GetIncludeCacheFile(cachefile), std::ios_base::binary);
				if (includeFile) {
					boost::archive::binary_oarchive ar(includeFile);
					ar << includes;
				}
			}
		}

		return data;
	}

	bool LoadVS(VertexShaderD3D11* pShader, const char* filepath, const char* VSName, const char* vs_cachekey,		
		IncludeProcessor* includeProcessor, D3D_SHADER_MACRO* shaderMacros, bool ignoreCache) 
	{
		includeProcessor->SetCurrentFolder(FileSystem::GetParentPath(filepath));
		auto data = CompileShaderFunc(filepath, VSName, "vs_5_0", includeProcessor, shaderMacros, vs_cachekey, ignoreCache);
		if (data.empty()) {
			pShader->SetCompileFailed(true);
			return false;
		}		
		// Create VS
		ID3D11VertexShader* pVertexShader = 0;
		auto hr = mDevice->CreateVertexShader(&data[0], data.size(), 0, &pVertexShader);
		if (FAILED(hr))
		{			
			pShader->SetCompileFailed(true);
			return false;
		}

		pShader->SetCompileFailed(false);
		static std::atomic<unsigned> vsID = 0;
		char buf[256];
		sprintf_s(buf, "VS ID(%u)", vsID++);
		pVertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		
		pShader->SetVertexShader(pVertexShader);
		pShader->SetVertexShaderBytecode(std::move(data)); // pVSBlob will be released here		
		return true;
	}

	bool LoadGS(GeometryShaderD3D11* pShader, const char* filepath, const char* GSName, const char* gs_cachekey,
		IncludeProcessor* includeProcessor, D3D_SHADER_MACRO* shaderMacros, bool ignoreCache)
	{
		includeProcessor->SetCurrentFolder(FileSystem::GetParentPath(filepath));
		auto data = CompileShaderFunc(filepath, GSName, "gs_5_0", includeProcessor, shaderMacros, gs_cachekey, ignoreCache);
		if (data.empty()) {
			pShader->SetCompileFailed(true);
			return false;
		}
		// Create GS
		ID3D11GeometryShader* pGeometryShader = 0;
		auto hr = mDevice->CreateGeometryShader(&data[0], data.size(), 0, &pGeometryShader);
		if (FAILED(hr))
		{
			pShader->SetCompileFailed(true);
			return false;
		}
		
		pShader->SetCompileFailed(false);
		static std::atomic<unsigned> gsID = 0;
		char buf[256];
		sprintf_s(buf, "GS ID(%u)", gsID++);
		pGeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		
		pShader->SetGeometryShader(pGeometryShader);		
		return true;
	}

	bool LoadPS(PixelShaderD3D11* pShader, const char* filepath, const char* PSName, const char* ps_cachekey,
		IncludeProcessor* includeProcessor, D3D_SHADER_MACRO* shaderMacros, bool ignoreCache)
	{
		includeProcessor->SetCurrentFolder(FileSystem::GetParentPath(filepath));
		auto data = CompileShaderFunc(filepath, PSName, "ps_5_0", includeProcessor, shaderMacros, ps_cachekey, ignoreCache);
		if (data.empty()) {
			pShader->SetCompileFailed(true);
			return false;
		}
		// Create PS
		ID3D11PixelShader* pPixelShader = 0;
		auto hr = mDevice->CreatePixelShader(&data[0], data.size(), 0, &pPixelShader);
		if (FAILED(hr))
		{
			pShader->SetCompileFailed(true);
			return false;
		}
		
		pShader->SetCompileFailed(false);
		static std::atomic<unsigned> psID = 0;
		char buf[256];
		sprintf_s(buf, "PS ID(%u)", psID++);
		pPixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);
		
		pShader->SetPixelShader(pPixelShader);
		return true;
	}

	bool LoadCS(ComputeShaderD3D11* pShader, const char* filepath, const char* CSName, const char* cs_cachekey,
		IncludeProcessor* includeProcessor, D3D_SHADER_MACRO* shaderMacros, bool ignoreCache)
	{
		includeProcessor->SetCurrentFolder(FileSystem::GetParentPath(filepath));
		auto data = CompileShaderFunc(filepath, CSName, "cs_5_0", includeProcessor, shaderMacros, cs_cachekey, ignoreCache);
		if (data.empty()) {
			pShader->SetCompileFailed(true);
			return false;
		}
		// Create CS
		ID3D11ComputeShader* pComputeShader = 0;
		// Compute shader is created in Device2
		auto hr = mDevice2->CreateComputeShader(&data[0], data.size(), 0, &pComputeShader);
		if (FAILED(hr))
		{
			pShader->SetCompileFailed(true);
			return false;
		}

		pShader->SetCompileFailed(false);
		static std::atomic<unsigned> csID = 0;
		char buf[256];
		sprintf_s(buf, "CS ID(%u)", csID++);
		pComputeShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);

		pShader->SetComputeShader(pComputeShader);
		return true;
	}

	std::string BuildCacheKeyForShader(const char* path, SHADER_TYPE shaderType, const SHADER_DEFINES& defines) {
		// build cache key
		std::string cachekey = FileSystem::GetAppDataLocalGameFolder();
		cachekey += path;
		
		if (!defines.empty())
		{
			std::string namevalue;
			for (const auto& define : defines)
			{
				namevalue += define.mName;
				namevalue += define.mValue;
			}
			uLong crc = crc32(0L, Z_NULL, 0);
			crc = crc32(crc, (const Bytef*)namevalue.c_str(), namevalue.size());
			char buf[255];
			sprintf_s(buf, "%u", crc);
			cachekey += buf;
		}
		switch (shaderType) {
		case SHADER_TYPE_VS:
			cachekey += ".vscache";
			break;
		case SHADER_TYPE_HS:
			cachekey += ".hscache";
			break;
		case SHADER_TYPE_DS:
			cachekey += ".dscache";
			break;
		case SHADER_TYPE_GS:
			cachekey += ".gscache";
			break;
		case SHADER_TYPE_PS:
			cachekey += ".pscache";
			break;
		case SHADER_TYPE_CS:
			cachekey += ".cscache";
			break;
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Unknown shader type.");
		}
		return cachekey;
	}

	std::vector<D3D_SHADER_MACRO> BuildShaderMacros(const SHADER_DEFINES& defines) {
		std::vector<D3D_SHADER_MACRO> shaderMacros;
		for (DWORD i = 0; i<defines.size(); i++)
		{
			shaderMacros.push_back(D3D_SHADER_MACRO());
			shaderMacros.back().Name = defines[i].mName.c_str();
			shaderMacros.back().Definition = defines[i].mValue.c_str();
		}
		return shaderMacros;		
	}

	IPlatformShaderPtr CreateVertexShader(const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		auto pShader = VertexShaderD3D11::Create();
		CreateVertexShader(pShader.get(), path, defines, ignoreCache);
		return pShader;
	}

	bool CreateVertexShader(VertexShaderD3D11* pShader, const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache) 
	{
		std::vector<D3D_SHADER_MACRO> shaderMacros = BuildShaderMacros(defines);
		D3D_SHADER_MACRO* pShaderMacros = 0;
		if (!shaderMacros.empty())
		{
			shaderMacros.push_back(D3D_SHADER_MACRO());
			shaderMacros.back().Name = 0;
			shaderMacros.back().Definition = 0;
			pShaderMacros = &shaderMacros[0];
		}

		IncludeProcessor includeProcessor(pShader, path);
		// Load VS
		auto cachekey = BuildCacheKeyForShader(path, SHADER_TYPE_VS, defines);
		std::string VSName = FileSystem::GetName(path);
		VSName += "_VertexShader";		
		return LoadVS(pShader, path, VSName.c_str(), cachekey.c_str(), &includeProcessor, pShaderMacros, ignoreCache);		
	}

	IPlatformShaderPtr CreateGeometryShader(const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		auto pShader = GeometryShaderD3D11::Create();
		CreateGeometryShader(pShader.get(), path, defines, ignoreCache);
		return pShader;
	}

	bool CreateGeometryShader(GeometryShaderD3D11* pShader, const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		std::vector<D3D_SHADER_MACRO> shaderMacros = BuildShaderMacros(defines);
		D3D_SHADER_MACRO* pShaderMacros = 0;
		if (!shaderMacros.empty())
		{
			shaderMacros.push_back(D3D_SHADER_MACRO());
			shaderMacros.back().Name = 0;
			shaderMacros.back().Definition = 0;
			pShaderMacros = &shaderMacros[0];
		}

		IncludeProcessor includeProcessor(pShader, path);
		auto cachekey = BuildCacheKeyForShader(path, SHADER_TYPE_GS, defines);
		std::string GSName = FileSystem::GetName(path);
		GSName += "_GeometryShader";
		return LoadGS(pShader, path, GSName.c_str(), cachekey.c_str(), &includeProcessor, pShaderMacros, ignoreCache);		
	}

	IPlatformShaderPtr CreatePixelShader(const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		auto pShader = PixelShaderD3D11::Create();
		CreatePixelShader(pShader.get(), path, defines, ignoreCache);		
		return pShader;
	}

	bool CreatePixelShader(PixelShaderD3D11* pShader, const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		std::vector<D3D_SHADER_MACRO> shaderMacros = BuildShaderMacros(defines);
		D3D_SHADER_MACRO* pShaderMacros = 0;
		if (!shaderMacros.empty())
		{
			shaderMacros.push_back(D3D_SHADER_MACRO());
			shaderMacros.back().Name = 0;
			shaderMacros.back().Definition = 0;
			pShaderMacros = &shaderMacros[0];
		}

		IncludeProcessor includeProcessor(pShader, path);
		auto cachekey = BuildCacheKeyForShader(path, SHADER_TYPE_PS, defines);
		std::string PSName = FileSystem::GetName(path);
		PSName += "_PixelShader";
		return LoadPS(pShader, path, PSName.c_str(), cachekey.c_str(), &includeProcessor, pShaderMacros, ignoreCache);		
	}
	
	IPlatformShaderPtr CreateComputeShader(const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		auto pShader = ComputeShaderD3D11::Create();
		CreateComputeShader(pShader.get(), path, defines, ignoreCache);
		return pShader;
	}

	bool CreateComputeShader(ComputeShaderD3D11* pShader, const char* path,
		const SHADER_DEFINES& defines, bool ignoreCache)
	{
		std::vector<D3D_SHADER_MACRO> shaderMacros = BuildShaderMacros(defines);
		D3D_SHADER_MACRO* pShaderMacros = 0;
		if (!shaderMacros.empty())
		{
			shaderMacros.push_back(D3D_SHADER_MACRO());
			shaderMacros.back().Name = 0;
			shaderMacros.back().Definition = 0;
			pShaderMacros = &shaderMacros[0];
		}

		IncludeProcessor includeProcessor(pShader, path);		
		auto cachekey = BuildCacheKeyForShader(path, SHADER_TYPE_CS, defines);
		std::string CSName = FileSystem::GetName(path);
		CSName += "_ComputeShader";
		return LoadCS(pShader, path, CSName.c_str(), cachekey.c_str(), &includeProcessor, pShaderMacros, ignoreCache);		
	}

	IPlatformShaderPtr CompileComputeShader(const char* code, const char* entry, const SHADER_DEFINES& defines) {
		auto pShader = ComputeShaderD3D11::Create();
		std::vector<D3D_SHADER_MACRO> shaderMacros = BuildShaderMacros(defines);
		D3D_SHADER_MACRO* pShaderMacros = 0;
		if (!shaderMacros.empty())
		{
			shaderMacros.push_back(D3D_SHADER_MACRO());
			shaderMacros.back().Name = 0;
			shaderMacros.back().Definition = 0;
			pShaderMacros = &shaderMacros[0];
		}

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* bytecodeBlob;
		ID3DBlob* errorBlob;		
		auto hr = D3DCompile(code, strlen(code), NULL, pShaderMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entry, "cs_5_0", dwShaderFlags, 0, &bytecodeBlob, &errorBlob);
		ID3DBlobPtr bytecodeBlobP = ID3DBlobPtr(bytecodeBlob, IUnknownDeleter());
		ID3DBlobPtr errBlobP = ID3DBlobPtr(errorBlob, IUnknownDeleter());		
		if (FAILED(hr)) {
			if (errorBlob)
			{
				Logger::Log(FB_ERROR_LOG_ARG, errorBlob->GetBufferPointer());
			}
			else
			{
				Logger::Log(FB_ERROR_LOG_ARG, "Compute shader compiling failed.");								
			}
			Beep(100, 50);
		}
		else {
			ByteArray data(bytecodeBlob->GetBufferSize());
			memcpy(&data[0], bytecodeBlob->GetBufferPointer(), bytecodeBlob->GetBufferSize());			
			ID3D11ComputeShader* pComputeShader = 0;
			auto hr = mDevice2->CreateComputeShader(&data[0], data.size(), 0, &pComputeShader);
			if (FAILED(hr))
			{
				pShader->SetCompileFailed(true);
				return false;
			}

			static std::atomic<unsigned> csID = 0;
			char buf[256];
			sprintf_s(buf, "Compiled CS ID(%u)", csID++);
			pComputeShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buf), buf);

			pShader->SetComputeShader(pComputeShader);
		}		
		return pShader;
	}

	bool ReloadShader(ShaderD3D11* shader, const SHADER_DEFINES& defines) {
		if (!shader) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		}
		auto shaderType = shader->GetShaderType();
		switch (shaderType) {
		case SHADER_TYPE_VS: {
			auto vs = dynamic_cast<VertexShaderD3D11*>(shader);
			if (!vs) {
				Logger::Log(FB_ERROR_LOG_ARG, "Type error.");
				return false;
			}			
			return CreateVertexShader(vs, vs->GetRelatedFiles()[0].c_str(), defines, true);
		}
		case SHADER_TYPE_GS: {
			auto gs = dynamic_cast<GeometryShaderD3D11*>(shader);
			if (!gs) {
				Logger::Log(FB_ERROR_LOG_ARG, "Type error.");
				return false;
			}
			return CreateGeometryShader(gs, gs->GetRelatedFiles()[0].c_str(), defines, true);
		}
		case SHADER_TYPE_PS: {
			auto ps = dynamic_cast<PixelShaderD3D11*>(shader);
			if (!ps) {
				Logger::Log(FB_ERROR_LOG_ARG, "Type error.");
				return false;
			}
			return CreatePixelShader(ps, ps->GetRelatedFiles()[0].c_str(), defines, true);
		}
		case SHADER_TYPE_CS: {
			auto cs = dynamic_cast<ComputeShaderD3D11*>(shader);
			if (!cs) {
				Logger::Log(FB_ERROR_LOG_ARG, "Type error.");
				return false;
			}
			return CreateComputeShader(cs, cs->GetRelatedFiles()[0].c_str(), defines, true);
		}
		default:
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Unsupported shader type.");
			return false;
		}
		}
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
			static std::atomic<unsigned> ID = 0;
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
		size_t numPendingInit = 0;
		{
			EnterSpinLock<SpinLockWaitSleep> lock(sPendingTextureInitLock);
			numPendingInit = sPendingTextureInit.size();
		}
		return mNumLoadingTexture + numPendingInit;
	}

	// Resource Binding
	void SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
		const IPlatformTexturePtr& pDepthStencil, size_t dsViewIndex)
	{
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
		SetDepthTarget(pDepthStencil, dsViewIndex);
	}
	
	void SetDepthTarget(const IPlatformTexturePtr& pDepthStencil, size_t dsViewIndex) {
		MAIN_THREAD_CHECK
		mCurrentDSView = 0;
		if (pDepthStencil)
		{
			TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(pDepthStencil.get());
			mCurrentDSView = pTextureD3D11->GetDepthStencilView(dsViewIndex);
		}
		auto numrts = mCurrentRTViews.size();
		mImmediateContext->OMSetRenderTargets(numrts, numrts ? &mCurrentRTViews[0] : nullptr, mCurrentDSView);
	}

	void SetViewports(const Viewport viewports[], int num){
		MAIN_THREAD_CHECK
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
		MAIN_THREAD_CHECK
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
		MAIN_THREAD_CHECK
		D3D11_PRIMITIVE_TOPOLOGY d3d11PT = ConvertEnumD3D11(pt);
		mImmediateContext->IASetPrimitiveTopology(d3d11PT);
	}

	void SetTextures(IPlatformTexturePtr pTextures[], int num, SHADER_TYPE shaderType, int startSlot){
		MAIN_THREAD_CHECK
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
			case SHADER_TYPE_VS:
				mImmediateContext->VSSetShaderResources(startSlot, num, &rvs[0]);
				break;
			case SHADER_TYPE_PS:
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

	void BindConstants() {
		if (IsMainThread()) {
			mImmediateContext->VSSetConstantBuffers(0, ShaderConstants::Num, mShaderConstants);
			mImmediateContext->GSSetConstantBuffers(0, ShaderConstants::Num, mShaderConstants);
			mImmediateContext->PSSetConstantBuffers(0, ShaderConstants::Num, mShaderConstants);
		}

#if SEPERATED_THREAD_FOR_COMPUTE_SHADER
		mImmediateContext2->CSSetConstantBuffers(3, 1, &mComputeShaderConstants);		
#else
		mImmediateContext->CSSetConstantBuffers(3, 1, &mShaderConstants[ShaderConstants::MaterialParam]);
#endif
	}

	// Data
	void UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size){
		MAIN_THREAD_CHECK
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		auto buffer = mShaderConstants[type];
		mImmediateContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (mappedResource.pData){
			memcpy(mappedResource.pData, data, size);
			mImmediateContext->Unmap(buffer, 0);
		}
	}

	void UpdateComputeShaderConstants(void* data, int size) {
#if SEPERATED_THREAD_FOR_COMPUTE_SHADER				
		auto buffer = mComputeShaderConstants;
#else
		auto buffer = mShaderConstants[ShaderConstants::MaterialParam];
#endif
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mImmediateContext2->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (mappedResource.pData) {
			memcpy(mappedResource.pData, data, size);
			mImmediateContext2->Unmap(buffer, 0);
		}
	}

	void* MapShaderConstantsBuffer() const{
		MAIN_THREAD_CHECK
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mImmediateContext->Map(mShaderConstants[ShaderConstants::MaterialParam], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		return mappedResource.pData;
	}

	void UnmapShaderConstantsBuffer() const{
		MAIN_THREAD_CHECK
		mImmediateContext->Unmap(mShaderConstants[ShaderConstants::MaterialParam], 0);
	}

	void* MapBigBuffer() const{
		MAIN_THREAD_CHECK
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		mImmediateContext->Map(mShaderConstants[ShaderConstants::BigData], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		return mappedResource.pData;
	}

	void UnmapBigBuffer() const{
		MAIN_THREAD_CHECK
		mImmediateContext->Unmap(mShaderConstants[ShaderConstants::BigData], 0);
	}

	void UnbindInputLayout(){
		MAIN_THREAD_CHECK
		mImmediateContext->IASetInputLayout(0);
	}

	void UnbindShader(SHADER_TYPE shader){
		MAIN_THREAD_CHECK
		switch (shader){
		case SHADER_TYPE_VS:
			mImmediateContext->VSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_VS)] = 0;
			break;
		case SHADER_TYPE_HS:
			mImmediateContext->HSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_HS)] = 0;
			break;
		case SHADER_TYPE_DS:
			mImmediateContext->DSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_DS)] = 0;
			break;
		case SHADER_TYPE_GS:
			mImmediateContext->GSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_GS)] = 0;
			break;
		case SHADER_TYPE_PS:
			mImmediateContext->PSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_PS)] = 0;
			break;		
		}
	}

	void UnbindTexture(SHADER_TYPE shader, int slot){
		MAIN_THREAD_CHECK
		ID3D11ShaderResourceView* srv = 0;
		switch (shader){
		case SHADER_TYPE_VS:
			mImmediateContext->VSSetShaderResources(slot, 1, &srv);
			break;
		case SHADER_TYPE_HS:
			mImmediateContext->HSSetShaderResources(slot, 1, &srv);
			break;
		case SHADER_TYPE_DS:
			mImmediateContext->DSSetShaderResources(slot, 1, &srv);
			break;
		case SHADER_TYPE_GS:
			mImmediateContext->GSSetShaderResources(slot, 1, &srv);
			break;
		case SHADER_TYPE_PS:
			mImmediateContext->PSSetShaderResources(slot, 1, &srv);
			break;
		}
	}

	void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
		IPlatformTexture* src, UINT srcSubresource, Box3D* pBox) {
		MAIN_THREAD_CHECK
		assert(dst && src && dst != src);
		TextureD3D11* pDstD3D11 = static_cast<TextureD3D11*>(dst);
		TextureD3D11* pSrcD3D11 = static_cast<TextureD3D11*>(src);
		auto hardwareTextureDest = pDstD3D11->GetHardwareTexture();
		auto hardwareTextureSrc = pSrcD3D11->GetHardwareTexture();
		if (hardwareTextureDest && hardwareTextureSrc) {
			mImmediateContext->CopySubresourceRegion(hardwareTextureDest, dstSubresource,
				dstx, dsty, dstz, hardwareTextureSrc, srcSubresource, (D3D11_BOX*)pBox);
		}
		else {
			Error(FB_ERROR_LOG_ARG, "No hardware texture found.");
		}
	}

	void CopyResource(IPlatformTexture* dst, IPlatformTexture* src) {
		MAIN_THREAD_CHECK
		assert(dst && src && dst != src);
		TextureD3D11* pDstD3D11 = static_cast<TextureD3D11*>(dst);
		TextureD3D11* pSrcD3D11 = static_cast<TextureD3D11*>(src);
		auto hardwareTextureDest = pDstD3D11->GetHardwareTexture();
		auto hardwareTextureSrc = pSrcD3D11->GetHardwareTexture();
		if (hardwareTextureDest && hardwareTextureSrc) {
			mImmediateContext->CopyResource(hardwareTextureDest, hardwareTextureSrc);
		}
		else {
			Error(FB_ERROR_LOG_ARG, "No hardware texture found.");
		}
	}

	// Drawing
	void Draw(unsigned int vertexCount, unsigned int startVertexLocation) {
		MAIN_THREAD_CHECK
		mImmediateContext->Draw(vertexCount, startVertexLocation);
	}

	void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) {
		MAIN_THREAD_CHECK
		mImmediateContext->DrawIndexed(indexCount, startIndexLocation, startVertexLocation);
	}

	void Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil) {
		MAIN_THREAD_CHECK
		Clear(r, g, b, a);
		if (mCurrentDSView)
			mImmediateContext->ClearDepthStencilView(mCurrentDSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, (float)z, stencil);
	}

	void Clear(Real r, Real g, Real b, Real a) {
		MAIN_THREAD_CHECK
		float ClearColor[4] = { (float)r, (float)g, (float)b, (float)a }; // red,green,blue,alpha
		for (size_t i = 0; i<mCurrentRTViews.size(); i++)
		{
			if (mCurrentRTViews[i])
				mImmediateContext->ClearRenderTargetView(mCurrentRTViews[i], ClearColor);
		}
	}

	void ClearDepthStencil(Real z, UINT8 stencil) {
		MAIN_THREAD_CHECK
		if (mCurrentDSView)
			mImmediateContext->ClearDepthStencilView(mCurrentDSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, (float)z, stencil);
	}

	void ClearState() {
		MAIN_THREAD_CHECK
		mImmediateContext->ClearState();		
	}

	void Present() {
		for (auto& it : mSwapChains)
		{
			HRESULT hr = it.second->Present(0, mStandBy ? DXGI_PRESENT_TEST : 0);
			if (hr == DXGI_ERROR_DEVICE_REMOVED) {
				auto reason = mDevice->GetDeviceRemovedReason();
				char buf[512];
				sprintf_s(buf, "device removed reason : %d", reason);
				OutputDebugString(buf);
				mDeviceRemoved = true;
			}			
			if (hr == DXGI_STATUS_OCCLUDED) {
				mStandBy = true;
			}
			else {
				mStandBy = false;
			}
			if (!mTakeScreenshot.empty()) {
				if (!mRenderTargetTextures.empty()) {
					auto srcTexture = mRenderTargetTextures.begin()->second.first;
					Vec2I size = srcTexture->GetSize();
					auto pStaging = CreateTexture(0, size.x, size.y, srcTexture->GetPixelFormat(), 1,
						BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);
					CopyResource(pStaging.get(), srcTexture.get());					
					pStaging->SaveToFile(mTakeScreenshot.c_str());
				}
				mTakeScreenshot.clear();
			}
		}
		std::vector<std::function<void()>> queuedComputeShaders;
		{
			ENTER_CRITICAL_SECTION lock(ComputeShaderQueueLock);
			queuedComputeShaders.swap(mQueuedComputeShaders);
		}
		for (auto& f : queuedComputeShaders) {
			f();
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
		if (ValidCString(filename))
			mTakeScreenshot = filename;
	}

	ID3D11DeviceContext* GetRelatedDC(ID3D11DeviceChild* pResource) const {
		if (!pResource)
			return nullptr;
		
		ID3D11Device* device;
		pResource->GetDevice(&device);
		ID3D11DeviceContext* dc;
		if (device == mDevice.get())
			dc = mImmediateContext.get();
		else
			dc = mImmediateContext2.get();
		device->Release();
		return dc;
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
		auto dc = GetRelatedDC(pResource);
		if (!dc) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid device context.");
		}
		HRESULT hr = dc->Map(pResource, subResource, maptype,
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
		auto dc = GetRelatedDC(pResource);
		if (dc)
			dc->Unmap(pResource, subResource);
	}
	
	bool UpdateBuffer(ID3D11Resource* pResource, void* data, unsigned bytes) {
		if (!pResource) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}
		if (!data) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg2.");
			return false;
		}
		if (bytes == 0) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg3.");
			return false;
		}
		MAIN_THREAD_CHECK
		mImmediateContext->UpdateSubresource(pResource, 0,
			0, data, bytes, 0);
		return true;
	}

	const GUID& GetWICCodecFromExt(const char* ext) {
		using namespace DirectX;
		if (_stricmp(ext, ".bmp") == 0) {
			return GetWICCodec(WIC_CODEC_BMP);
		}
		else if (_stricmp(ext, ".jpg") == 0) {
			return GetWICCodec(WIC_CODEC_JPEG);
		}
		else if (_stricmp(ext, ".png") == 0) {
			return GetWICCodec(WIC_CODEC_PNG);
		}
		else if (_stricmp(ext, ".tif") == 0) {
			return GetWICCodec(WIC_CODEC_TIFF);
		}
		else if (_stricmp(ext, ".gif") == 0) {
			return GetWICCodec(WIC_CODEC_GIF);
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, "Unsupported format.");
			return GetWICCodec(WIC_CODEC_BMP);
		}		
	}

	void SaveTextureToFile(TextureD3D11* texture, const char* filename)
	{
		MAIN_THREAD_CHECK
		using namespace DirectX;
		TextureD3D11* pTextureD3D11 = static_cast<TextureD3D11*>(texture);
		if (pTextureD3D11)
		{
			const char* ext = FileSystem::GetExtension(filename);
			if (_stricmp(ext, ".bmp") == 0 ||
				_stricmp(ext, ".jpg") == 0 ||
				_stricmp(ext, ".png") == 0 ||
				_stricmp(ext, ".tif") == 0 ||
				_stricmp(ext, ".gif") == 0)
			{
				SaveWICTextureToFile(mImmediateContext.get(), pTextureD3D11->GetHardwareTexture(),
					GetWICCodecFromExt(ext), AnsiToWideMT(filename).c_str());
			}
			else if (_stricmp(ext, ".dds") == 0)
			{
				bool cubemap = false;
				auto textureResource = pTextureD3D11->GetHardwareTexture();
				D3D11_RESOURCE_DIMENSION dimention;
				textureResource->GetType(&dimention);
				D3D11_TEXTURE2D_DESC desc;
				ID3D11Texture2D* texture2D = 0;
				if (dimention == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
					if ( FAILED(textureResource->QueryInterface(IID_ID3D11Texture2D, (void**)&texture2D)) ) {
						Logger::Log(FB_ERROR_LOG_ARG, "QueryInterface failed.");
						return;
					}					
					texture2D->GetDesc(&desc);
					cubemap = desc.ArraySize == 6;
					texture2D->Release();
				}
				if (!cubemap) {
					SaveDDSTextureToFile(mImmediateContext.get(), pTextureD3D11->GetHardwareTexture(), AnsiToWideMT(filename).c_str());
				}
				else {
					auto pStaging = CreateTexture(0, desc.Width, desc.Height, (PIXEL_FORMAT)desc.Format, desc.MipLevels, BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_CUBE_MAP);
					if (!pStaging) {
						Logger::Log(FB_ERROR_LOG_ARG, "Failed to create staging texture for the cubemap.");
						return;
					}
					TextureD3D11* stagingTexture = (TextureD3D11*)pStaging.get();
					mImmediateContext->CopyResource(stagingTexture->GetHardwareTexture(), textureResource);
					DirectX::ScratchImage scratchImage;
					scratchImage.InitializeCube(desc.Format, desc.Width, desc.Height, 1, desc.MipLevels);
					UINT subResource = 0;					
					for (int a = 0; a < 6; ++a) {						
						for (int m = 0; m < (int)desc.MipLevels; ++m) {														
							auto map = stagingTexture->Map(subResource, MAP_TYPE_READ, MAP_FLAG_NONE);
							if (map.pData) {
								auto image = scratchImage.GetImage(m, a, 0);								
								memcpy(image->pixels, map.pData, image->slicePitch);								
								stagingTexture->Unmap(subResource);
							}							
							++subResource;
						}									
					}
					SaveToDDSFile(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::DDS_FLAGS_NONE, AnsiToWideMT(filename).c_str());
				}
				
			}
			else
			{
				Error("Unsupported file format!");
				return;
			}
		}
	}

	void GenerateMips(TextureD3D11* pTexture){		
		MAIN_THREAD_CHECK
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
		MAIN_THREAD_CHECK
		ID3D11Buffer* pHardwareBuffer = pIndexBuffer->GetHardwareBuffer();
		mImmediateContext->IASetIndexBuffer(pHardwareBuffer, pIndexBuffer->GetFormatD3D11(), offset);
	}

	void SetTexture(TextureD3D11* pTexture, SHADER_TYPE shaderType, unsigned int slot){
		ID3D11ShaderResourceView* const pSRV = pTexture ? pTexture->GetHardwareResourceView() : 0;		
		try
		{
			switch (shaderType)
			{
			case SHADER_TYPE_VS:
				MAIN_THREAD_CHECK
				mImmediateContext->VSSetShaderResources(slot, 1, &pSRV);
				break;
			case SHADER_TYPE_GS:
				MAIN_THREAD_CHECK
				mImmediateContext->GSSetShaderResources(slot, 1, &pSRV);
				break;
			case SHADER_TYPE_PS:
				MAIN_THREAD_CHECK
				mImmediateContext->PSSetShaderResources(slot, 1, &pSRV);
				break;
			case SHADER_TYPE_CS: {
				ID3D11Device* device;
				pSRV->GetDevice(&device);
				if (device  == mDevice2.get())
					mImmediateContext2->CSSetShaderResources(slot, 1, &pSRV);
				else
					Logger::Log(FB_ERROR_LOG_ARG, "Incorrect device.");
				device->Release();
				break;
			}
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

	void SetShader(VertexShaderD3D11* pShader){
		MAIN_THREAD_CHECK
		if (pShader == 0 || pShader->GetCompileFailed())
		{
			mCurrentShaders[ShaderIndex(SHADER_TYPE_VS)] = 0;
			mImmediateContext->VSSetShader(0, 0, 0);
			return;
		}		

		ID3D11VertexShader* pVS = pShader->GetVertexShader();
		mCurrentShaders[ShaderIndex(SHADER_TYPE_VS)] = pShader;
		mImmediateContext->VSSetShader(pVS, 0, 0);		
	}

	/*void SetHSShader(ShaderD3D11* pShader){
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
	}*/

	void SetShader(GeometryShaderD3D11* pShader){
		MAIN_THREAD_CHECK
		if (pShader == 0 || pShader->GetCompileFailed())
		{
			mImmediateContext->GSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_GS)] = 0;
			return;
		}

		ID3D11GeometryShader* pGS = pShader->GetGeometryShader();
		mCurrentShaders[ShaderIndex(SHADER_TYPE_GS)] = pShader;
		mImmediateContext->GSSetShader(pGS, 0, 0);		
	}

	void SetShader(PixelShaderD3D11* pShader){
		MAIN_THREAD_CHECK
		if (pShader == 0 || pShader->GetCompileFailed())
		{
			mImmediateContext->PSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_PS)] = 0;
			return;
		}

		ID3D11PixelShader* pPS = pShader->GetPixelShader();
		mCurrentShaders[ShaderIndex(SHADER_TYPE_PS)] = pShader;
		mImmediateContext->PSSetShader(pPS, 0, 0);
	}

	void SetShader(ComputeShaderD3D11* pShader) {
		if (pShader == 0 || pShader->GetCompileFailed())
		{
			mImmediateContext2->CSSetShader(0, 0, 0);
			mCurrentShaders[ShaderIndex(SHADER_TYPE_PS)] = 0;
			return;
		}

		auto pCS = pShader->GetComputeShader();
		mCurrentShaders[ShaderIndex(SHADER_TYPE_CS)] = pShader;
		mImmediateContext2->CSSetShader(pCS, 0, 0);
	}

	HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
	{
		D3D11_BUFFER_DESC descBuf;
		ZeroMemory(&descBuf, sizeof(descBuf));
		pBuffer->GetDesc(&descBuf);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.BufferEx.FirstElement = 0;

		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			// This is a Raw Buffer

			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
		}
		else
			if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// This is a Structured Buffer

				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
			}
			else
			{
				return E_INVALIDARG;
			}

		return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
	}

	HRESULT CreateRawBuffer(ID3D11Device* pDevice, UINT uSize, VOID* pInitData, ID3D11Buffer** ppBufOut)
	{
		*ppBufOut = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER | D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = uSize;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

		if (pInitData)
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
		}
		else
			return pDevice->CreateBuffer(&desc, NULL, ppBufOut);
	}

	HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
	{
		D3D11_BUFFER_DESC descBuf;
		ZeroMemory(&descBuf, sizeof(descBuf));
		pBuffer->GetDesc(&descBuf);

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;

		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			// This is a Raw Buffer

			desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			desc.Buffer.NumElements = descBuf.ByteWidth / 4;
		}
		else
			if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
			{
				// This is a Structured Buffer

				desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
				desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
			}
			else
			{
				return E_INVALIDARG;
			}

		return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
	}

	ID3D11Buffer* CreateStagingBuffer(ID3D11Device* pDevice, ID3D11Buffer* pBuffer)
	{
		ID3D11Buffer* debugbuf = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		pBuffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		if (SUCCEEDED(pDevice->CreateBuffer(&desc, NULL, &debugbuf)))
		{
			return debugbuf;
		}

		return nullptr;
	}

	bool RunComputeShader(ComputeShaderD3D11* pShader, void* constants, size_t size,
		int x, int y, int z,
		ByteArray& output, size_t outputSize) 
	{
		if (!pShader) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return false;
		}
		if (!mDevice2) {
			return false;
		}
		auto cs = pShader->GetComputeShader();
		if (!cs || pShader->GetCompileFailed()) {
			return false;
		}
		mImmediateContext2->CSSetShader(cs, 0, 0);
		if (constants && size>0) {
			UpdateComputeShaderConstants(constants, size);
		}
		if (!mComputeShaderResult || mComputeShaderResultSize < outputSize) {
			ID3D11Buffer* buffer;
			auto hr = CreateRawBuffer(mDevice2.get(), outputSize, NULL, &buffer);
			if (FAILED(hr)) {
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create buffer.");
				return false;
			}
			mComputeShaderResult = ID3D11BufferPtr(buffer, IUnknownDeleter());

			ID3D11Buffer* stageBuffer = CreateStagingBuffer(mDevice2.get(), buffer);
			if (!stageBuffer) {
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create staging buffer");
				return false;
			}
			mComputeShaderResultStage = ID3D11BufferPtr(stageBuffer, IUnknownDeleter());
			
			ID3D11UnorderedAccessView* uav;
			hr = CreateBufferUAV(mDevice2.get(), buffer, &uav);
			if (FAILED(hr)) {
				Logger::Log(FB_ERROR_LOG_ARG, "Failed to create UAV.");
				return false;
			}
			mComputeShaderResultUAV = ID3D11UnorderedAccessViewPtr(uav, IUnknownDeleter());
		}	
		if (!mComputeShaderResultUAV || !mComputeShaderResultStage) {
			return false;
		}
		ID3D11UnorderedAccessView* uavs[] = { mComputeShaderResultUAV.get() };
		mImmediateContext2->CSSetUnorderedAccessViews(0, 1, uavs, 0);
		mImmediateContext2->Dispatch(x, y, z);
		mImmediateContext2->CSSetShader(NULL, NULL, 0);
		mImmediateContext2->CopyResource(mComputeShaderResultStage.get(), mComputeShaderResult.get());
		
		D3D11_MAPPED_SUBRESOURCE MappedResource;		
		mImmediateContext2->Map(mComputeShaderResultStage.get(), 0, D3D11_MAP_READ, 0, &MappedResource);
		if (MappedResource.pData) {
			output.resize(outputSize);
			memcpy(&output[0], MappedResource.pData, outputSize);
			mImmediateContext2->Unmap(mComputeShaderResultStage.get(), 0);
			return true;
		}
		Logger::Log(FB_ERROR_LOG_ARG, "Map failed.");
		return false;
	}

	CriticalSection ComputeShaderQueueLock;
	std::vector<std::function<void()>> mQueuedComputeShaders;
	bool QueueRunComputeShader(ComputeShaderD3D11* pShader, void* constants, size_t size,
		int x, int y, int z,
		std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&& callback)
	{
		ByteArray constantsData(size);
		memcpy(&constantsData[0], constants, size);
		auto self = this;
		ENTER_CRITICAL_SECTION lock(ComputeShaderQueueLock);
		if (mQuit)
			return false;
		mQueuedComputeShaders.push_back(
			[pShader, constantsData, x, y, z, output, outputSize, callback, self]() {
			auto success = self->RunComputeShader(pShader, (void*)&constantsData[0], constantsData.size(), x, y, z, *output, outputSize);
			if (success) {
				callback();
			}
		}
		);
		return true;
	}

	void SetInputLayout(InputLayoutD3D11* pInputLayout){
		MAIN_THREAD_CHECK
		ID3D11InputLayout* pLayout = pInputLayout->GetHardwareInputLayout();
		assert(pLayout);
		mImmediateContext->IASetInputLayout(pLayout);
	}

	void SetRasterizerState(RasterizerStateD3D11* pRasterizerState){
		MAIN_THREAD_CHECK
		mImmediateContext->RSSetState(pRasterizerState->GetHardwareRasterizerState());
	}

	void SetBlendState(BlendStateD3D11* pBlendState){		
		MAIN_THREAD_CHECK
		mImmediateContext->OMSetBlendState(pBlendState->GetHardwareBlendState(),
			pBlendState->GetBlendFactor(), pBlendState->GetSampleMask());
	}

	void SetDepthStencilState(DepthStencilStateD3D11* pDepthStencilState, int stencilRef){
		MAIN_THREAD_CHECK
		mImmediateContext->OMSetDepthStencilState(pDepthStencilState->GetHardwareDSState(),
			(unsigned)stencilRef);
	}

	void SetSamplerState(SamplerStateD3D11* pSamplerState, SHADER_TYPE shader, int slot){
		MAIN_THREAD_CHECK
		ID3D11SamplerState* pSS = pSamplerState->GetHardwareSamplerState();
		switch (shader)
		{
		case SHADER_TYPE_VS:
			mImmediateContext->VSSetSamplers(slot, 1, &pSS);
			break;
		case SHADER_TYPE_PS:
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
		color->SetPixelFormat(ConvertEnumD3D11(mColorFormat));

		depth = std::dynamic_pointer_cast<TextureD3D11>(
			CreateTexture(0, size.x, size.y, mDepthStencilFormat, 1, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL));
		return true;
	}

	bool ResizeSwapChain(HWindowId hwndId, const Vec2I& resol, TextureD3D11Ptr& outColor, TextureD3D11Ptr& outDepth){
		MAIN_THREAD_CHECK
		mImmediateContext->OMSetRenderTargets(0, 0, 0);
		mCurrentRTViews.clear();
		mCurrentDSView = 0;
		Vec2I originalSize;
		// release render target textures
		auto it = mRenderTargetTextures.find(hwndId);
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
		auto itSwapChain = mSwapChains.find(hwndId);
		TextureD3D11Ptr color, depth;
		if (itSwapChain != mSwapChains.end()) {
			BOOL fullscreen;
			itSwapChain->second->GetFullscreenState(&fullscreen, 0);
			auto hr = itSwapChain->second->ResizeBuffers(1, resol.x, resol.y, ConvertEnumD3D11(mColorFormat), 0);
			if (!SUCCEEDED(hr)){
				Error("Resizing swapchain to %dx%d is failed(0x%x", resol.x, resol.y, hr);	
				bool successful = CreateTargetTexturesFor(itSwapChain->second.get(), originalSize, color, depth);
				if (!successful){
					Error("Failed to recover original size of render target textures.");
					return false;
				}
				else{
					Logger::Log(FB_DEFAULT_LOG_ARG, "Swapchain is recovered to the original size.");
				}
				mRenderTargetTextures[hwndId] = { color, depth };
				outColor = color;
				outDepth = depth;
				return true;
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
	mImpl = 0;
	auto handle = GetModuleHandle("Dxgidebug.dll");
	if (handle) {
		typedef HRESULT(__stdcall *fPtr)(const IID&, void**);
		auto proc = (fPtr)GetProcAddress(handle, "DXGIGetDebugInterface");
		IDXGIDebug* pDebug;
		proc(__uuidof(IDXGIDebug), (void**)&pDebug);
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
		SAFE_RELEASE(pDebug);
	}
}

void RendererD3D11::RegisterThreadIdConsideredMainThread(std::thread::id threadId) {
	mImpl->mMainThreadId2 = threadId;
}

void RendererD3D11::PrepareQuit() {
	mImpl->PrepareQuit();
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

bool RendererD3D11::InitCanvas(const CanvasInitInfo& info,	IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) {
	return mImpl->InitCanvas(info, outColorTexture, outDepthTexture);
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

bool RendererD3D11::IsDeviceRemoved() const {
	return mImpl->mDeviceRemoved;
}

bool RendererD3D11::IsFullscreen() const {
	return mImpl->IsFullscreen();
}

// Resource creation
void RendererD3D11::SetShaderCacheOption(bool useShaderCache, bool generateCache) {
	mImpl->SetShaderCacheOption(useShaderCache, generateCache);
}

IPlatformTexturePtr RendererD3D11::CreateTexture(const char* path, const TextureCreationOption& options) {
	return mImpl->CreateTexture(path, options);
}

IPlatformTexturePtr RendererD3D11::CreateTexture(void* data, int width, int height, PIXEL_FORMAT format, int numMips, BUFFER_USAGE usage, int  buffer_cpu_access,	int texture_type) {
	return mImpl->CreateTexture(data, width, height, format, numMips, usage, buffer_cpu_access, texture_type);
}

IPlatformVertexBufferPtr RendererD3D11::CreateVertexBuffer(void* data, unsigned stride,	unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) {
	return mImpl->CreateVertexBuffer(data, stride, numVertices, usage, accessFlag);
}

IPlatformIndexBufferPtr RendererD3D11::CreateIndexBuffer(void* data, unsigned int numIndices,	INDEXBUFFER_FORMAT format) {
	return mImpl->CreateIndexBuffer(data, numIndices, format);
}

IPlatformShaderPtr RendererD3D11::CreateVertexShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache) 
{
	return mImpl->CreateVertexShader(path, defines, ignoreCache);
}

IPlatformShaderPtr RendererD3D11::CreateGeometryShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache)
{
	return mImpl->CreateGeometryShader(path, defines, ignoreCache);
}

IPlatformShaderPtr RendererD3D11::CreatePixelShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache)
{
	return mImpl->CreatePixelShader(path, defines, ignoreCache);
}

IPlatformShaderPtr RendererD3D11::CreateComputeShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache)
{
	return mImpl->CreateComputeShader(path, defines, ignoreCache);
}

IPlatformShaderPtr RendererD3D11::CompileComputeShader(const char* code, const char* entry, 
	const SHADER_DEFINES& defines) {
	return mImpl->CompileComputeShader(code, entry, defines);
}

bool RendererD3D11::ReloadShader(ShaderD3D11* shader, const SHADER_DEFINES& defines) {
	return mImpl->ReloadShader(shader, defines);
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

unsigned RendererD3D11::GetNumLoadingTexture() const
{
	return mImpl->GetNumLoadingTexture();
}

// Resource Binding
void RendererD3D11::SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,	IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) {
	mImpl->SetRenderTarget(pRenderTargets, rtViewIndex, num, pDepthStencil, dsViewIndex);
}

void RendererD3D11::SetDepthTarget(IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) {
	mImpl->SetDepthTarget(pDepthStencil, dsViewIndex);
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

void RendererD3D11::SetTextures(IPlatformTexturePtr pTextures[], int num, SHADER_TYPE shaderType, int startSlot) {
	mImpl->SetTextures(pTextures, num, shaderType, startSlot);
}

void RendererD3D11::UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size) {
	mImpl->UpdateShaderConstants(type, data, size);
}

void* RendererD3D11::MapShaderConstantsBuffer() const {
	return mImpl->MapShaderConstantsBuffer();
}

void RendererD3D11::UnmapShaderConstantsBuffer() const {
	mImpl->UnmapShaderConstantsBuffer();
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

void RendererD3D11::UnbindShader(SHADER_TYPE shader) {
	mImpl->UnbindShader(shader);
}

void RendererD3D11::UnbindTexture(SHADER_TYPE shader, int slot) {
	mImpl->UnbindTexture(shader, slot);
}

void RendererD3D11::CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,	IPlatformTexture* src, UINT srcSubresource, Box3D* pBox) {
	mImpl->CopyToStaging(dst, dstSubresource, dstx, dsty, dstz, src, srcSubresource, pBox);
}

void RendererD3D11::CopyResource(IPlatformTexture* dst, IPlatformTexture* src) {
	mImpl->CopyResource(dst, src);
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

void RendererD3D11::ClearDepthStencil(Real z, UINT8 stencil) {
	mImpl->ClearDepthStencil(z, stencil);
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

bool RendererD3D11::UpdateBuffer(ID3D11Resource* pResource, void* data, unsigned bytes) {
	return mImpl->UpdateBuffer(pResource, data, bytes);
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

void RendererD3D11::SetTexture(TextureD3D11* pTexture, SHADER_TYPE shaderType, unsigned int slot) {
	mImpl->SetTexture(pTexture, shaderType, slot);
}

void RendererD3D11::SetShader(VertexShaderD3D11* pShader) {
	mImpl->SetShader(pShader);
}

void RendererD3D11::SetShader(GeometryShaderD3D11* pShader) {
	mImpl->SetShader(pShader);
}

void RendererD3D11::SetShader(PixelShaderD3D11* pShader) {
	mImpl->SetShader(pShader);
}

void RendererD3D11::SetShader(ComputeShaderD3D11* pShader) {
	mImpl->SetShader(pShader);
}

bool RendererD3D11::RunComputeShader(ComputeShaderD3D11* pShader, void* constants, size_t size,
	int x, int y, int z,
	ByteArray& output, size_t outputSize) 
{
	return mImpl->RunComputeShader(pShader, constants, size, x, y, z, output, outputSize);
}

bool RendererD3D11::QueueRunComputeShader(ComputeShaderD3D11* pShader, void* constants, size_t size,
	int x, int y, int z, std::shared_ptr<ByteArray> output, size_t outputSize, std::function<void()>&& callback) {
	return mImpl->QueueRunComputeShader(pShader, constants, size, x, y, z, output, outputSize, std::move(callback));
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

void RendererD3D11::SetDepthStencilState(DepthStencilStateD3D11* pDepthStencilState, int stencilRef) {
	mImpl->SetDepthStencilState(pDepthStencilState, stencilRef);
}

void RendererD3D11::SetSamplerState(SamplerStateD3D11* pSamplerState, SHADER_TYPE shader, int slot) {
	mImpl->SetSamplerState(pSamplerState, shader, slot);
}

static DirectX::TexMetadata GetMetadata(const char* path) {
	DirectX::TexMetadata metadata{};
	FileSystem::Open file(path, "rb");
	if (!file.IsOpen()) {
		Logger::Log(FB_ERROR_LOG_ARG, FormatString(
			"Cannot open file %s", path).c_str());
	}
	auto& data = *file.GetBinaryData();
	
	auto ret = DirectX::GetMetadataFromWICMemory(&data[0], data.size(), 0, metadata);
	if (SUCCEEDED(ret)) {
		return metadata;
	}
	ret = DirectX::GetMetadataFromDDSMemory(&data[0], data.size(), 0, metadata);
	if (SUCCEEDED(ret)) {
		return metadata;
	}
	Logger::Log(FB_ERROR_LOG_ARG, FormatString(
		"Failed to read metadata from %s", path).c_str());
	return metadata;	
}

static ScratchImagePtr LoadScratchImage(const char* path, bool generateMip, DirectX::TexMetadata& metadata) {
	using namespace DirectX;
	ByteArray image;
	{
		FileSystem::Lock l;
		image = FileSystem::ReadBinaryFile(path);
	}
	if (image.empty()) {
		return nullptr;
	}
	auto ret = GetMetadataFromDDSMemory(&image[0], image.size(), DDS_FLAGS_NONE, metadata);
	bool dds = true;
	if (FAILED(ret)) {
		dds = false;		
		ret = GetMetadataFromWICMemory(&image[0], image.size(), WIC_FLAGS_NONE, metadata);
		if (FAILED(ret)) {
			Logger::Log(FB_DEFAULT_LOG_ARG, "DDS and WIC load failed. image format is not supported.");
			return nullptr;
		}
	}

	
	ScratchImagePtr scratchImg = std::make_shared<ScratchImage>();	
	if (dds) {
		ret = LoadFromDDSMemory(&image[0], image.size(), DDS_FLAGS_NONE, &metadata, *scratchImg);
		if (FAILED(ret)) {
			Logger::Log(FB_ERROR_LOG_ARG, "LoadFromDDSMemory failed.");
			return nullptr;
		}		
	}
	else {
		auto ret = LoadFromWICMemory(&image[0], image.size(), WIC_FLAGS_NONE, &metadata, *scratchImg);
		if (FAILED(ret)) {
			Logger::Log(FB_ERROR_LOG_ARG, "LoadFromWICMemory failed.");
			return nullptr;
		}		
	}	
	if (generateMip && scratchImg->GetMetadata().mipLevels == 1) {
		if (DirectX::IsCompressed(scratchImg->GetMetadata().format)) {
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString(
				"Image(%s) has compressed foramt. Cannot generate mipmaps.", path).c_str());
		}
		else {
			auto mipmappedScratchTexture = std::make_shared<DirectX::ScratchImage>();
			auto ret = DirectX::GenerateMipMaps(scratchImg->GetImages(), scratchImg->GetImageCount(),
				scratchImg->GetMetadata(), 0, 0, *mipmappedScratchTexture);
			if (SUCCEEDED(ret)) {
				scratchImg = mipmappedScratchTexture;
			}
			else {
				Logger::Log(FB_ERROR_LOG_ARG, FormatString(
					"Cannot generate mips for Texture(%s).", path).c_str());
			}
		}
	}
	return scratchImg;
}

static ScratchImagePtr ConvertScratchImage(const ScratchImagePtr& srcImage) {
	auto& metadata = srcImage->GetMetadata();
	if (metadata.format == DXGI_FORMAT_B8G8R8A8_UNORM) {
		auto newScratchImage = std::make_shared<DirectX::ScratchImage>();
		auto ret = Convert(srcImage->GetImages(), srcImage->GetImageCount(), srcImage->GetMetadata(),
			DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 0.5f, *newScratchImage);
		if (SUCCEEDED(ret))
			return newScratchImage;
		else {
			Logger::Log(FB_ERROR_LOG_ARG, "Format conversion failed.");
			return srcImage;
		}
	}
	return srcImage;
}