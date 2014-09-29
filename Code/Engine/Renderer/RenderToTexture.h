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

		virtual IScene* GetScene() const { return mScene;}
		virtual ICamera* GetCamera() const{ return mCamera; }
		virtual ILight* GetLight();

		virtual ITexture* GetRenderTargetTexture() { return mRenderTargetTexture; }
		virtual ITexture* GetDepthStencilTexture() { return mDepthStencilTexture; }
		virtual void SetClearValues(const Color& color, float z, UINT8 stencil);

		virtual void Render(size_t face=0);

		virtual void SetEnable(bool enable){ mEnabled = enable; }
		virtual bool GetEnable() const { return mEnabled; }

		virtual void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);

	protected:
		SmartPtr<ITexture> mRenderTargetTexture;
		SmartPtr<ITexture> mDepthStencilTexture;
		SmartPtr<IScene> mScene;
		SmartPtr<ICamera> mCamera;
		SmartPtr<ILight> mLight;
		Color mClearColor;
		float mDepthClear;
		UINT8 mStencilClear;
		bool mEnabled;
	};
}