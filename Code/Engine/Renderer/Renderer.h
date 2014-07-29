#pragma once
#include <Engine/IRenderer.h>

namespace fastbird
{

class ILight;
class DebugHud;
class IFont;
class RenderToTexture;

class Renderer : public IRenderer
{
public:
	Renderer();
	virtual ~Renderer();

	bool OnPrepared();
	void OnDeinit();
	
	virtual void ProcessRenderToTexture();

	virtual Vec2I ToSreenPos(const Vec3& ndcPos) const;
	virtual Vec2 ToNdcPos(const Vec2I& screenPos) const;
	virtual unsigned GetWidth() const { return mWidth; }
	virtual unsigned GetHeight() const { return mHeight; }
	//virtual void SetWireframe(bool enable); // see RendererD3D11
	virtual bool GetWireframe() const { return mForcedWireframe; }
	virtual void SetClearColor(float r, float g, float b, float a=1.f);
	virtual void SetClearDepthStencil(float z, UINT8 stencil);
	virtual void SetCamera(ICamera* pCamera);
	virtual ICamera* GetCamera() const;
	virtual void InitFrameProfiler(float dt);
	virtual const RENDERER_FRAME_PROFILER& GetFrameProfiler() const;
	virtual void DrawText(const Vec2I& pos, WCHAR* text, const Color& color);
	virtual void DrawText(const Vec2I& pos, const char* text, const Color& color);
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color);
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
		const Color& color);
	virtual void DrawLine(const Vec3& start, const Vec3& end, 
		const Color& color0, const Color& color1);
	virtual void DrawLine(const Vec2I& start, const Vec2I& end, 
		const Color& color0, const Color& color1);
	virtual void RenderDebugHud(); 
	virtual inline IFont* GetFont() const;
	
	virtual const INPUT_ELEMENT_DESCS& GetInputElementDesc(
		DEFAULT_INPUTS::Enum e);

	// use this if you are sure there is instance of the descs.
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs);

	// use these if you are not sure.
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
		IMaterial* material) = 0;
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
		IShader* shader) = 0;
	// auxiliary
	virtual IInputLayout* GetInputLayout(DEFAULT_INPUTS::Enum e,
		IMaterial* material);
	virtual IInputLayout* GetInputLayout(DEFAULT_INPUTS::Enum e,
		IShader* shader);

	virtual TextureAtlas* GetTextureAtlas(const char* path);
	virtual TextureAtlasRegion* GetTextureAtlasRegion(const char* path, const char* region);

	virtual IMaterial* CreateMaterial(const char* file);
	virtual IMaterial* GetMissingMaterial();

	virtual void SetDirectionalLight(ILight* pLight);

	virtual int BindFullscreenQuadUV_VB(bool farSide);

	virtual void SetEnvironmentTexture(ITexture* pTexture);


protected:
	unsigned				mWidth;
	unsigned				mHeight;

	SmartPtr<ILight>		mDirectionalLight;
	SmartPtr<ILight>		mDirectionalLightOverride;
	SmartPtr<IVertexBuffer>	mVBQuadUV_Far;
	SmartPtr<IVertexBuffer>	mVBQuadUV_Near;
	SmartPtr<DebugHud>		mDebugHud;
	SmartPtr<IFont> mFont;
	SmartPtr<IMaterial> mMaterials[DEFAULT_MATERIALS::COUNT];
	SmartPtr<IMaterial> mMissingMaterial;
	SmartPtr<ITexture> mEnvironmentTexture;
	std::vector< SmartPtr<RenderToTexture> > mRenderToTextures;

	Color mClearColor;
	float mDepthClear;
	UINT8 mStencilClear;
	bool					mForcedWireframe;
	bool					mDepthStencilCreated;

	ICamera*				mCamera;
	RENDERER_FRAME_PROFILER		mFrameProfiler;
	IShader* mBindedShader;
	IInputLayout* mBindedInputLayout;
	PRIMITIVE_TOPOLOGY mCurrentTopology;

	typedef std::map<INPUT_ELEMENT_DESCS, SmartPtr<IInputLayout> > INPUTLAYOUT_MAP;
	INPUTLAYOUT_MAP mInputLayouts;

	
	INPUT_ELEMENT_DESCS mInputLayoutDescs[DEFAULT_INPUTS::COUNT];
	const int DEFAULT_DYN_VERTEX_COUNTS;
	SmartPtr<IVertexBuffer> mDynVBs[DEFAULT_INPUTS::COUNT];

	typedef VectorMap< std::string, SmartPtr<ITexture> > TextureCache;
	TextureCache mTextureCache;

	typedef std::vector< SmartPtr<TextureAtlas> > TextureAtlasCache;
	TextureAtlasCache mTextureAtalsCache;

	typedef VectorMap< std::string, SmartPtr<IMaterial> > MaterialCache;
	MaterialCache mMaterialCache;
};

inline bool operator < (const INPUT_ELEMENT_DESCS& left, const INPUT_ELEMENT_DESCS& right)
{
	if (left.size() < right.size())
		return true;
	else if (left.size() == right.size())
	{
		DWORD size = left.size();
		for (DWORD i=0; i<size; i++)
		{
			if (left[i].mInputSlot < right[i].mInputSlot)
				return true;
			else if (left[i].mInputSlot == right[i].mInputSlot)
			{
				if (left[i] < right[i])
					return true;
			}
		}
	}
	
	return false;
}

}