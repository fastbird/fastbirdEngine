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
#include "Renderer.h"
#include "IPlatformRenderer.h"
#include "NullPlatformRenderer.h"
#include "RendererEnums.h"
#include "RendererStructs.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "IPlatformTexture.h"
#include "Font.h"
#include "RendererOptions.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "TextureAtlas.h"
#include "PointLightManager.h"
#include "DebugHud.h"
#include "GeometryRenderer.h"
#include "RenderStates.h"
#include "IVideoPlayer.h"
#include "ResourceProvider.h"
#include "ResourceTypes.h"
#include "Camera.h"
#include "ConsoleRenderer.h"
#include "StarDef.h"
#include "FBMathLib/MurmurHash.h"
#include "FBConsole/Console.h"
#include "EssentialEngineData/shaders/Constants.h"
#include "FBStringLib/StringConverter.h"
#include "FBStringLib/StringLib.h"
#include "FBStringMathLib/StringMathConverter.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBCommonHeaders/Factory.h"
#include "FBSystemLib/ModuleHandler.h"
#include "FBSystemLib/System.h"
#include "FBFileSystem/FileSystem.h"
#include "FBSceneManager/IScene.h"
#include "FBInputManager/IInputInjector.h"
#include "FBLua/LuaObject.h"
#include "TinyXmlLib/tinyxml2.h"
#include "FBTimer/Timer.h"
#include <set>
#undef DrawText
#undef CreateDirectory
namespace fb{
	ShaderPtr GetShaderFromExistings(IPlatformShaderPtr platformShader);
	TexturePtr GetTextureFromExistings(IPlatformTexturePtr platformTexture);
	FB_DECLARE_SMART_PTR(UI3DObj);
	FB_DECLARE_SMART_PTR(UIObject);
}
using namespace fb;

static const float defaultFontSize = 20.f;
const HWindow Renderer::INVALID_HWND = (HWindow)-1;
Timer* fb::gpTimer = 0;
BinaryData tempData;
unsigned tempSize;
class Renderer::Impl{
public:
	typedef fb::Factory<IPlatformRenderer>::CreateCallback CreateCallback;
	typedef std::vector<RenderTargetWeakPtr> RenderTargets;

	RendererWeakPtr mSelf;
	Renderer* mObject;	

	std::string mPlatformRendererType;
	struct PlatformRendererHolder{
		IPlatformRenderer* mPlatformRenderer;
		ModuleHandle mLoadedModule;

		PlatformRendererHolder(IPlatformRenderer* platformRenderer, ModuleHandle module)
			: mPlatformRenderer(platformRenderer)
			, mLoadedModule(module)
		{
		}

		~PlatformRendererHolder(){
			typedef void(*Destroy)();
			Destroy DestroyFunction = (Destroy)ModuleHandler::GetFunction(mLoadedModule, "DeleteRenderEngine");
			if (DestroyFunction){
				DestroyFunction();
			}
			ModuleHandler::UnloadModule(mLoadedModule);
		}		
		operator IPlatformRenderer* () const { return mPlatformRenderer; }
	};
	std::shared_ptr<PlatformRendererHolder> mPlatformRenderer;
	IPlatformRendererPtr mNullRenderer;
	

	VectorMap<HWindowId, HWindow> mWindowHandles;
	VectorMap<HWindow, HWindowId> mWindowIds;
	VectorMap<HWindowId, Vec2I> mWindowSizes;
	HWindowId mMainWindowId;	
	VectorMap<HWindowId, RenderTargetPtr> mWindowRenderTargets;
	RenderTargetPtr mCurrentRenderTarget;
	std::vector<TexturePtr> mCurrentRTTextures;
	std::vector<size_t> mCurrentViewIndices;
	TexturePtr mCurrentDSTexture;
	size_t mCurrentDSViewIndex;
	Vec2I mCurrentRTSize;
	std::vector<RenderTargetPtr> mRenderTargetPool;
	RenderTargets mRenderTargets;
	RenderTargets mRenderTargetsEveryFrame;	
	ISceneWeakPtr mCurrentScene;
	CameraPtr mCamera;
	CameraPtr mCameraBackup;
	struct InputInfo{
		Vec2I mCurrentMousePos;
		bool mLButtonDown;

		InputInfo()
			:mCurrentMousePos(0, 0)
			, mLButtonDown(false)
		{}
	};
	InputInfo mInputInfo;
	
	VectorMap<Vec2I, TexturePtr> mTempDepthBuffers;
	TexturePtr mEnvironmentTexture;
	TexturePtr mEnvironmentTextureOverride;
	VectorMap<SystemTextures::Enum, std::vector< TextureBinding > > mSystemTextureBindings;
	FRAME_CONSTANTS			mFrameConstants;
	CAMERA_CONSTANTS		mCameraConstants;
	RENDERTARGET_CONSTANTS	mRenderTargetConstants;
	SCENE_CONSTANTS			mSceneConstants;

	DirectionalLightInfo	mDirectionalLight[2];
	bool mRefreshPointLight;
	PointLightManagerPtr mPointLightMan;
	VectorMap<int, FontPtr> mFonts;
	DebugHudPtr		mDebugHud;
	GeometryRendererPtr mGeomRenderer;	
	RendererOptionsPtr mRendererOptions;
	bool mForcedWireframe;
	RENDERER_FRAME_PROFILER mFrameProfiler;
	PRIMITIVE_TOPOLOGY mCurrentTopology;	
	const int DEFAULT_DYN_VERTEX_COUNTS=100;
	VertexBufferPtr mDynVBs[DEFAULT_INPUTS::COUNT];
	INPUT_ELEMENT_DESCS mInputLayoutDescs[DEFAULT_INPUTS::COUNT];
	ResourceProviderPtr mResourceProvider;

	// 1/4
	// x, y,    offset, weight;
	VectorMap< std::pair<DWORD, DWORD>, std::pair<std::vector<Vec4f>, std::vector<Vec4f> > > mGauss5x5;	
	InputLayoutPtr mPositionInputLayout;
	ConsoleRendererPtr mConsoleRenderer;

	struct DebugRenderTarget
	{
		Vec2 mPos;
		Vec2 mSize;

		TexturePtr mTexture;
	};
	static const unsigned MaxDebugRenderTargets = 4;
	DebugRenderTarget mDebugRenderTargets[MaxDebugRenderTargets];	
	bool mLuminanceOnCpu;
	bool mUseFilmicToneMapping;
	bool m3DUIEnabled;
	Real mLuminance;
	unsigned mFrameLuminanceCalced;
	Real mFadeAlpha;

	typedef VectorMap<HWindowId, std::vector<UIObjectPtr> > UI_OBJECTS;
	UI_OBJECTS mUIObjectsToRender;
	typedef VectorMap< std::pair<HWindowId, std::string>, std::vector<UIObjectPtr>> UI_3DOBJECTS;
	UI_3DOBJECTS mUI3DObjects;
	VectorMap<std::string, RenderTargetPtr> mUI3DObjectsRTs;
	VectorMap<std::string, UI3DObjPtr> mUI3DRenderObjs;	
	std::vector<IVideoPlayerPtr> mVideoPlayers;
	INT64 mLastRenderedTime;
	unsigned mMainWindowStyle;
	bool mWindowSizeInternallyChanging;
	
	//-----------------------------------------------------------------------
	Impl(Renderer* renderer)
		: mObject(renderer)
		, mNullRenderer(NullPlatformRenderer::Create())
		, mCurrentTopology(PRIMITIVE_TOPOLOGY_UNKNOWN)
		, mForcedWireframe(false)
		, mUseFilmicToneMapping(true)
		, mFadeAlpha(0.)
		, mLuminance(0.5f)
		, mLuminanceOnCpu(false)
		, mLastRenderedTime(0)
		, mWindowSizeInternallyChanging(false)
		, mConsoleRenderer(ConsoleRenderer::Create())
		, mRendererOptions(RendererOptions::Create())
		, mPointLightMan(PointLightManager::Create())
		, mMainWindowStyle(0)
	{
		auto filepath = "_FBRenderer.log";
		FileSystem::BackupFile(filepath, 5, "Backup_Log");
		Logger::Init(filepath);

		gpTimer = Timer::GetMainTimer().get();
		mLastRenderedTime = gpTimer->GetTickCount();
		auto& envBindings = mSystemTextureBindings[SystemTextures::Environment];
		envBindings.push_back(TextureBinding{ BINDING_SHADER_PS, 4 });
		auto& depthBindings = mSystemTextureBindings[SystemTextures::Depth];
		depthBindings.push_back(TextureBinding{ BINDING_SHADER_GS, 5 });
		depthBindings.push_back(TextureBinding{ BINDING_SHADER_PS, 5 });
		auto& cloudBindings = mSystemTextureBindings[SystemTextures::CloudVolume];
		cloudBindings.push_back(TextureBinding{BINDING_SHADER_PS, 6});
		auto& noiseBindings = mSystemTextureBindings[SystemTextures::Noise];
		noiseBindings.push_back(TextureBinding{ BINDING_SHADER_PS, 7 });
		auto& shadowBindings = mSystemTextureBindings[SystemTextures::ShadowMap];
		shadowBindings.push_back(TextureBinding{ BINDING_SHADER_PS, 8 });
		auto& ggxBindings = mSystemTextureBindings[SystemTextures::GGXPrecalc];
		ggxBindings.push_back(TextureBinding{ BINDING_SHADER_PS, 9 });

		if (Console::HasInstance()){
			Console::GetInstance().AddObserver(ICVarObserver::Default, mRendererOptions);
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "The console is not initialized!");
		}
	}

	~Impl(){
		StarDef::FinalizeStatic();
		Logger::Release();
	}	

	bool PrepareRenderEngine(const char* type){
		if (!type || strlen(type) == 0){
			Logger::Log(FB_DEFAULT_LOG_ARG, "Cannot prepare a render engine : invalid arg.");
			return false;
		}
		if (mPlatformRenderer){
			Logger::Log(FB_DEFAULT_LOG_ARG, "Render engine is already prepared.");
			return true;
		}

		mPlatformRendererType = type;		
		auto module = ModuleHandler::LoadModule(mPlatformRendererType.c_str());
		if (module){
			typedef fb::IPlatformRenderer*(*Create)();
			Create createCallback = (Create)ModuleHandler::GetFunction(module, "CreateRenderEngine");
			if (createCallback){
				auto platformRenderer = createCallback();
				if (platformRenderer){
					mPlatformRenderer = std::shared_ptr<PlatformRendererHolder>(
						new PlatformRendererHolder(platformRenderer, module), 
						[](PlatformRendererHolder* obj){delete obj; });
					Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Render engine %s is prepared.", type).c_str());
					return true;
				}
				else{
					Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot create a platform renderer(%s)", mPlatformRendererType.c_str()).c_str());
				}
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, "Cannot find the entry point 'CreateRenderEngine()'");
			}
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to load a platform renderer module(%s)", mPlatformRendererType.c_str()).c_str());
		}
		return false;
	}

	IPlatformRenderer& GetPlatformRenderer() const {
		if (!mPlatformRenderer)
		{
			return *mNullRenderer.get();

		}

		return *mPlatformRenderer->mPlatformRenderer;
	}

	bool InitCanvas(HWindowId id, HWindow window, int width, int height){
		bool mainCanvas = false;
		if (mWindowHandles.empty()){
			mMainWindowId = id;
			mainCanvas = true;
		}

		mWindowHandles[id] = window;
		mWindowIds[window] = id;
		if (width == 0 || height == 0){
			width = mRendererOptions->r_resolution.x;
			height = mRendererOptions->r_resolution.y;
		}
		mWindowSizes[id] = { width, height };		
		IPlatformTexturePtr colorTexture;
		IPlatformTexturePtr depthTexture;
		GetPlatformRenderer().InitCanvas(id, window, width, height, false, colorTexture, depthTexture);
		if (colorTexture && depthTexture){
			RenderTargetParam param;
			param.mSize = { width, height };
			param.mWillCreateDepth = true;
			auto rt = CreateRenderTarget(param);	
			rt->SetColorTexture(CreateTexture(colorTexture));
			rt->SetDepthTexture(CreateTexture(depthTexture));
			
			mWindowRenderTargets[id] = rt;

			mRenderTargetConstants.gScreenSize = Vec2((Real)width, (Real)height);
			mRenderTargetConstants.gScreenRatio = width / (float)height;
			mRenderTargetConstants.rendertarget_dummy = 0;
			GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::RenderTarget, &mRenderTargetConstants, sizeof(RENDERTARGET_CONSTANTS));
			if (mainCanvas){
				OnMainCavasCreated();				
			}
			return true;
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "Failed to create cavas");
			return false;
		}		
	}

	void OnMainCavasCreated(){
		mResourceProvider = ResourceProvider::Create();
		GetMainRenderTarget()->Bind();
		// POSITION
		{
			INPUT_ELEMENT_DESC desc("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION].push_back(desc);
		}

		// POSITION_COLOR
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR].push_back(desc[1]);
		}

		// POSITION_COLOR_TEXCOORD
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 16,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD].push_back(desc[1]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD].push_back(desc[2]);
		}

		// POSITION_HDR_COLOR
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_FLOAT4, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_HDR_COLOR].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_HDR_COLOR].push_back(desc[1]);
		}

		// POSITION_NORMAL,
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("NORMAL", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL].push_back(desc[1]);
		}

		//POSITION_TEXCOORD,
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_TEXCOORD].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_TEXCOORD].push_back(desc[1]);
		}
		//POSITION_COLOR_TEXCOORD_BLENDINDICES,
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 16,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("BLENDINDICES", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 24,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0)
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
				.push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
				.push_back(desc[1]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
				.push_back(desc[2]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES]
				.push_back(desc[3]);
		}

		//POSITION_NORMAL_TEXCOORD,
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("NORMAL", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT2, 0, 24,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD].push_back(desc[1]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD].push_back(desc[2]);
		}

		// POSITION_VEC4,
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT4, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[1]);
		}

		// POSITION_VEC4_COLOR,
		{
			INPUT_ELEMENT_DESC desc[] =
			{
				INPUT_ELEMENT_DESC("POSITION", 0, INPUT_ELEMENT_FORMAT_FLOAT3, 0, 0,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("TEXCOORD", 0, INPUT_ELEMENT_FORMAT_FLOAT4, 0, 12,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
				INPUT_ELEMENT_DESC("COLOR", 0, INPUT_ELEMENT_FORMAT_UBYTE4, 0, 28,
				INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0),
			};
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[0]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[1]);
			mInputLayoutDescs[DEFAULT_INPUTS::POSITION_VEC4].push_back(desc[2]);
		}

		//-----------------------------------------------------------------------
		mDynVBs[DEFAULT_INPUTS::POSITION] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_P),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_COLOR] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PC),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PCT),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_NORMAL] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PN),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_TEXCOORD] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PT),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD_BLENDINDICES] =
			CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PCTB),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_VEC4] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PV4),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mDynVBs[DEFAULT_INPUTS::POSITION_VEC4_COLOR] = CreateVertexBuffer(0, sizeof(DEFAULT_INPUTS::V_PV4C),
			DEFAULT_DYN_VERTEX_COUNTS, BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

		//-----------------------------------------------------------------------
		static_assert(DEFAULT_INPUTS::COUNT == 10, "You may not define a new element of mInputLayoutDesc for the new description.");

		LuaObject multiFontSet("r_multiFont");
		if (multiFontSet.IsValid()){
			auto it = multiFontSet.GetSequenceIterator();
			LuaObject data;
			while (it.GetNext(data)){
				auto fontPath = data.GetString();
				if (!fontPath.empty()){
					FontPtr font = Font::Create();
					auto err = font->Init(fontPath.c_str());
					if (!err){
						font->SetTextEncoding(Font::UTF16);
						int height = Round(font->GetHeight());						
						mFonts[height] = font;
					}
				}
			}
		}
		else{
			FontPtr font = Font::Create();
			LuaObject r_font("r_font");
			std::string fontPath = r_font.GetString();
			if (fontPath.empty())
			{
				fontPath = "EssentialEngineData/fonts/font22.fnt";
			}
			auto err = font->Init(fontPath.c_str());
			if (!err){
				font->SetTextEncoding(Font::UTF16);
				int height = Round(font->GetHeight());
				mFonts[height] = font;
			}
		}

		mDebugHud = DebugHud::Create();
		mGeomRenderer = GeometryRenderer::Create();	

		for (int i = 0; i <  ResourceTypes::SamplerStates::Num; ++i)
		{
			auto sampler = mResourceProvider->GetSamplerState(i);
			SetSamplerState(i, BINDING_SHADER_PS, i);
		}

		UpdateRareConstantsBuffer();

		const auto& rtSize = GetMainRenderTargetSize();
		for (auto it : mFonts){
			it.second->SetRenderTargetSize(rtSize);
		}
		if (mDebugHud){
			mDebugHud->SetRenderTargetSize(rtSize);
		}
		if (mGeomRenderer){
			mGeomRenderer->SetRenderTargetSize(rtSize);
		}
	}

	void ReleaseCanvas(HWindowId id){
		auto it = mWindowHandles.Find(id);
		if (it == mWindowHandles.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "Cannot find the window.");
			return;
		}
	}

	void Render3DUIsToTexture()
	{
		/*if (!m3DUIEnabled)
			return;

		RenderEventMarker mark("Render3DUIsToTexture");
		for (auto scIt : mWindowRenderTargets) {
			for (auto rtIt : mUI3DObjectsRTs) {
				if (!rtIt.second->GetEnable())
					continue;
				auto& uiObjectsIt = mUI3DObjects.Find(std::make_pair(scIt.first, rtIt.first));
				if (uiObjectsIt != mUI3DObjects.end()){
					auto& uiObjects = uiObjectsIt->second;
					auto& rt = rtIt.second;
					rt->Bind();

					for (auto& uiobj : uiObjects)
					{
						uiobj->PreRender();
						uiobj->Render();
						uiobj->PostRender();
					}

					rt->Unbind();
					rt->GetRenderTargetTexture()->GenerateMips();
				}
			}
		}*/
	}

	void RenderUI(HWindowId hwndId)
	{
		//D3DEventMarker mark("RenderUI");
		//auto& uiobjects = mUIObjectsToRender[hwndId];
		//auto it = uiobjects.begin(), itEnd = uiobjects.end();
		//for (; it != itEnd; it++)
		//{
		//	(*it)->PreRender(); // temporary :)
		//	(*it)->Render();
		//	(*it)->PostRender();
		//}
	}

	void RenderDebugRenderTargets()
	{
		auto rt = GetMainRenderTarget();
		assert(rt);
		const auto& size = rt->GetSize();
		for (int i = 0; i < MaxDebugRenderTargets; i++)
		{
			if (mDebugRenderTargets[i].mTexture)
			{
				Vec2 pixelPos = mDebugRenderTargets[i].mPos * Vec2((Real)size.x, (Real)size.y);
				Vec2 pixelSize = mDebugRenderTargets[i].mSize * Vec2((Real)size.x, (Real)size.y);
				DrawQuadWithTexture(Round(pixelPos), Round(pixelSize), Color(1, 1, 1, 1),
					mDebugRenderTargets[i].mTexture);
			}
		}
	}

	void RenderFade()
	{
		if (mFadeAlpha <= 0)
			return;
		auto mainRT = GetMainRenderTarget();
		assert(mainRT);
		DrawQuad(Vec2I(0, 0), mainRT->GetSize(), Color(0, 0, 0, mFadeAlpha));
	}

	void Render(){
		auto mainRT = GetMainRenderTarget();
		if (!mainRT)
			return;
		Real dt = (gpTimer->GetTickCount() - mLastRenderedTime) / (Real)gpTimer->GetFrequency();
		InitFrameProfiler(dt);
		UpdateFrameConstantsBuffer();

		for (auto pRT : mRenderTargetsEveryFrame)
		{
			auto rt = pRT.lock();
			if (rt)
				rt->Render();
		}

		Render3DUIsToTexture();		
		for (auto it : mWindowRenderTargets)
		{
			RenderEventMarker mark(FormatString("Processing render target for %u", it.first).c_str());
			auto hwndId = it.first;
			auto rt = (RenderTarget*)it.second.get();
			assert(rt);
			bool rendered = rt->Render();
			if (rendered) {
				auto& observers = mObject->mObservers_[IRendererObserver::DefaultRenderEvent];
				for (auto it = observers.begin(); it != observers.end(); /**/){
					auto observer = it->lock();
					if (!observer){
						it = observers.erase(it);
						continue;
					}
					++it;
					observer->BeforeUIRendering(hwndId, GetWindowHandle(hwndId));
				}

				for (auto it = observers.begin(); it != observers.end(); /**/){
					auto observer = it->lock();
					if (!observer){
						it = observers.erase(it);
						continue;
					}
					++it;
					observer->RenderUI(hwndId, GetWindowHandle(hwndId));
				}

				//RenderUI(hwndId);

				for (auto it = observers.begin(); it != observers.end(); /**/){
					auto observer = it->lock();
					if (!observer){
						it = observers.erase(it);
						continue;
					}
					++it;
					observer->AfterUIRendered(hwndId, GetWindowHandle(hwndId));
				}
			}
		}
		mainRT->BindTargetOnly(false);

		for (auto& it : mVideoPlayers){
			it->Render();
		}

		RenderDebugHud();
		RenderDebugRenderTargets();

		RenderFade();

		mConsoleRenderer->Render();
		GetPlatformRenderer().Present();
	}

	//-------------------------------------------------------------------
	// Resource Creation
	//-------------------------------------------------------------------
	RenderTargetPtr CreateRenderTarget(const RenderTargetParam& param){
		if (param.mUsePool){
			for (auto it = mRenderTargetPool.begin(); it != mRenderTargetPool.end(); it++)
			{
				if ((*it)->CheckOptions(param))
				{
					if (param.mEveryFrame)
						mRenderTargets.push_back(*it);
					auto rt = *it;
					mRenderTargetPool.erase(it);
					return rt;
				}
			}
		}

		auto rt = RenderTarget::Create();
		mRenderTargets.push_back(rt);		
		rt->SetColorTextureDesc(param.mSize.x, param.mSize.y, param.mPixelFormat, param.mShaderResourceView,
				param.mMipmap, param.mCubemap);
		
		rt->SetUsePool(param.mUsePool);
		if (param.mEveryFrame)
		{
			mRenderTargetsEveryFrame.push_back(rt);
			return rt;
		}
		else
		{
			mRenderTargets.push_back(rt);
			return rt;
		}
		return rt;
	}

	void KeepRenderTargetInPool(RenderTargetPtr rt){
		if (!rt)
			return;
		if (!ValueExistsInVector(mRenderTargetPool, rt)){
			mRenderTargetPool.push_back(rt);
		}
	}

	VectorMap<std::string, IPlatformTextureWeakPtr> sPlatformTextures;
	TexturePtr CreateTexture(const char* file, bool async){
		if (!ValidCStringLength(file)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}
		std::string loweredFilepath(file);
		ToLowerCase(loweredFilepath);
		auto it = sPlatformTextures.Find(loweredFilepath);
		if (it != sPlatformTextures.end()){
			auto platformTexture = it->second.lock();
			if (platformTexture){
				auto texture = GetTextureFromExistings(platformTexture);
				if (texture){
					return texture;
				}
			}
		}

		IPlatformTexturePtr platformTexture = GetPlatformRenderer().CreateTexture(file, async);
		if (!platformTexture){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Platform renderer failed to load a texture(%s)", file).c_str());
			return 0;
		}
		sPlatformTextures[loweredFilepath] = platformTexture;
		auto texture = CreateTexture(platformTexture);
		texture->SetFilePath(loweredFilepath.c_str());
		return texture;
	}

	TexturePtr CreateTexture(void* data, int width, int height, PIXEL_FORMAT format,
		BUFFER_USAGE usage, int  buffer_cpu_access, int texture_type){
		auto platformTexture = GetPlatformRenderer().CreateTexture(data, width, height, format, usage, buffer_cpu_access, texture_type);
		if (!platformTexture){
			Logger::Log(FB_ERROR_LOG_ARG, "Failed to create texture with data.");
			return 0;
		}
		auto texture = Texture::Create();
		texture->SetPlatformTexture(platformTexture);
		return texture;
	}

	TexturePtr CreateTexture(IPlatformTexturePtr platformTexture){
		auto texture = Texture::Create();
		texture->SetPlatformTexture(platformTexture);
		return texture;
	}

	VertexBufferPtr CreateVertexBuffer(void* data, unsigned stride,
		unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) {
		auto platformBuffer = GetPlatformRenderer().CreateVertexBuffer(data, stride, numVertices, usage, accessFlag);
		if (!platformBuffer){
			Logger::Log(FB_ERROR_LOG_ARG, "Platform renderer failed to create a vertex buffer");
			return 0;
		}
		auto vertexBuffer = VertexBuffer::Create(stride, numVertices);
		vertexBuffer->SetPlatformBuffer(platformBuffer);
		return vertexBuffer;
	}

	IndexBufferPtr CreateIndexBuffer(void* data, unsigned int numIndices,
		INDEXBUFFER_FORMAT format) {
		auto platformBuffer = GetPlatformRenderer().CreateIndexBuffer(data, numIndices, format);
		if (!platformBuffer){
			Logger::Log(FB_ERROR_LOG_ARG, "Platform renderer failed to create a index buffer");
			return 0;
		}
		auto indexBuffer = IndexBuffer::Create(numIndices, format);
		indexBuffer->SetPlatformBuffer(platformBuffer);
		return indexBuffer;
	}

	struct ShaderCreationInfo{
		ShaderCreationInfo(const char* path, int shaders, const SHADER_DEFINES& defines)
			: mFilepath(path)
			, mShaders(shaders)
			, mDefines(defines)
		{
			ToLowerCase(mFilepath);
			std::sort(mDefines.begin(), mDefines.end());
		}
		bool operator < (const ShaderCreationInfo& other) const{			
			if (mShaders < other.mShaders)
				return true;
			else if (mShaders == other.mShaders){
				if (mFilepath < other.mFilepath)
					return true;
				else if (mFilepath == other.mFilepath){
					return mDefines < other.mDefines;
				}
			}
			return false;
		}

		bool operator == (const ShaderCreationInfo& other) const{
			return mShaders == other.mShaders && mFilepath == other.mFilepath && mDefines == other.mDefines;
		}

		int mShaders;
		std::string mFilepath;		
		SHADER_DEFINES mDefines;
	};
	VectorMap<ShaderCreationInfo, IPlatformShaderWeakPtr> sPlatformShaders;
	IPlatformShaderPtr FindPlatformShader(const ShaderCreationInfo& key){
		auto it = sPlatformShaders.Find(key);
		if (it != sPlatformShaders.end()){
			auto platformShader = it->second.lock();
			if (platformShader){
				return platformShader;
			}
		}
		return 0;
	}
	ShaderPtr CreateShader(const char* filepath, int shaders) {
		CreateShader(filepath, shaders, SHADER_DEFINES());
	}
	ShaderPtr CreateShader(const char* filepath, int shaders,
		const SHADER_DEFINES& defines) {
		if (!ValidCStringLength(filepath)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}

		SHADER_DEFINES sortedDefines(defines);
		std::string loweredPath(filepath);		
		std::sort(sortedDefines.begin(), sortedDefines.end());
		auto key = ShaderCreationInfo(loweredPath.c_str(), shaders, sortedDefines);		
		auto platformShader = FindPlatformShader(key);
		if (platformShader){
			auto shader = GetShaderFromExistings(platformShader);
			if (shader){				
				assert(shader->GetShaderDefines() == sortedDefines);
				return shader;
			}
		}
		if (!platformShader)
			platformShader = GetPlatformRenderer().CreateShader(filepath, shaders, sortedDefines);
		if (platformShader){
			auto shader = Shader::Create();
			shader->SetPlatformShader(platformShader);
			shader->SetShaderDefines(sortedDefines);
			shader->SetPath(filepath);
			shader->SetBindingShaders(shaders);
			sPlatformShaders[key] = platformShader;
			if (strcmp(filepath, "EssentialEngineData/shaders/UI.hlsl") == 0 && defines.size() == 1){
				if (!tempData && defines[0].name == "DIFFUSE_TEXTURE"){
					void* data = platformShader->GetVSByteCode(tempSize);
					tempData = BinaryData(new char[tempSize], [](char* obj){ delete[] obj; });
					memcpy(tempData.get(), data, tempSize);
				}
			}
			return shader;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to create a shader(%s)", filepath).c_str());
		return 0;
	}

	bool ReapplyShaderDefines(Shader* shader){
		if (!shader)
			return false;
		auto filepath = shader->GetPath();
		std::string loweredPath = filepath;
		if (loweredPath.empty()){
			Logger::Log(FB_ERROR_LOG_ARG, "Path is empty.");
			return false;
		}
		ToLowerCase(loweredPath);
		int bindingShaders = shader->GetBindingShaders();
		auto sortedDefines = shader->GetShaderDefines();
		std::sort(sortedDefines.begin(), sortedDefines.end());
		auto key = ShaderCreationInfo(loweredPath.c_str(), bindingShaders, sortedDefines);
		auto platformShader = FindPlatformShader(key);
		if (platformShader){
			shader->SetPlatformShader(platformShader);
			return true;
		}
		platformShader = GetPlatformRenderer().CreateShader(filepath, bindingShaders, sortedDefines);
		if (platformShader){
			shader->SetPlatformShader(platformShader);
			return true;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to reapply shader defines(%s)", filepath).c_str());
		return false;
	}

	VectorMap<std::string, MaterialWeakPtr> sLoadedMaterials;
	MaterialPtr CreateMaterial(const char* file){
		if (!ValidCStringLength(file)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return 0;
		}
		std::string loweredPath(file);
		ToLowerCase(loweredPath);		
		auto it = sLoadedMaterials.Find(loweredPath);
		if (it != sLoadedMaterials.end()){
			auto material = it->second.lock();
			if (material){
				return material->Clone();
			}
		}
		auto material = Material::Create();
		if (!material->LoadFromFile(file))
			return 0;

		sLoadedMaterials[loweredPath] = material;
		return material;

	}
	
	VectorMap<unsigned, InputLayoutWeakPtr> sInputLayouts;
	// use this if you are sure there is instance of the descs.
	InputLayoutPtr CreateInputLayout(const INPUT_ELEMENT_DESCS& descs, ShaderPtr shader){
		if (!shader || descs.empty()){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid param.");
			return 0;
		}
		unsigned size;
		void* data = shader->GetVSByteCode(size);
		if (!data){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid shader");
			return 0;
		}
		auto descsSize = sizeof(INPUT_ELEMENT_DESCS) * descs.size();
		auto totalSize = descsSize + size;
		BinaryData temp(new char[totalSize]);
		memcpy(temp.get(), &descs[0], sizeof(descs));
		memcpy(temp.get() + descsSize, data, size);
		unsigned key = murmur3_32(temp.get(), totalSize, murmurSeed);
		auto it = sInputLayouts.Find(key);
		if (it != sInputLayouts.end()){
			auto inputLayout = it->second.lock();
			if (inputLayout)
				return inputLayout;
		}
		auto platformInputLayout = GetPlatformRenderer().CreateInputLayout(descs, data, size);
		auto inputLayout = InputLayout::Create();
		inputLayout->SetPlatformInputLayout(platformInputLayout);
		sInputLayouts[key] = inputLayout;
		return inputLayout;
	}

	InputLayoutPtr GetInputLayout(DEFAULT_INPUTS::Enum e, ShaderPtr shader){
		const auto& desc = GetInputElementDesc(e);
		return CreateInputLayout(desc, shader);
	}

	VectorMap<RASTERIZER_DESC, RasterizerStateWeakPtr> sRasterizerStates;
	RasterizerStatePtr CreateRasterizerState(const RASTERIZER_DESC& desc){
		auto it = sRasterizerStates.Find(desc);
		if (it != sRasterizerStates.end()){
			auto state = it->second.lock();
			if (state){
				return state;
			}
		}
		auto platformRaster = GetPlatformRenderer().CreateRasterizerState(desc);
		auto raster = RasterizerState::Create();
		raster->SetPlatformState(platformRaster);
		sRasterizerStates[desc] = raster;
		return raster;
	}

	VectorMap<BLEND_DESC, BlendStateWeakPtr> sBlendStates;
	BlendStatePtr CreateBlendState(const BLEND_DESC& desc){		
		auto it = sBlendStates.Find(desc);
		if (it != sBlendStates.end()){
			auto state = it->second.lock();
			if (state){
				return state;
			}
		}
		auto platformState = GetPlatformRenderer().CreateBlendState(desc);
		auto state = BlendState::Create();
		state->SetPlatformState(platformState);
		sBlendStates[desc] = state;
		return state;
		
	}

	VectorMap<DEPTH_STENCIL_DESC, DepthStencilStateWeakPtr> sDepthStates;
	DepthStencilStatePtr CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc){
		auto it = sDepthStates.Find(desc);
		if (it != sDepthStates.end()){
			auto state = it->second.lock();
			if (state){
				return state;
			}
		}
		auto platformState = GetPlatformRenderer().CreateDepthStencilState(desc);
		auto state = DepthStencilState::Create();
		state->SetPlatformState(platformState);
		sDepthStates[desc] = state;
		return state;
	}

	VectorMap<SAMPLER_DESC, SamplerStateWeakPtr> sSamplerStates;
	SamplerStatePtr CreateSamplerState(const SAMPLER_DESC& desc){
		auto it = sSamplerStates.Find(desc);
		if (it != sSamplerStates.end()){
			auto state = it->second.lock();
			if (state){
				return state;
			}
		}
		auto platformState = GetPlatformRenderer().CreateSamplerState(desc);
		auto state = SamplerState::Create();
		state->SetPlatformState(platformState);
		sSamplerStates[desc] = state;
		return state;

	}

	// holding strong pointer
	VectorMap<std::string, TextureAtlasPtr> sTextureAtlas;
	TextureAtlasPtr GetTextureAtlas(const char* path){
		std::string filepath(path);
		ToLowerCase(filepath);
		auto it = sTextureAtlas.Find(filepath);
		if (it != sTextureAtlas.end()){
			return it->second;
		}

		tinyxml2::XMLDocument doc;
		doc.LoadFile(filepath.c_str());
		if (doc.Error())
		{
			const char* errMsg = doc.GetErrorStr1();
			if (ValidCStringLength(errMsg)){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("%s(%s)", errMsg, path));
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot load texture atlas(%s)", path));
			}
			return 0;
		}

		tinyxml2::XMLElement* pRoot = doc.FirstChildElement("TextureAtlas");
		if (!pRoot)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invalid Texture Atlas(%s)", path).c_str());
			return 0;
		}

		const char* szBuffer = pRoot->Attribute("file");
		TextureAtlasPtr pTextureAtlas;
		if (szBuffer)
		{			
			pTextureAtlas = TextureAtlas::Create();
			pTextureAtlas->SetPath(filepath.c_str());			
			pTextureAtlas->SetTexture(CreateTexture(szBuffer, true));
			sTextureAtlas[filepath] = pTextureAtlas;		
			if (!pTextureAtlas->GetTexture())
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Texture for atlas(%s) is not found", szBuffer).c_str());
				return pTextureAtlas;
			}
		}
		else
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid TextureAtlas format! No Texture Defined.");
			return 0;
		}

		auto texture = pTextureAtlas->GetTexture();
		Vec2I textureSize = texture->GetSize();
		if (textureSize.x != 0 && textureSize.y != 0)
		{
			tinyxml2::XMLElement* pRegionElem = pRoot->FirstChildElement("region");
			while (pRegionElem)
			{
				szBuffer = pRegionElem->Attribute("name");
				if (!szBuffer)
				{
					Logger::Log(FB_DEFAULT_LOG_ARG, "No name for texture atlas region");
					continue;
				}

				auto pRegion = pTextureAtlas->AddRegion(szBuffer);				
				pRegion->mID = pRegionElem->UnsignedAttribute("id");
				pRegion->mStart.x = pRegionElem->IntAttribute("x");
				pRegion->mStart.y = pRegionElem->IntAttribute("y");
				pRegion->mSize.x = pRegionElem->IntAttribute("width");
				pRegion->mSize.y = pRegionElem->IntAttribute("height");
				Vec2 start((Real)pRegion->mStart.x, (Real)pRegion->mStart.y);
				Vec2 end(start.x + pRegion->mSize.x, start.y + pRegion->mSize.y);
				pRegion->mUVStart = start / textureSize;
				pRegion->mUVEnd = end / textureSize;
				pRegionElem = pRegionElem->NextSiblingElement();
			}
		}
		else
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Texture size is 0, 0.");
		}

		return pTextureAtlas;
	}

	TextureAtlasRegionPtr GetTextureAtlasRegion(const char* path, const char* region){
		auto pTextureAtlas = GetTextureAtlas(path);
		if (pTextureAtlas)
		{
			return pTextureAtlas->GetRegion(region);
		}

		return 0;
	}

	TexturePtr GetTemporalDepthBuffer(const Vec2I& size){
		auto it = mTempDepthBuffers.Find(size);
		if (it == mTempDepthBuffers.end())
		{
			auto depthBuffer = CreateTexture(0, size.x, size.y, PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
				BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL);
			mTempDepthBuffers.Insert(std::make_pair(size, depthBuffer));
			return depthBuffer;
		}
		return it->second;
	}

	PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime,
		bool manualDeletion){
		assert(mPointLightMan);
		RefreshPointLight();
		return mPointLightMan->CreatePointLight(pos, range, color, intensity, lifeTime, manualDeletion);
	}
	

	//-------------------------------------------------------------------
	// Resource Bindings
	//-------------------------------------------------------------------	
	void SetRenderTarget(TexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
		TexturePtr pDepthStencil, size_t dsViewIndex){
		assert(num <= 4);
		if (mCurrentDSTexture == pDepthStencil && mCurrentDSViewIndex == dsViewIndex){
			if (mCurrentRTTextures.size() == num && mCurrentViewIndices.size() == num){
				bool same = true;
				for (int i = 0; i < num; ++i){
					if (mCurrentRTTextures[i] != pRenderTargets[i] || mCurrentViewIndices[i] != rtViewIndex[i]){
						same = false;
					}
				}
				if (same){
					return;
				}
			}
		}
		static TIME_PRECISION time = 0;
		static std::set<TextureWeakPtr, std::owner_less<TextureWeakPtr>> usedRenderTargets;
		if (GetRendererOptions()->r_numRenderTargets && gpTimer)
		{
			for (int i = 0; i<num; i++)
			{
				usedRenderTargets.insert(pRenderTargets[i]);
			}
			if (gpTimer->GetTime() - time > 5)
			{
				time = gpTimer->GetTime();
				Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("used RenderTargets", usedRenderTargets.size()).c_str());				
			}
		}
		mCurrentRTTextures.clear();
		mCurrentViewIndices.clear();		
		std::copy(pRenderTargets, pRenderTargets + num, std::back_inserter(mCurrentRTTextures));
		std::copy(rtViewIndex, rtViewIndex + num, std::back_inserter(mCurrentViewIndices));
		mCurrentDSTexture = pDepthStencil;
		mCurrentDSViewIndex = dsViewIndex;
		IPlatformTexturePtr platformRTs[4] = { 0 };
		for (int i = 0; i < num; ++i){
			platformRTs[i] = pRenderTargets[i] ? pRenderTargets[i]->GetPlatformTexture() : 0;
		}

		GetPlatformRenderer().SetRenderTarget(platformRTs, rtViewIndex, num,
			pDepthStencil ? pDepthStencil->GetPlatformTexture() : 0, dsViewIndex);

		if (pRenderTargets && num>0 && pRenderTargets[0])
		{
			mCurrentRTSize = pRenderTargets[0]->GetSize();
		}
		else
		{
			mCurrentRTSize = GetMainRenderTargetSize();
		}

		for (auto it : mFonts){
			it.second->SetRenderTargetSize(mCurrentRTSize);
		}

		UpdateRenderTargetConstantsBuffer();
	}

	void UnbindRenderTarget(TexturePtr renderTargetTexture){
		auto rtTextures = mCurrentRTTextures;
		auto viewIndices = mCurrentViewIndices;
		auto size = rtTextures.size();
		bool removed = false;
		for (unsigned i = 0; i < size; i++){
			if (rtTextures[i] == renderTargetTexture){
				rtTextures[i] = 0;
				viewIndices[i] = 0;
				removed = true;
			}
		}
		auto dsTexture = mCurrentDSTexture;
		auto dsViewIndex = mCurrentDSViewIndex;
		if (dsTexture == renderTargetTexture)
		{
			removed = true;
			dsTexture = 0;
			dsViewIndex = 0;
		}
		if (removed){
			SetRenderTarget(&rtTextures[0], &viewIndices[0], size, dsTexture, dsViewIndex);
		}
	}

	void SetViewports(const Viewport viewports[], int num){
		GetPlatformRenderer().SetViewports(viewports, num);
	}

	void SetScissorRects(const Rect rects[], int num){
		GetPlatformRenderer().SetScissorRects(rects, num);
	}

	void SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,
		VertexBufferPtr pVertexBuffers[], unsigned int strides[], unsigned int offsets[]) {
		static const unsigned int numMaxVertexInputSlot = 32; //D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT (32)
		IPlatformVertexBuffer const *  platformBuffers[numMaxVertexInputSlot];
		numBuffers = std::min(numMaxVertexInputSlot, numBuffers);		
		for (unsigned i = 0; i < numBuffers; ++i){
			platformBuffers[i] = pVertexBuffers[i] ? pVertexBuffers[i]->GetPlatformBuffer().get() : 0;
		}
		GetPlatformRenderer().SetVertexBuffers(startSlot, numBuffers, platformBuffers, strides, offsets);
	}

	void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt){
		if (mCurrentTopology == pt)
			return;
		GetPlatformRenderer().SetPrimitiveTopology(pt);
		mCurrentTopology = pt;
	}

	void SetTextures(TexturePtr pTextures[], int num, BINDING_SHADER shaderType, int startSlot){
		static const int maxBindableTextures = 20; // D3D11_REQ_RESOURCE_VIEW_COUNT_PER_DEVICE_2_TO_EXP(20)
		IPlatformTexturePtr textures[maxBindableTextures];
		num = std::min(num, maxBindableTextures);
		for (int i = 0; i < num; ++i){
			textures[i] = pTextures[i] ? pTextures[i]->GetPlatformTexture() : 0;
		}
		GetPlatformRenderer().SetTextures(textures, num, shaderType, startSlot);
	}

	void SetSystemTexture(SystemTextures::Enum type, TexturePtr texture){
		auto it = mSystemTextureBindings.Find(type);
		if (it == mSystemTextureBindings.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the binding information for the system texture(%d)", type).c_str());
			return;
		}
		if (texture){
			for (const auto& binding : it->second){
				texture->Bind(binding.mShader, binding.mSlot);
			}
		}
		else{
			for (const auto& binding : it->second){
				UnbindTexture(binding.mShader, binding.mSlot);				
			}
		}
	}
	
	void BindDepthTexture(bool set){
		auto mainRt = GetMainRenderTarget();
		if (mainRt){
			mainRt->BindDepthTexture(set);
		}
	}

	void SetDepthWriteShader(){
		auto depthWriteShader = mResourceProvider->GetShader(ResourceTypes::Shaders::DepthWriteVSPS);
		if (depthWriteShader){
			SetPositionInputLayout();
			depthWriteShader->Bind();
		}
	}

	void SetDepthWriteShaderCloud(){
		auto cloudDepthWriteShader = mResourceProvider->GetShader(ResourceTypes::Shaders::CloudDepthWriteVSPS);
		if (cloudDepthWriteShader){
			SetPositionInputLayout();
			cloudDepthWriteShader->Bind();
		}
	}

	void SetPositionInputLayout(){
		if (!mPositionInputLayout)
		{
			auto shader = mResourceProvider->GetShader(ResourceTypes::Shaders::ShadowMapVSPS);
			if (!shader){
				//fall-back
				shader = mResourceProvider->GetShader(ResourceTypes::Shaders::DepthWriteVSPS);
			}
			if (shader)
				mPositionInputLayout = GetInputLayout(DEFAULT_INPUTS::POSITION, shader);					
			if (mPositionInputLayout)
				mPositionInputLayout->Bind();
		}
		else{
			mPositionInputLayout->Bind();
		}
	}

	void SetSystemTextureBindings(SystemTextures::Enum type, const TextureBindings& bindings){
		mSystemTextureBindings[type] = bindings;
	}

	const TextureBindings& GetSystemTextureBindings(SystemTextures::Enum type){
		auto it = mSystemTextureBindings.Find(type);
		if (it != mSystemTextureBindings.end())
			return it->second;
		static TextureBindings noBindingInfo;
		return noBindingInfo;
	}

	//-------------------------------------------------------------------
	// Device RenderStates
	//-------------------------------------------------------------------
	void RestoreRenderStates(){
		RestoreRasterizerState();
		RestoreBlendState();
		RestoreDepthStencilState();
	}
	void RestoreRasterizerState(){
		auto state = mResourceProvider->GetRasterizerState(ResourceTypes::RasterizerStates::Default);
		if (state)
			state->Bind();
	}
	void RestoreBlendState(){
		auto state = mResourceProvider->GetBlendState(ResourceTypes::BlendStates::Default);
		if (state)
			state->Bind();
	}
	void RestoreDepthStencilState(){
		auto state = mResourceProvider->GetDepthStencilState(ResourceTypes::DepthStencilStates::Default);
		if (state)
			state->Bind();
	}

	// sampler
	void SetSamplerState(int ResourceTypes_SamplerStates, BINDING_SHADER shader, int slot){
		auto sampler = mResourceProvider->GetSamplerState(ResourceTypes_SamplerStates);
		if (!sampler){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find sampler (%d)", ResourceTypes_SamplerStates).c_str());
			return;
		}
		sampler->Bind(shader, slot);
	}


	//-------------------------------------------------------------------
	// GPU constants
	//-------------------------------------------------------------------
	void UpdateObjectConstantsBuffer(const void* pData, bool record){
		if (mRendererOptions->r_noObjectConstants)
			return;
		if (record)
			mFrameProfiler.NumUpdateObjectConst += 1;

		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::Object, pData, sizeof(OBJECT_CONSTANTS));
	}

	void UpdatePointLightConstantsBuffer(const void* pData){
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::PointLight, pData, sizeof(POINT_LIGHT_CONSTANTS));
	}

	void UpdateFrameConstantsBuffer(){
		mFrameConstants.gMousePos.x = (float)mInputInfo.mCurrentMousePos.x;
		mFrameConstants.gMousePos.y = (float)mInputInfo.mCurrentMousePos.y;
		bool lbuttonDown = mInputInfo.mLButtonDown;
		mFrameConstants.gMousePos.z = lbuttonDown ? (float)mInputInfo.mCurrentMousePos.x : 0;
		mFrameConstants.gMousePos.w = lbuttonDown ? (float)mInputInfo.mCurrentMousePos.y : 0;
		mFrameConstants.gTime = (float)gpTimer->GetTime();
		mFrameConstants.gDeltaTime = (float)gpTimer->GetDeltaTime();
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::Frame, &mFrameConstants, sizeof(FRAME_CONSTANTS));
	}

	void UpdateMaterialConstantsBuffer(const void* pData){
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::MaterialConstant, pData, sizeof(MATERIAL_CONSTANTS));
	}

	void UpdateCameraConstantsBuffer(){
		if (!mCamera)
			return;

		mCameraConstants.gView = mCamera->GetMatrix(ICamera::View);
		
		mCameraConstants.gInvView = mCamera->GetMatrix(ICamera::InverseView);
		mCameraConstants.gViewProj = mCamera->GetMatrix(ICamera::ViewProj);
		mCameraConstants.gInvViewProj = mCamera->GetMatrix(ICamera::InverseViewProj);
		mCamera->GetTransformation().GetHomogeneous(mCameraConstants.gCamTransform);
		mCameraConstants.gProj = mCamera->GetMatrix(ICamera::Proj);
		mCameraConstants.gInvProj = mCamera->GetMatrix(ICamera::InverseProj);
		Real ne, fa;
		mCamera->GetNearFar(ne, fa);
		mCameraConstants.gNearFar.x = (float)ne;
		mCameraConstants.gNearFar.y = (float)fa;
		mCameraConstants.gTangentTheta = (float)tan(mCamera->GetFOV() / 2.0);
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::Camera, &mCameraConstants, sizeof(CAMERA_CONSTANTS));
	}

	void UpdateRenderTargetConstantsBuffer(){
		mRenderTargetConstants.gScreenSize.x = (float)mCurrentRTSize.x;
		mRenderTargetConstants.gScreenSize.y = (float)mCurrentRTSize.y;
		mRenderTargetConstants.gScreenRatio = mCurrentRTSize.x / (float)mCurrentRTSize.y;
		mRenderTargetConstants.rendertarget_dummy = 0.f;
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::RenderTarget, &mRenderTargetConstants, sizeof(RENDERTARGET_CONSTANTS));
	}

	void UpdateSceneConstantsBuffer(){
		if (!mCurrentRenderTarget){
			Logger::Log(FB_ERROR_LOG_ARG, "No current render target found.");
			return;
		}
		auto scene = mCurrentScene.lock();
		if (!scene){
			return;
		}
		auto pLightCam = mCurrentRenderTarget->GetLightCamera();
		if (pLightCam)
			mSceneConstants.gLightViewProj = pLightCam->GetMatrix(ICamera::ViewProj);

		for (int i = 0; i < 2; i++)
		{
			mSceneConstants.gDirectionalLightDir_Intensity[i] = float4(mDirectionalLight[i].mDirection_Intensiy);
			mSceneConstants.gDirectionalLightDiffuse[i] = float4(mDirectionalLight[i].mDiffuse);
			mSceneConstants.gDirectionalLightSpecular[i] = float4(mDirectionalLight[i].mSpecular);
		}
		mSceneConstants.gFogColor = scene->GetFogColor().GetVec4();
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::Scene, &mSceneConstants, sizeof(SCENE_CONSTANTS));
	}
	void UpdateRareConstantsBuffer(){
		RARE_CONSTANTS rare;
		rare.gMiddleGray = mRendererOptions->r_HDRMiddleGray;
		rare.gStarPower = mRendererOptions->r_StarPower;
		rare.gBloomPower = mRendererOptions->r_BloomPower;
		rare.gRareDummy = 0.f;
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::RareChange, &rare, sizeof(RARE_CONSTANTS));

	}
	void UpdateRadConstantsBuffer(const void* pData){
		GetPlatformRenderer().UpdateShaderConstants(ShaderConstants::Radiance, pData, sizeof(IMMUTABLE_CONSTANTS));
	}

	void* MapMaterialParameterBuffer(){
		return GetPlatformRenderer().MapMaterialParameterBuffer();
	}

	void UnmapMaterialParameterBuffer(){
		GetPlatformRenderer().UnmapMaterialParameterBuffer();
	}
	void* MapBigBuffer(){
		return GetPlatformRenderer().MapBigBuffer();
	}
	void UnmapBigBuffer(){
		GetPlatformRenderer().UnmapBigBuffer();
	}
	//-------------------------------------------------------------------
	// GPU Manipulation
	//-------------------------------------------------------------------
	void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation){
		GetPlatformRenderer().DrawIndexed(indexCount, startIndexLocation, startVertexLocation);
	}

	void Draw(unsigned int vertexCount, unsigned int startVertexLocation){
		GetPlatformRenderer().Draw(vertexCount, startVertexLocation);
	}

	void DrawFullscreenQuad(ShaderPtr pixelShader, bool farside){
		// vertex buffer

		ShaderPtr shader;
		if (farside)
			shader = mResourceProvider->GetShader(ResourceTypes::Shaders::FullscreenQuadFarVS);
		else
			shader = mResourceProvider->GetShader(ResourceTypes::Shaders::FullscreenQuadNearVS);

		if (shader){
			shader->Bind();
		}
		else{
			return;
		}

		if (pixelShader)
			pixelShader->BindPS();

		UnbindInputLayout();
		UnbindShader(BINDING_SHADER_GS);
		// draw
		// using full screen triangle : http://blog.naver.com/jungwan82/220108100698
		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Draw(3, 0);
	}

	void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec4& color, MaterialPtr mat){
		VertexBuffer* pVB = mDynVBs[DEFAULT_INPUTS::POSITION].get();
		assert(pVB);
		MapData mapped = pVB->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		DEFAULT_INPUTS::V_P* data = (DEFAULT_INPUTS::V_P*)mapped.pData;
		data[0].p = a;
		data[1].p = b;
		data[2].p = c;
		pVB->Unmap(0);
		pVB->Bind();
		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mat->SetMaterialParameter(0, color);
		mat->Bind(true);
		Draw(3, 0);
	}

	void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color, bool updateRs = true){
		// vertex buffer
		auto rtSize = GetCurrentRenderTargetSize();
		Mat44 screenToProj(2.f / rtSize.x, 0, 0, -1.f,
			0.f, -2.f / rtSize.y, 0, 1.f,
			0, 0, 1.f, 0.f,
			0, 0, 0, 1.f);

		MapData mapped = mDynVBs[DEFAULT_INPUTS::POSITION_COLOR]->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		DEFAULT_INPUTS::V_PC data[4] = {
			DEFAULT_INPUTS::V_PC(Vec3((float)pos.x, (float)pos.y, 0.f), color.Get4Byte()),
			DEFAULT_INPUTS::V_PC(Vec3((float)pos.x + size.x, (float)pos.y, 0.f), color.Get4Byte()),
			DEFAULT_INPUTS::V_PC(Vec3((float)pos.x, (float)pos.y + size.y, 0.f), color.Get4Byte()),
			DEFAULT_INPUTS::V_PC(Vec3((float)pos.x + size.x, (float)pos.y + size.y, 0.f), color.Get4Byte()),
		};
		for (int i = 0; i < 4; i++){
			data[i].p = (screenToProj * Vec4(data[i].p, 1.0)).GetXYZ();
		}
		if (mapped.pData)
		{
			memcpy(mapped.pData, data, sizeof(data));
			mDynVBs[DEFAULT_INPUTS::POSITION_COLOR]->Unmap(0);
		}

		if (updateRs){
			// set primitive topology
			SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			// set material
			auto quadMateiral = mResourceProvider->GetMaterial(ResourceTypes::Materials::Quad);
			if (quadMateiral){
				quadMateiral->Bind(true);
			}
		}


		// set vertex buffer
		mDynVBs[DEFAULT_INPUTS::POSITION_COLOR]->Bind();
		// draw
		Draw(4, 0);
	}

	void DrawQuadWithTexture(const Vec2I& pos, const Vec2I& size, const Color& color, TexturePtr texture, MaterialPtr materialOverride = 0){
		DrawQuadWithTextureUV(pos, size, Vec2(0, 0), Vec2(1, 1), color, texture, materialOverride);
	}

	void DrawQuadWithTextureUV(const Vec2I& pos, const Vec2I& size, const Vec2& uvStart, const Vec2& uvEnd,
		const Color& color, TexturePtr texture, MaterialPtr materialOverride = 0){
		// vertex buffer
		MapData mapped = mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD]->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		DEFAULT_INPUTS::V_PCT data[4] = {
			DEFAULT_INPUTS::V_PCT(Vec3((float)pos.x, (float)pos.y, 0.f), color.Get4Byte(), Vec2(uvStart.x, uvStart.y)),
			DEFAULT_INPUTS::V_PCT(Vec3((float)pos.x + size.x, (float)pos.y, 0.f), color.Get4Byte(), Vec2(uvEnd.x, uvStart.y)),
			DEFAULT_INPUTS::V_PCT(Vec3((float)pos.x, (float)pos.y + size.y, 0.f), color.Get4Byte(), Vec2(uvStart.x, uvEnd.y)),
			DEFAULT_INPUTS::V_PCT(Vec3((float)pos.x + size.x, (float)pos.y + size.y, 0.f), color.Get4Byte(), Vec2(uvEnd.x, uvEnd.y)),
		};
		auto& rtSize = GetCurrentRenderTargetSize();
		Mat44 screenToProj(2.f / rtSize.x, 0, 0, -1.f,
			0.f, -2.f / rtSize.y, 0, 1.f,
			0, 0, 1.f, 0.f,
			0, 0, 0, 1.f);

		for (int i = 0; i < 4; ++i){
			data[i].p = (screenToProj * Vec4(data[i].p, 1.)).GetXYZ();
		}

		if (mapped.pData)
		{
			memcpy(mapped.pData, data, sizeof(data));
			mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD]->Unmap(0);
		}

		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		if (materialOverride)
			materialOverride->Bind(true);
		else{
			auto quadTextureMat = mResourceProvider->GetMaterial(ResourceTypes::Materials::QuadTextured);
			if (quadTextureMat){
				quadTextureMat->Bind(true);
			}
		}
			
		texture->Bind(BINDING_SHADER_PS, 0);
		mDynVBs[DEFAULT_INPUTS::POSITION_COLOR_TEXCOORD]->Bind();
		Draw(4, 0);
	}

	void DrawBillboardWorldQuad(const Vec3& pos, const Vec2& size, const Vec2& offset,
		DWORD color, MaterialPtr pMat){
		VertexBuffer* pVB = mDynVBs[DEFAULT_INPUTS::POSITION_VEC4_COLOR].get();
		assert(pVB);
		MapData mapped = pVB->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		DEFAULT_INPUTS::POSITION_VEC4_COLOR_V* data = (DEFAULT_INPUTS::POSITION_VEC4_COLOR_V*)mapped.pData;
		data->p = pos;
		data->v4 = Vec4(size.x, size.y, offset.x, offset.y);
		data->color = color;
		pVB->Unmap(0);
		pVB->Bind();
		SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
		pMat->Bind(true);
		Draw(1, 0);
	}

	void QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color, Real size){
		if (mDebugHud)
			mDebugHud->DrawText(pos, text, color, size);
	}

	void QueueDrawText(const Vec2I& pos, const char* text, const Color& color, Real size){
		QueueDrawText(pos, AnsiToWide(text, strlen(text)), color, size);
	}

	void QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, Real size){
		if (mDebugHud)
			mDebugHud->Draw3DText(worldpos, text, color, size);
	}

	void QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color, Real size){
		QueueDraw3DText(worldpos, AnsiToWide(text), color, size);
	}

	void QueueDrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text,
		const Color& color, Real size){
		if (mDebugHud)
			mDebugHud->DrawTextForDuration(secs, pos, text, color, size);
	}

	void QueueDrawTextForDuration(Real secs, const Vec2I& pos, const char* text,
		const Color& color, Real size){
		QueueDrawTextForDuration(secs, pos, AnsiToWide(text, strlen(text)), color, size);
	}

	void ClearDurationTexts(){
		if (mDebugHud)
			mDebugHud->ClearDurationTexts();
	}

	void QueueDrawLine(const Vec3& start, const Vec3& end,
		const Color& color0, const Color& color1){
		if (mDebugHud)
			mDebugHud->DrawLine(start, end, color0, color1);
	}

	void QueueDrawLine(const Vec2I& start, const Vec2I& end,
		const Color& color0, const Color& color1){
		if (mDebugHud)
			mDebugHud->DrawLine(start, end, color0, color1);
	}

	void QueueDrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
		const Color& color0, const Color& color1){
		if (mDebugHud)
			mDebugHud->DrawLineBeforeAlphaPass(start, end, color0, color1);
	}

	void QueueDrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color){
		if (mDebugHud){
			mDebugHud->DrawQuad(pos, size, color);
		}
	}

	void QueueDrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness,
		const char* texture, bool textureFlow){
		if (mGeomRenderer)
			mGeomRenderer->DrawTexturedThickLine(start, end, color0, color1, thickness, texture, textureFlow);
	}

	void QueueDrawSphere(const Vec3& pos, Real radius, const Color& color){
		if (mGeomRenderer)
			mGeomRenderer->DrawSphere(pos, radius, color);
	}

	void QueueDrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha){
		if (mGeomRenderer)
			mGeomRenderer->DrawBox(boxMin, boxMax, color, alpha);
	}

	void QueueDrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha){
		if (mGeomRenderer)
			mGeomRenderer->DrawTriangle(a, b, c, color, alpha);
	}

	void QueueDrawQuadLine(const Vec2I& pos, const Vec2I& size, const Color& color){
		int left = pos.x - 1;
		int top = pos.y - 1;
		int right = pos.x + size.x + 1;
		int bottom = pos.y + size.y + 1;
		QueueDrawLine(Vec2I(left, top), Vec2I(right, top), color, color);
		QueueDrawLine(Vec2I(left, top), Vec2I(left, bottom), color, color);
		QueueDrawLine(Vec2I(right, top), Vec2I(right, bottom), color, color);
		QueueDrawLine(Vec2I(left, bottom), Vec2I(right, bottom), color, color);
	}

	//-------------------------------------------------------------------
	// Internal
	//-------------------------------------------------------------------
	void GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst){
		mPointLightMan->GatherPointLightData(aabb, transform, plConst);
	}
	void RefreshPointLight(){
		mRefreshPointLight = true;
	}
	bool NeedToRefreshPointLight() const{
		return mRefreshPointLight;
	}

	void RenderDebugHud(){
		if (!mDebugHud)
			return;
		RenderEventMarker devent("RenderDebugHud");
		auto& observers = mObject->mObservers_[IRendererObserver::DefaultRenderEvent];
		for (auto it = observers.begin(); it != observers.end(); /**/)
		{
			auto observer = it->lock();
			if (!observer){
				it = observers.erase(it);
				continue;
			}
			++it;
			observer->BeforeDebugHudRendering( mMainWindowId, GetMainWindowHandle() );
		}

		RestoreRenderStates();
		RenderParam param;
		param.mRenderPass = PASS_NORMAL;
		param.mCamera = mCamera.get();
		mDebugHud->Render(param, 0);
		//SetWireframe(backup);		
		for (auto it = observers.begin(); it != observers.end(); /**/)
		{
			auto observer = it->lock();
			if (!observer){
				it = observers.erase(it);
				continue;
			}
			++it;
			observer->AfterDebugHudRendered(mMainWindowId, GetMainWindowHandle());
		}
	}

	//-------------------------------------------------------------------
	// GPU Manipulation
	//-------------------------------------------------------------------
	void SetClearColor(HWindowId id, const Color& color){
		auto it = mWindowRenderTargets.Find(id);
		if (it == mWindowRenderTargets.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "Cannot find the render target.");
			return;
		}
		it->second->SetClearColor(color);
	}

	void SetClearDepthStencil(HWindowId id, Real z, UINT8 stencil){
		auto it = mWindowRenderTargets.Find(id);
		if (it == mWindowRenderTargets.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "Cannot find the render target.");
			return;
		}
		it->second->SetClearDepthStencil(z, stencil);
	}

	void Clear(Real r, Real g, Real b, Real a, Real z, UINT8 stencil){		
		GetPlatformRenderer().Clear(r, g, b, a, z, stencil);
	}

	void Clear(Real r, Real g, Real b, Real a){
		GetPlatformRenderer().Clear(r, g, b, a);
	}

	// Avoid to use
	void ClearState(){
		GetPlatformRenderer().ClearState();
	}

	void BeginEvent(const char* name){
		GetPlatformRenderer().BeginEvent(name);
	}

	void EndEvent(){
		GetPlatformRenderer().EndEvent();
	}

	void TakeScreenshot(){
		auto filepath = GetNextScreenshotFile();
		GetPlatformRenderer().TakeScreenshot(filepath.c_str());
	}

	void ChangeFullscreenMode(int mode){
		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Changing fullscreen mode: %d", mode).c_str());
		GetPlatformRenderer().ChangeFullscreenMode(mMainWindowId, mWindowHandles[mMainWindowId], mode);
	}

	void OnWindowSizeChanged(HWindow window, const Vec2I& clientSize){
		if (mWindowSizeInternallyChanging)
			return;

		ChangeResolution(GetMainWindowHandle(), clientSize);
	}

	void ChangeResolution(HWindow window, const Vec2I& resol){
		if (window == GetMainWindowHandle())
			mRendererOptions->r_resolution = resol;

		auto handleId = GetWindowHandleId(window);
		IPlatformTexturePtr color, depth;
		bool success = GetPlatformRenderer().ChangeResolution(handleId, window, 
			resol, color, depth);
		if (success){
			auto rt = GetRenderTarget(handleId);
			rt->SetColorTexture(CreateTexture(color));
			rt->SetDepthTexture(CreateTexture(depth));
		}
	}

	void ChangeWindowSizeAndResolution(HWindow window, const Vec2I& resol){
		mWindowSizeInternallyChanging = true;
		ChangeWindowSize(window, resol);
		mWindowSizeInternallyChanging = false;
		ChangeResolution(window, resol);
	}

	void UnbindTexture(BINDING_SHADER shader, int slot){
		GetPlatformRenderer().UnbindTexture(shader, slot);
	}

	void UnbindInputLayout(){
		GetPlatformRenderer().UnbindInputLayout();
	}

	void UnbindVertexBuffers(){
		GetPlatformRenderer().SetVertexBuffers(0, 0, 0, 0, 0);
	}
	
	void UnbindShader(BINDING_SHADER shader){
		GetPlatformRenderer().UnbindShader(shader);
	}

	std::string GetScreenhotFolder(){
		auto appData = FileSystem::GetAppDataFolder();
		const char* screenShotFolder = "./ScreenShot/";
		auto screenShotFolderFull = FileSystem::ConcatPath(appData.c_str(), screenShotFolder);
		return screenShotFolderFull;
	}
	std::string GetNextScreenshotFile(){
		auto screenShotFolder = GetScreenhotFolder();
		if (!FileSystem::Exists(screenShotFolder.c_str())){
			bool created = FileSystem::CreateDirectory(screenShotFolder.c_str());
			if (!created){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to create folder %s", screenShotFolder.c_str()).c_str());
				return "";
			}
		}
		auto it = FileSystem::GetDirectoryIterator(screenShotFolder.c_str(), false);
		
		while (it->HasNext()){
			const char* filename = it->GetNextFilePath();
		}
	

		unsigned n = 0;		
		while (it->HasNext())
		{
			const char* filename = it->GetNextFilePath();
			std::regex match("screenshot_([0-9]+)\\.bmp");
			std::smatch result;			
			if (std::regex_match(std::string(filename), result, match)){
				if (result.size() == 2){
					std::ssub_match subMatch = result[1];
					std::string matchNumber = subMatch.str();
					unsigned thisn = StringConverter::ParseUnsignedInt(matchNumber);
					if (thisn >= n){
						n = thisn + 1;
					}
				}
			}			
		}
		return FormatString("Screenshot/screenshot_%d.bmp", n);
	}

	//-------------------------------------------------------------------
	// FBRenderer State
	//-------------------------------------------------------------------
	ResourceProviderPtr GetResourceProvider() const{
		return mResourceProvider;
	}
	void SetResourceProvider(ResourceProviderPtr provider){
		if (!provider)
			return;
		mResourceProvider = provider;
	}

	void SetForcedWireFrame(bool enable){
		if (mForcedWireframe != enable){
			if (enable)
			{
				auto wireFrameR = mResourceProvider->GetRasterizerState(ResourceTypes::RasterizerStates::WireFrame);
				if (wireFrameR)
					wireFrameR->Bind();
				mForcedWireframe = true;
			}
			else
			{
				mForcedWireframe = false;
				auto defaultR = mResourceProvider->GetRasterizerState(ResourceTypes::RasterizerStates::Default);
				if (defaultR)
					defaultR->Bind();
			}
		}
	}

	bool GetForcedWireFrame() const{
		return mForcedWireframe;
	}

	RenderTargetPtr GetMainRenderTarget() const{
		auto it = mWindowRenderTargets.Find(mMainWindowId);
		if (it == mWindowRenderTargets.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "No main window render target found.");
			return 0;
		}
		return it->second;
	}

	IScenePtr GetMainScene() const{
		auto rt = GetMainRenderTarget();
		if (rt){
			return rt->GetScene();
		}
		return 0;
	}
 // move to SceneManager
	const Vec2I& GetMainRenderTargetSize() const{
		auto rt = GetMainRenderTarget();
		if (rt)
		{
			return rt->GetSize();
		}
		return Vec2I::ZERO;
	}

	const Vec2I& GetCurrentRenderTargetSize() const{
		return mCurrentRenderTarget->GetSize();
	}

	void SetCurrentRenderTarget(RenderTargetPtr renderTarget){
		mCurrentRenderTarget = renderTarget;
	}

	RenderTargetPtr GetCurrentRenderTarget() const{
		return mCurrentRenderTarget;
	}

	void SetCurrentScene(IScenePtr scene){
		mCurrentScene = scene;
		UpdateSceneConstantsBuffer();
	}

	bool IsMainRenderTarget() const{
		return GetMainRenderTarget() == mCurrentRenderTarget;
	}

	const Vec2I& GetRenderTargetSize(HWindowId id = INVALID_HWND_ID) const{
		
		if (id != INVALID_HWND_ID){
			auto rt = GetRenderTarget(id);
			if (rt){
				return rt->GetSize();
			}
		}

		return mCurrentRTSize;
	}

	const Vec2I& GetRenderTargetSize(HWindow hwnd = 0) const{
		RenderTargetPtr rt = GetRenderTarget(hwnd);
		if (rt){
			return rt->GetSize();
		}
		return mCurrentRTSize;
	}

	
	void SetDirectionalLightInfo(int idx, const DirectionalLightInfo& info){
		mDirectionalLight[idx] = info;
	}

	void InitFrameProfiler(Real dt){
		mFrameProfiler.Clear();
		mFrameProfiler.UpdateFrameRate(dt);
	}

	const RENDERER_FRAME_PROFILER& GetFrameProfiler() const{
		return mFrameProfiler;
	}

	inline FontPtr GetFont(Real fontHeight) const{
		if (mFonts.empty()){
			return 0;
		}

		if (mFonts.size() == 1){
			auto it = mFonts.begin();
			it->second->SetHeight(fontHeight);
			return it->second;
		}

		int requestedHeight = Round(fontHeight);
		int bestMatchHeight = mFonts.begin()->first;
		int curGap = std::abs(requestedHeight - bestMatchHeight);
		FontPtr bestFont = mFonts.begin()->second;
		for (auto it : mFonts){
			auto newGap = std::abs(requestedHeight - it.first);
			if (newGap < curGap){
				bestMatchHeight = it.first;
				curGap = newGap;
				bestFont = it.second;
			}
		}
		if (!bestFont){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Font not found with size %f", fontHeight).c_str());			
		}
		else{
			bestFont->SetHeight(fontHeight);
		}
		return bestFont;
	}

	const INPUT_ELEMENT_DESCS& GetInputElementDesc(DEFAULT_INPUTS::Enum e){
		return mInputLayoutDescs[e];
	}

	void SetEnvironmentTexture(TexturePtr pTexture){
		mEnvironmentTexture = pTexture;
		SetSystemTexture(SystemTextures::Environment, mEnvironmentTexture);
	}

	void SetEnvironmentTextureOverride(TexturePtr texture){
		mEnvironmentTextureOverride = texture;
		if (mEnvironmentTextureOverride)
		{
			auto& bindings = GetSystemTextureBindings(SystemTextures::Environment);
			for (auto& binding : bindings){
				mEnvironmentTextureOverride->Bind(binding.mShader, binding.mSlot);
			}
		}
		else
		{
			if (mEnvironmentTexture)
			{
				auto& bindings = GetSystemTextureBindings(SystemTextures::Environment);
				for (auto& binding : bindings){
					mEnvironmentTexture->Bind(binding.mShader, binding.mSlot);
				}			
			}
			else
			{
				auto& bindings = GetSystemTextureBindings(SystemTextures::Environment);
				for (auto& binding : bindings){
					UnbindTexture(binding.mShader, binding.mSlot);
				}
			}
		}
	}

	void SetDebugRenderTarget(unsigned idx, const char* textureName){
		assert(idx < MaxDebugRenderTargets);
		auto mainRT = GetMainRenderTarget();
		assert(mainRT);
		if (_stricmp(textureName, "Shadow") == 0)
			mDebugRenderTargets[idx].mTexture = mainRT->GetShadowMap();
		else
			mDebugRenderTargets[idx].mTexture = 0;
	}

	void SetFadeAlpha(Real alpha){
		mFadeAlpha = alpha;
	}

	PointLightManagerPtr GetPointLightMan() const{
		return mPointLightMan;
	}

	void RegisterVideoPlayer(IVideoPlayerPtr player){
		if (!ValueExistsInVector(mVideoPlayers, player)){
			mVideoPlayers.push_back(player);
		}
	}

	void UnregisterVideoPlayer(IVideoPlayerPtr player){
		DeleteValuesInVector(mVideoPlayers, player);
	}

	bool GetSampleOffsets_Bloom(DWORD dwTexSize,
		float afTexCoordOffset[15],
		Vec4* avColorWeight,
		float fDeviation, float fMultiplier){
		// if deviation is big, samples tend to have more distance among them.
		int i = 0;
		float tu = 1.0f / (float)dwTexSize;

		// Fill the center texel
		float weight = fMultiplier * GaussianDistribution(0, 0, fDeviation);
		avColorWeight[7] = Vec4(weight, weight, weight, weight);

		afTexCoordOffset[7] = 0.0f;

		// Fill one side
		for (i = 1; i < 8; i++)
		{
			weight = fMultiplier * GaussianDistribution((float)i, 0, fDeviation);
			afTexCoordOffset[7 - i] = -i * tu;

			avColorWeight[7 - i] = Vec4(weight, weight, weight, weight);
		}

		// Copy to the other side
		for (i = 8; i < 15; i++)
		{
			avColorWeight[i] = avColorWeight[14 - i];
			afTexCoordOffset[i] = -afTexCoordOffset[14 - i];
		}

		// Debug convolution kernel which doesn't transform input data
		/*ZeroMemory( avColorWeight, sizeof(D3DXVECTOR4)*15 );
		avColorWeight[7] = D3DXVECTOR4( 1, 1, 1, 1 );*/

		return S_OK;
	}

	float GaussianDistribution(float x, float y, float rho)
	{
		//http://en.wikipedia.org/wiki/Gaussian_filter

		float g = 1.0f / sqrtf(2.0f * (float)PI * rho * rho);
		g *= expf(-(x * x + y * y) / (2.0f * rho * rho));

		return g;
	}

	void GetSampleOffsets_GaussBlur5x5(DWORD texWidth, DWORD texHeight, Vec4f** avTexCoordOffset, Vec4f** avSampleWeight, float fMultiplier){
		assert(avTexCoordOffset && avSampleWeight);
		auto it = mGauss5x5.Find(std::make_pair(texWidth, texHeight));
		if (it == mGauss5x5.end())
		{
			float tu = 1.0f / (float)texWidth;
			float tv = 1.0f / (float)texHeight;

			Vec4 vWhite(1.0, 1.0, 1.0, 1.0);
			std::vector<Vec4f> offsets;
			std::vector<Vec4f> weights;

			float totalWeight = 0.0;
			int index = 0;
			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					// Exclude pixels with a block distance greater than 2. This will
					// create a kernel which approximates a 5x5 kernel using only 13
					// sample points instead of 25; this is necessary since 2.0 shaders
					// only support 16 texture grabs.
					if (abs(x) + abs(y) > 2)
						continue;

					// Get the unscaled Gaussian intensity for this offset
					offsets.push_back(Vec4f(x * tu, y * tv, 0, 0));
					weights.push_back(Vec4f(vWhite * GaussianDistribution((float)x, (float)y, 1.0f)));
					totalWeight += weights.back().x;
					++index;
				}
			}
			assert(weights.size() == 13);
			// Divide the current weight by the total weight of all the samples; Gaussian
			// blur kernels add to 1.0f to ensure that the intensity of the image isn't
			// changed when the blur occurs. An optional multiplier variable is used to
			// add or remove image intensity during the blur.
			for (int i = 0; i < index; i++)
			{
				weights[i] /= totalWeight;
				weights[i] *= fMultiplier;
			}
			auto it = mGauss5x5.Insert(std::make_pair(std::make_pair(texWidth, texHeight), std::make_pair(offsets, weights)));
			*avTexCoordOffset = &(it->second.first[0]);
			*avSampleWeight = &(it->second.second[0]);
		}
		else
		{
			*avTexCoordOffset = &(it->second.first[0]);
			*avSampleWeight = &(it->second.second[0]);
		}
	}

	void GetSampleOffsets_DownScale2x2(DWORD texWidth, DWORD texHeight, Vec4f* avSampleOffsets){
		if (NULL == avSampleOffsets)
			return;

		float tU = 1.0f / texWidth;
		float tV = 1.0f / texHeight;

		// Sample from the 4 surrounding points. Since the center point will be in
		// the exact center of 4 texels, a 0.5f offset is needed to specify a texel
		// center.
		int index = 0;
		for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				avSampleOffsets[index].x = (x - 0.5f) * tU;
				avSampleOffsets[index].y = (y - 0.5f) * tV;

				index++;
			}
		}
	}

	bool IsLuminanceOnCpu() const{
		return mLuminanceOnCpu;
	}

	void SetLockDepthStencilState(bool lock){
		DepthStencilState::SetLock(lock);
	}

	void SetLockBlendState(bool lock){
		BlendState::SetLock(lock);
	}

	void SetFontTextureAtlas(const char* path){
		if (!ValidCStringLength(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return;
		}
		auto textureAtlas = GetTextureAtlas(path);
		if (!textureAtlas){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("No texture atlas(%s)", path).c_str());
			return;
		}
		for (auto font : mFonts){
			font.second->SetTextureAtlas(textureAtlas);
		}
	}

	void Update(TIME_PRECISION dt){
		mPointLightMan->Update(dt);
		// good point to reset.
		mRefreshPointLight = false;
	}

	//-------------------------------------------------------------------
	// Queries
	//-------------------------------------------------------------------
	unsigned GetMultiSampleCount() const{
		return 1;
	}

	bool GetFilmicToneMapping() const{
		return mUseFilmicToneMapping;
	}

	void SetFilmicToneMapping(bool use){
		mUseFilmicToneMapping = use;
	}

	bool GetLuminanaceOnCPU() const{
		return mLuminanceOnCpu;
	}

	void SetLuminanaceOnCPU(bool oncpu){
		mLuminance = oncpu;
	}

	RenderTargetPtr GetRenderTarget(HWindowId id) const{
		auto it = mWindowRenderTargets.Find(id);
		if (it != mWindowRenderTargets.end()){
			return it->second;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to find the render target for the window id(%u)", id).c_str());
		return 0;
	}

	RenderTargetPtr GetRenderTarget(HWindow hwnd) const{
		auto it = mWindowIds.Find(hwnd);
		if (it == mWindowIds.end()){
			Logger::Log(FB_ERROR_LOG_ARG, "Failed to find window Id");
			return 0;
		}
		return GetRenderTarget(it->second);
	}


	void SetCamera(CameraPtr pCamera){
		if (mCamera)
			mCamera->SetCurrent(false);
		mCamera = pCamera;
		if (mCamera){
			mCamera->SetCurrent(true);
			UpdateCameraConstantsBuffer();
		}
	}
	CameraPtr GetCamera() const{
		return mCamera;		
	} 

	CameraPtr GetMainCamera() const{
		auto rt = GetMainRenderTarget();
		if (rt)
			return rt->GetCamera();

		Logger::Log(FB_ERROR_LOG_ARG, "No main camera");
		return 0;

	}

	HWindow GetMainWindowHandle(){
		auto it = mWindowHandles.Find(mMainWindowId);
		if (it != mWindowHandles.end())
			return it->second;
		Logger::Log(FB_ERROR_LOG_ARG, "Cannot find maint window handle.");
		return INVALID_HWND;
	}

	HWindowId GetMainWindowHandleId() const{
		return mMainWindowId;
	}

	HWindow GetWindowHandle(HWindowId windowId){
		auto it = mWindowHandles.Find(windowId);
		if (it != mWindowHandles.end()){
			return it->second;
		}
		return INVALID_HWND;
	}

	HWindowId GetWindowHandleId(HWindow window){
		auto it = mWindowIds.Find(window);
		if (it != mWindowIds.end()){
			return it->second;
		}
		return INVALID_HWND_ID;
	}

	Vec2I ToSreenPos(HWindowId id, const Vec3& ndcPos) const{
		auto it = mWindowRenderTargets.Find(id);
		if (it == mWindowRenderTargets.end())
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Window id %u is not found.", id).c_str());			
			return Vec2I(0, 0);
		}
		const auto& size = it->second->GetSize();
		Vec2I ret;
		ret.x = (int)((size.x*.5) * ndcPos.x + size.x*.5);
		ret.y = (int)((-size.y*.5) * ndcPos.y + size.y*.5);
		return ret;
	}

	Vec2 ToNdcPos(HWindowId id, const Vec2I& screenPos) const{
		auto it = mWindowRenderTargets.Find(id);
		if (it == mWindowRenderTargets.end())
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Window id %u is not found.", id).c_str());			
			return Vec2(0, 0);
		}
		const auto& size = it->second->GetSize();
		Vec2 ret;
		ret.x = (Real)screenPos.x / (Real)size.x * 2.0f - 1.0f;
		ret.y = -((Real)screenPos.y / (Real)size.y * 2.0f - 1.0f);
		return ret;
	}

	unsigned GetNumLoadingTexture() const{
		return GetPlatformRenderer().GetNumLoadingTexture();
	}

	Vec2I FindClosestSize(HWindowId id, const Vec2I& input){
		return GetPlatformRenderer().FindClosestSize(id, input);
	}

	bool GetResolutionList(unsigned& outNum, Vec2I* list){
		std::shared_ptr<Vec2ITuple> tuples;
		if (list){
			tuples = std::shared_ptr<Vec2ITuple>(new Vec2ITuple[outNum], [](Vec2ITuple* obj){ delete [] obj; });
		}
		auto ret = GetPlatformRenderer().GetResolutionList(outNum, list ? tuples.get() : 0);
		if (ret && list){
			for (unsigned i = 0; i < outNum; ++i){
				list[i] = tuples.get()[i];
			}
		}
		return ret;
	}

	RendererOptionsPtr GetRendererOptions() const{
		return mRendererOptions;
	}

	void SetMainWindowStyle(unsigned style){
		mMainWindowStyle = style;
	}

	//-------------------------------------------------------------------
	// ISceneObserver
	//-------------------------------------------------------------------
	void OnAfterMakeVisibleSet(IScene* scene){

	}

	void OnBeforeRenderingOpaques(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut){
		
	}

	void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut){
		if (mDebugHud){
			mDebugHud->OnBeforeRenderingTransparents(scene, renderParam, renderParamOut);
		}

		if (mGeomRenderer){
			mGeomRenderer->Render(renderParam, renderParamOut);
		}

		BindDepthTexture(true);
	}
	
	//-------------------------------------------------------------------
	// IInputConsumer
	//-------------------------------------------------------------------
	void ConsumeInput(IInputInjectorPtr injector){
		mInputInfo.mCurrentMousePos = injector->GetMousePos();
		mInputInfo.mLButtonDown = injector->IsLButtonDown();
	}
};

static RendererWeakPtr sRenderer;
RendererPtr Renderer::Create(){
	if (sRenderer.expired()){
		auto renderer = RendererPtr(FB_NEW(Renderer), [](Renderer* obj){ FB_DELETE(obj); });
		renderer->mImpl->mSelf = renderer;
		sRenderer = renderer;
		return renderer;
	}
	return sRenderer.lock();
}

RendererPtr Renderer::Create(const char* renderEngineName){
	if (sRenderer.expired()){
		auto renderer = Create();		
		renderer->PrepareRenderEngine(renderEngineName);
		return renderer;
	}
	else{
		Logger::Log(FB_ERROR_LOG_ARG, "You can create only one renderer!");
		return sRenderer.lock();
	}
}

Renderer& Renderer::GetInstance(){
	return *sRenderer.lock();
}

bool Renderer::HasInstance(){
	return sRenderer.lock() != 0;
}

RendererPtr Renderer::GetInstancePtr(){
	return sRenderer.lock();
}

//---------------------------------------------------------------------------
Renderer::Renderer()
	: mImpl(new Impl(this))
{

}

Renderer::~Renderer(){
	Logger::Log(FB_DEFAULT_LOG_ARG, "Renderer deleted.");
}

bool Renderer::PrepareRenderEngine(const char* rendererPlugInName) {
	return mImpl->PrepareRenderEngine(rendererPlugInName);
}

//-------------------------------------------------------------------
// Canvas & System
//-------------------------------------------------------------------
bool Renderer::InitCanvas(HWindowId id, HWindow window, int width, int height) {
	return mImpl->InitCanvas(id, window, width, height);
}

void Renderer::ReleaseCanvas(HWindowId id) {
	mImpl->ReleaseCanvas(id);
}

void Renderer::Render() {
	mImpl->Render();
}

//-------------------------------------------------------------------
// Resource Creation
//-------------------------------------------------------------------
RenderTargetPtr Renderer::CreateRenderTarget(const RenderTargetParam& param) {
	return mImpl->CreateRenderTarget(param);
}

void Renderer::KeepRenderTargetInPool(RenderTargetPtr rt) {
	mImpl->KeepRenderTargetInPool(rt);
}

TexturePtr Renderer::CreateTexture(const char* file){
	return mImpl->CreateTexture(file, true);
}

TexturePtr Renderer::CreateTexture(const char* file, bool async) {
	return mImpl->CreateTexture(file, async);
}

TexturePtr Renderer::CreateTexture(void* data, int width, int height, PIXEL_FORMAT format, BUFFER_USAGE usage, int  buffer_cpu_access, int texture_type) {
	return mImpl->CreateTexture(data, width, height, format, usage, buffer_cpu_access, texture_type);
}

VertexBufferPtr Renderer::CreateVertexBuffer(void* data, unsigned stride, unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) {
	return mImpl->CreateVertexBuffer(data, stride, numVertices, usage, accessFlag);
}

IndexBufferPtr Renderer::CreateIndexBuffer(void* data, unsigned int numIndices, INDEXBUFFER_FORMAT format) {
	return mImpl->CreateIndexBuffer(data, numIndices, format);
}

ShaderPtr Renderer::CreateShader(const char* filepath, int shaders){
	return mImpl->CreateShader(filepath, shaders, SHADER_DEFINES());
}

ShaderPtr Renderer::CreateShader(const char* filepath, int shaders, const SHADER_DEFINES& defines) {
	return mImpl->CreateShader(filepath, shaders, defines);
}

bool Renderer::ReapplyShaderDefines(Shader* shader) {
	return mImpl->ReapplyShaderDefines(shader);
}

MaterialPtr Renderer::CreateMaterial(const char* file) {
	return mImpl->CreateMaterial(file);
}

// use this if you are sure there is instance of the descs.
InputLayoutPtr Renderer::CreateInputLayout(const INPUT_ELEMENT_DESCS& descs, ShaderPtr shader) {
	return mImpl->CreateInputLayout(descs, shader);
}

InputLayoutPtr Renderer::GetInputLayout(DEFAULT_INPUTS::Enum e, ShaderPtr shader) {
	return mImpl->GetInputLayout(e, shader);
}

RasterizerStatePtr Renderer::CreateRasterizerState(const RASTERIZER_DESC& desc) {
	return mImpl->CreateRasterizerState(desc);
}

BlendStatePtr Renderer::CreateBlendState(const BLEND_DESC& desc) {
	return mImpl->CreateBlendState(desc);
}

DepthStencilStatePtr Renderer::CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc) {
	return mImpl->CreateDepthStencilState(desc);
}

SamplerStatePtr Renderer::CreateSamplerState(const SAMPLER_DESC& desc) {
	return mImpl->CreateSamplerState(desc);
}

TextureAtlasPtr Renderer::GetTextureAtlas(const char* path) {
	return mImpl->GetTextureAtlas(path);
}

TextureAtlasRegionPtr Renderer::GetTextureAtlasRegion(const char* path, const char* region) {
	return mImpl->GetTextureAtlasRegion(path, region);
}

TexturePtr Renderer::GetTemporalDepthBuffer(const Vec2I& size) {
	return mImpl->GetTemporalDepthBuffer(size);
}

PointLightPtr Renderer::CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion) {
	return mImpl->CreatePointLight(pos, range, color, intensity, lifeTime, manualDeletion);
}

//-------------------------------------------------------------------
// Hot reloading
//-------------------------------------------------------------------
//bool Renderer::ReloadShader(ShaderPtr shader) {
//	return mImpl->ReloadShader(shader);
//}
//
//bool Renderer::ReloadTexture(ShaderPtr shader) {
//	return mImpl->ReloadTexture(shader);
//}

//-------------------------------------------------------------------
// Resource Bindings
//-------------------------------------------------------------------
void Renderer::SetRenderTarget(TexturePtr pRenderTargets[], size_t rtViewIndex[], int num, TexturePtr pDepthStencil, size_t dsViewIndex) {
	mImpl->SetRenderTarget(pRenderTargets, rtViewIndex,  num, pDepthStencil, dsViewIndex);
}

void Renderer::UnbindRenderTarget(TexturePtr renderTargetTexture) {
	mImpl->UnbindRenderTarget(renderTargetTexture);
}

void Renderer::SetViewports(const Viewport viewports[], int num) {
	mImpl->SetViewports(viewports, num);
}

void Renderer::SetScissorRects(Rect rects[], int num) {
	mImpl->SetScissorRects(rects, num);
}

void Renderer::SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers, VertexBufferPtr pVertexBuffers[], unsigned int strides[], unsigned int offsets[]) {
	mImpl->SetVertexBuffers(startSlot, numBuffers, pVertexBuffers, strides, offsets);
}

void Renderer::SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt) {
	mImpl->SetPrimitiveTopology(pt);
}

void Renderer::SetTextures(TexturePtr pTextures[], int num, BINDING_SHADER shaderType, int startSlot) {
	mImpl->SetTextures(pTextures, num, shaderType, startSlot);
}

void Renderer::SetSystemTexture(SystemTextures::Enum type, TexturePtr texture) {
	mImpl->SetSystemTexture(type, texture);
}

void Renderer::UnbindTexture(BINDING_SHADER shader, int slot) {
	mImpl->UnbindTexture(shader, slot);
}

void Renderer::UnbindShader(BINDING_SHADER shader){
	mImpl->UnbindShader(shader);
}

void Renderer::UnbindInputLayout(){
	mImpl->UnbindInputLayout();
}

void Renderer::UnbindVertexBuffers(){
	mImpl->UnbindVertexBuffers();
}

// pre defined
void Renderer::BindDepthTexture(bool set) {
	mImpl->BindDepthTexture(set);
}

void Renderer::SetDepthWriteShader() {
	mImpl->SetDepthWriteShader();
}

void Renderer::SetDepthWriteShaderCloud() {
	mImpl->SetDepthWriteShaderCloud();
}

void Renderer::SetPositionInputLayout() {
	mImpl->SetPositionInputLayout();
}

void Renderer::SetSystemTextureBindings(SystemTextures::Enum type, const TextureBindings& bindings) {
	mImpl->SetSystemTextureBindings(type, bindings);
}

const TextureBindings& Renderer::GetSystemTextureBindings(SystemTextures::Enum type) const {
	return mImpl->GetSystemTextureBindings(type);
}

//-------------------------------------------------------------------
// Device RenderStates
//-------------------------------------------------------------------
void Renderer::RestoreRenderStates() {
	mImpl->RestoreRenderStates();
}

void Renderer::RestoreRasterizerState() {
	mImpl->RestoreRasterizerState();
}

void Renderer::RestoreBlendState() {
	mImpl->RestoreBlendState();
}

void Renderer::RestoreDepthStencilState() {
	mImpl->RestoreDepthStencilState();
}

// sampler
void Renderer::SetSamplerState(int ResourceTypes_SamplerStates, BINDING_SHADER shader, int slot) {
	mImpl->SetSamplerState(ResourceTypes_SamplerStates, shader, slot);
}

//-------------------------------------------------------------------
// GPU constants
//-------------------------------------------------------------------
void Renderer::UpdateObjectConstantsBuffer(const void* pData){
	mImpl->UpdateObjectConstantsBuffer(pData, false);
}

void Renderer::UpdateObjectConstantsBuffer(const void* pData, bool record) {
	mImpl->UpdateObjectConstantsBuffer(pData, record);
}

void Renderer::UpdatePointLightConstantsBuffer(const void* pData) {
	mImpl->UpdatePointLightConstantsBuffer(pData);
}

void Renderer::UpdateFrameConstantsBuffer() {
	mImpl->UpdateFrameConstantsBuffer();
}

void Renderer::UpdateMaterialConstantsBuffer(const void* pData) {
	mImpl->UpdateMaterialConstantsBuffer(pData);
}

void Renderer::UpdateCameraConstantsBuffer() {
	mImpl->UpdateCameraConstantsBuffer();
}

void Renderer::UpdateRenderTargetConstantsBuffer() {
	mImpl->UpdateRenderTargetConstantsBuffer();
}

void Renderer::UpdateSceneConstantsBuffer() {
	mImpl->UpdateSceneConstantsBuffer();
}

void Renderer::UpdateRareConstantsBuffer() {
	mImpl->UpdateRareConstantsBuffer();
}

void Renderer::UpdateRadConstantsBuffer(const void* pData) {
	mImpl->UpdateRadConstantsBuffer(pData);
}

void* Renderer::MapMaterialParameterBuffer() {
	return mImpl->MapMaterialParameterBuffer();
}

void Renderer::UnmapMaterialParameterBuffer() {
	mImpl->UnmapMaterialParameterBuffer();
}

void* Renderer::MapBigBuffer() {
	return mImpl->MapBigBuffer();
}

void Renderer::UnmapBigBuffer() {
	mImpl->UnmapBigBuffer();
}

//-------------------------------------------------------------------
// GPU Manipulation
//-------------------------------------------------------------------
void Renderer::SetClearColor(HWindowId id, const Color& color) {
	mImpl->SetClearColor(id, color);
}

void Renderer::SetClearDepthStencil(HWindowId id, Real z, UINT8 stencil) {
	mImpl->SetClearDepthStencil(id, z, stencil);
}

void Renderer::Clear(Real r, Real g, Real b, Real a, Real z, UINT8 stencil) {
	mImpl->Clear(r, g, b, a, z, stencil);
}

void Renderer::Clear(Real r, Real g, Real b, Real a) {
	mImpl->Clear(r, g, b, a);
}

// Avoid to use
void Renderer::ClearState() {
	mImpl->ClearState();
}

void Renderer::BeginEvent(const char* name) {
	mImpl->BeginEvent(name);
}

void Renderer::EndEvent() {
	mImpl->EndEvent();
}

void Renderer::TakeScreenshot() {
	mImpl->TakeScreenshot();
}

void Renderer::ChangeFullscreenMode(int mode){
	mImpl->ChangeFullscreenMode(mode);
}

void Renderer::OnWindowSizeChanged(HWindow window, const Vec2I& clientSize){
	mImpl->OnWindowSizeChanged(window, clientSize);
}

void Renderer::ChangeResolution(const Vec2I& resol){
	mImpl->ChangeResolution(GetMainWindowHandle(), resol);
}

void Renderer::ChangeResolution(HWindow window, const Vec2I& resol){
	mImpl->ChangeResolution(window, resol);
}

void Renderer::ChangeWindowSizeAndResolution(const Vec2I& resol){
	mImpl->ChangeWindowSizeAndResolution(GetMainWindowHandle(), resol);
}

void Renderer::ChangeWindowSizeAndResolution(HWindow window, const Vec2I& resol){
	mImpl->ChangeWindowSizeAndResolution(window, resol);
}

//-------------------------------------------------------------------
// FBRenderer State
//-------------------------------------------------------------------
ResourceProviderPtr Renderer::GetResourceProvider() const{
	return mImpl->GetResourceProvider();
}

void Renderer::SetResourceProvider(ResourceProviderPtr provider){
	mImpl->SetResourceProvider(provider);
}

void Renderer::SetForcedWireFrame(bool enable) {
	mImpl->SetForcedWireFrame(enable);
}

bool Renderer::GetForcedWireFrame() const {
	return mImpl->GetForcedWireFrame();
}

RenderTargetPtr Renderer::GetMainRenderTarget() const {
	return mImpl->GetMainRenderTarget();
}

IScenePtr Renderer::GetMainScene() const {
	return mImpl->GetMainScene();
}

const Vec2I& Renderer::GetMainRenderTargetSize() const {
	return mImpl->GetMainRenderTargetSize();
}

void Renderer::SetCurrentRenderTarget(RenderTargetPtr renderTarget) {
	mImpl->SetCurrentRenderTarget(renderTarget);
}

RenderTargetPtr Renderer::GetCurrentRenderTarget() const {
	return mImpl->GetCurrentRenderTarget();
}

void Renderer::SetCurrentScene(IScenePtr scene){
	mImpl->SetCurrentScene(scene);
}

bool Renderer::IsMainRenderTarget() const {
	return mImpl->IsMainRenderTarget();
}

const Vec2I& Renderer::GetRenderTargetSize(HWindowId id) const {
	return mImpl->GetRenderTargetSize(id);
}

const Vec2I& Renderer::GetRenderTargetSize(HWindow hwnd) const {
	return mImpl->GetRenderTargetSize(hwnd);
}

void Renderer::SetDirectionalLightInfo(int idx, const DirectionalLightInfo& info){
	mImpl->SetDirectionalLightInfo(idx, info);
}

void Renderer::InitFrameProfiler(Real dt) {
	mImpl->InitFrameProfiler(dt);
}

const RENDERER_FRAME_PROFILER& Renderer::GetFrameProfiler() const {
	return mImpl->GetFrameProfiler();
}

inline FontPtr Renderer::GetFont(Real fontHeight) const {
	return mImpl->GetFont(fontHeight);
}

const INPUT_ELEMENT_DESCS& Renderer::GetInputElementDesc(DEFAULT_INPUTS::Enum e) {
	return mImpl->GetInputElementDesc(e);
}

void Renderer::SetEnvironmentTexture(TexturePtr pTexture) {
	mImpl->SetEnvironmentTexture(pTexture);
}

void Renderer::SetEnvironmentTextureOverride(TexturePtr texture) {
	mImpl->SetEnvironmentTextureOverride(texture);
}

void Renderer::SetDebugRenderTarget(unsigned idx, const char* textureName) {
	mImpl->SetDebugRenderTarget(idx, textureName);
}

void Renderer::SetFadeAlpha(Real alpha) {
	mImpl->SetFadeAlpha(alpha);
}

PointLightManagerPtr Renderer::GetPointLightMan() const {
	return mImpl->GetPointLightMan();
}

void Renderer::RegisterVideoPlayer(IVideoPlayerPtr player) {
	mImpl->RegisterVideoPlayer(player);
}

void Renderer::UnregisterVideoPlayer(IVideoPlayerPtr player) {
	mImpl->UnregisterVideoPlayer(player);
}

bool Renderer::GetSampleOffsets_Bloom(DWORD dwTexSize,
	float afTexCoordOffset[15],
	Vec4* avColorWeight,
	float fDeviation, float fMultiplier){
	return mImpl->GetSampleOffsets_Bloom(dwTexSize, afTexCoordOffset, avColorWeight, fDeviation, fMultiplier);
}

void Renderer::GetSampleOffsets_GaussBlur5x5(DWORD texWidth, DWORD texHeight, Vec4f** avTexCoordOffset, Vec4f** avSampleWeight, float fMultiplier) {
	mImpl->GetSampleOffsets_GaussBlur5x5(texWidth, texHeight, avTexCoordOffset, avSampleWeight, fMultiplier);
}

void Renderer::GetSampleOffsets_DownScale2x2(DWORD texWidth, DWORD texHeight, Vec4f* avSampleOffsets) {
	mImpl->GetSampleOffsets_DownScale2x2(texWidth, texHeight, avSampleOffsets);
}

bool Renderer::IsLuminanceOnCpu() const {
	return mImpl->IsLuminanceOnCpu();
}

void Renderer::SetLockDepthStencilState(bool lock){
	mImpl->SetLockDepthStencilState(lock);
}

void Renderer::SetLockBlendState(bool lock){
	mImpl->SetLockBlendState(lock);
}

void Renderer::SetFontTextureAtlas(const char* path){
	mImpl->SetFontTextureAtlas(path);
}

void Renderer::Update(TIME_PRECISION dt){
	mImpl->Update(dt);
}

//-------------------------------------------------------------------
// Queries
//-------------------------------------------------------------------
unsigned Renderer::GetMultiSampleCount() const {
	return mImpl->GetMultiSampleCount();
}

bool Renderer::GetFilmicToneMapping() const{
	return mImpl->GetFilmicToneMapping();
}

void Renderer::SetFilmicToneMapping(bool use){
	mImpl->SetFilmicToneMapping(use);
}

bool Renderer::GetLuminanaceOnCPU() const{
	return mImpl->GetLuminanaceOnCPU();
}

void Renderer::SetLuminanaceOnCPU(bool oncpu){
	mImpl->SetLuminanaceOnCPU(oncpu);
}

RenderTargetPtr Renderer::GetRenderTarget(HWindowId id) const {
	return mImpl->GetRenderTarget(id);
}

void Renderer::SetCamera(CameraPtr pCamera) {
	mImpl->SetCamera(pCamera);
}

CameraPtr Renderer::GetCamera() const {
	return mImpl->GetCamera();
}

CameraPtr Renderer::GetMainCamera() const {
	return mImpl->GetMainCamera();
}

HWindow Renderer::GetMainWindowHandle() const {
	return mImpl->GetMainWindowHandle();
}

HWindowId Renderer::GetMainWindowHandleId(){
	return mImpl->GetMainWindowHandleId();
}

HWindow Renderer::GetWindowHandle(HWindowId windowId){
	return mImpl->GetWindowHandle(windowId);
}

HWindowId Renderer::GetWindowHandleId(HWindow window){
	return mImpl->GetWindowHandleId(window);
}

Vec2I Renderer::ToSreenPos(HWindowId id, const Vec3& ndcPos) const {
	return mImpl->ToSreenPos(id, ndcPos);
}

Vec2 Renderer::ToNdcPos(HWindowId id, const Vec2I& screenPos) const {
	return mImpl->ToNdcPos(id, screenPos);
}

unsigned Renderer::GetNumLoadingTexture() const {
	return mImpl->GetNumLoadingTexture();
}

Vec2I Renderer::FindClosestSize(HWindowId id, const Vec2I& input) {
	return mImpl->FindClosestSize(id, input);
}

bool Renderer::GetResolutionList(unsigned& outNum, Vec2I* list) {
	return mImpl->GetResolutionList(outNum, list);
}

RendererOptionsPtr Renderer::GetRendererOptions() const {
	return mImpl->GetRendererOptions();
}

void Renderer::SetMainWindowStyle(unsigned style){
	mImpl->SetMainWindowStyle(style);
}

//-------------------------------------------------------------------
// Drawing
//-------------------------------------------------------------------
void Renderer::DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) {
	mImpl->DrawIndexed(indexCount, startIndexLocation, startVertexLocation);
}

void Renderer::Draw(unsigned int vertexCount, unsigned int startVertexLocation) {
	mImpl->Draw(vertexCount, startVertexLocation);
}

void Renderer::DrawFullscreenQuad(ShaderPtr pixelShader, bool farside) {
	mImpl->DrawFullscreenQuad(pixelShader, farside);
}

void Renderer::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec4& color, MaterialPtr mat) {
	mImpl->DrawTriangle(a, b, c, color, mat);
}

void Renderer::DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color){
	mImpl->DrawQuad(pos, size, color, true);
}

void Renderer::DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color, bool updateRs) {
	mImpl->DrawQuad(pos, size, color, updateRs);
}

void Renderer::DrawQuadWithTexture(const Vec2I& pos, const Vec2I& size, const Color& color, TexturePtr texture, MaterialPtr materialOverride) {
	mImpl->DrawQuadWithTexture(pos, size, color, texture, materialOverride);
}

void Renderer::DrawQuadWithTextureUV(const Vec2I& pos, const Vec2I& size, const Vec2& uvStart, const Vec2& uvEnd, const Color& color, TexturePtr texture, MaterialPtr materialOverride) {
	mImpl->DrawQuadWithTextureUV(pos, size, uvStart, uvEnd, color, texture, materialOverride);
}

void Renderer::DrawBillboardWorldQuad(const Vec3& pos, const Vec2& size, const Vec2& offset, DWORD color, MaterialPtr pMat) {
	mImpl->DrawBillboardWorldQuad(pos, size, offset, color, pMat);
}

void Renderer::QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color){
	mImpl->QueueDrawText(pos, text, color, defaultFontSize);
}

void Renderer::QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color, Real size) {
	mImpl->QueueDrawText(pos, text, color, size);
}

void Renderer::QueueDrawText(const Vec2I& pos, const char* text, const Color& color){
	mImpl->QueueDrawText(pos, text, color, defaultFontSize);
}

void Renderer::QueueDrawText(const Vec2I& pos, const char* text, const Color& color, Real size) {
	mImpl->QueueDrawText(pos, text, color, size);
}

void Renderer::QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color){
	mImpl->QueueDraw3DText(worldpos, text, color, defaultFontSize);
}

void Renderer::QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, Real size) {
	mImpl->QueueDraw3DText(worldpos, text, color, size);
}

void Renderer::QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color){
	mImpl->QueueDraw3DText(worldpos, text, color, defaultFontSize);
}

void Renderer::QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color, Real size) {
	mImpl->QueueDraw3DText(worldpos, text, color, size);
}

void Renderer::QueueDrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text, const Color& color){
	mImpl->QueueDrawTextForDuration(secs, pos, text, color, defaultFontSize);
}

void Renderer::QueueDrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text, const Color& color, Real size) {
	mImpl->QueueDrawTextForDuration(secs, pos, text, color, size);
}

void Renderer::QueueDrawTextForDuration(Real secs, const Vec2I& pos, const char* text, const Color& color){
	mImpl->QueueDrawTextForDuration(secs, pos, text, color, defaultFontSize);
}

void Renderer::QueueDrawTextForDuration(Real secs, const Vec2I& pos, const char* text, const Color& color, Real size) {
	mImpl->QueueDrawTextForDuration(secs, pos, text, color, size);
}

void Renderer::ClearDurationTexts() {
	mImpl->ClearDurationTexts();
}

void Renderer::QueueDrawLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1) {
	mImpl->QueueDrawLine(start, end, color0, color1);
}

void Renderer::QueueDrawLine(const Vec2I& start, const Vec2I& end, const Color& color0, const Color& color1) {
	mImpl->QueueDrawLine(start, end, color0, color1);
}

void Renderer::QueueDrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1) {
	mImpl->QueueDrawLineBeforeAlphaPass(start, end, color0, color1);
}

void Renderer::QueueDrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color) {
	mImpl->QueueDrawQuad(pos, size, color);
}

void Renderer::QueueDrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness, const char* texture, bool textureFlow) {
	mImpl->QueueDrawTexturedThickLine(start, end, color0, color1, thickness, texture, textureFlow);
}

void Renderer::QueueDrawSphere(const Vec3& pos, Real radius, const Color& color) {
	mImpl->QueueDrawSphere(pos, radius, color);
}

void Renderer::QueueDrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha) {
	mImpl->QueueDrawBox(boxMin, boxMax, color, alpha);
}

void Renderer::QueueDrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha) {
	mImpl->QueueDrawTriangle(a, b, c, color, alpha);
}

void Renderer::QueueDrawQuadLine(const Vec2I& pos, const Vec2I& size, const Color& color) {
	mImpl->QueueDrawQuadLine(pos, size, color);
}

//-------------------------------------------------------------------
// Internal
//-------------------------------------------------------------------
void Renderer::GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst) {
	mImpl->GatherPointLightData(aabb, transform, plConst);
}

void Renderer::RefreshPointLight() {
	mImpl->RefreshPointLight();
}

bool Renderer::NeedToRefreshPointLight() const {
	return mImpl->NeedToRefreshPointLight();
}

void Renderer::RenderDebugHud() {
	mImpl->RenderDebugHud();
}

//-------------------------------------------------------------------
// ISceneObserver
//-------------------------------------------------------------------
void Renderer::OnAfterMakeVisibleSet(IScene* scene) {
	mImpl->OnAfterMakeVisibleSet(scene);
}

void Renderer::OnBeforeRenderingOpaques(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	mImpl->OnBeforeRenderingOpaques(scene, renderParam, renderParamOut);
}

void Renderer::OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	mImpl->OnBeforeRenderingTransparents(scene, renderParam, renderParamOut);
}

//-------------------------------------------------------------------
// ISceneObserver
//-------------------------------------------------------------------
void Renderer::ConsumeInput(IInputInjectorPtr injector) {
	mImpl->ConsumeInput(injector);
}

 /// inject to main camera