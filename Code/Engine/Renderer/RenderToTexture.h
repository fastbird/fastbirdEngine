#pragma once
#include <Engine/IRenderToTexture.h>
#include <Engine/ICamera.h>
namespace fastbird
{
	class RenderToTexture : public IRenderToTexture
	{
	public:
		RenderToTexture();
		virtual ~RenderToTexture(){}
		
		const Vec2I& GetSize() const;
		virtual bool CheckOptions(const Vec2I& size, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap, bool hasDepth);

		virtual IScene* GetScene() const { return mScene;}
		virtual ICamera* GetCamera() const{ return mCamera; }
		virtual ILight* GetLight(int idx);

		virtual ITexture* GetRenderTargetTexture() { return mRenderTargetTexture; }
		virtual ITexture* GetDepthStencilTexture() { return mDepthStencilTexture; }
		virtual void SetClearValues(const Color& color, float z, UINT8 stencil);

		virtual void Bind(size_t face = 0);
		virtual void Render(size_t face=0);
		virtual void Unbind();

		virtual void SetEnable(bool enable){ mEnabled = enable; }
		virtual bool GetEnable() const { return mEnabled; }

		virtual void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);

	protected:
		Vec2I mSize;
		PIXEL_FORMAT mFormat;
		bool mSRV;
		bool mMiplevel;
		bool mCubeMap;
		bool mHasDepth;
		SmartPtr<ITexture> mRenderTargetTexture;
		SmartPtr<ITexture> mDepthStencilTexture;
		SmartPtr<IScene> mScene;
		SmartPtr<ICamera> mCamera;
		SmartPtr<ILight> mLight[2];
		Color mClearColor;
		float mDepthClear;
		UINT8 mStencilClear;
		bool mEnabled;
	};
}