#include <Engine/StdAfx.h>
#include <Engine/Renderer/D3D11/TextureD3D11.h>
#include <Engine/Renderer/D3D11/RendererD3D11.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>

using namespace fastbird;

//----------------------------------------------------------------------------
TextureD3D11* TextureD3D11::CreateInstance()
{
	TextureD3D11* pTexture = new TextureD3D11();
	return pTexture;
}

//----------------------------------------------------------------------------
TextureD3D11::TextureD3D11()
	: mTexture(0)
	, mSRView(0)
	, mSamplerState(0)
	, mSlot(0)
	, mSRViewParent(0)
	, mAdamTexture(0)
	, mBindingShader(BINDING_SHADER_PS)
{
	mHr = S_FALSE;
	mLoadInfo.pSrcInfo = &mImageInfo;
				
	mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
}

//----------------------------------------------------------------------------
TextureD3D11::~TextureD3D11()
{
	if (gFBEnv && gFBEnv->pRenderer)
	{
		RendererD3D11* pRenderer = static_cast<RendererD3D11*>(gFBEnv->pRenderer);
		for each(auto v in mRTViews)
		{
			pRenderer->OnReleaseRenderTarget(v);
			SAFE_RELEASE(v);
		}		
		for each(auto v in mDSViews)
		{
			pRenderer->OnReleaseDepthStencil(v);
			SAFE_RELEASE(v);
		}
	}

	SAFE_RELEASE(mSamplerState);
	SAFE_RELEASE(mSRView);	
	SAFE_RELEASE(mTexture);	
}

//----------------------------------------------------------------------------
// ITexture
//----------------------------------------------------------------------------
bool TextureD3D11::IsReady() const
{
	return mSRView!=0;
}

//----------------------------------------------------------------------------
Vec2I TextureD3D11::GetSize() const
{
	return Vec2I(mImageInfo.Width, mImageInfo.Height);
}

//----------------------------------------------------------------------------
PIXEL_FORMAT TextureD3D11::GetFormat() const
{
	return mPixelFormat;
}

//----------------------------------------------------------------------------
void TextureD3D11::Bind()
{
	gFBEnv->pRenderer->SetTexture(this, mBindingShader, mSlot);
}

void TextureD3D11::Unbind()
{
	gFBEnv->pRenderer->SetTexture(0, mBindingShader, mSlot);
}

//----------------------------------------------------------------------------
void TextureD3D11::SetSlot(int slot)
{
	mSlot = slot;
}

//----------------------------------------------------------------------------
void TextureD3D11::SetShaderStage(BINDING_SHADER shader)
{
	mBindingShader = shader;
}

//----------------------------------------------------------------------------
void TextureD3D11::SetSamplerDesc(const SAMPLER_DESC& desc)
{
	__super::SetSamplerDesc(desc);
	gFBEnv->pRenderer->SetTextureSamplerState((ITexture*)this, desc);
}

//----------------------------------------------------------------------------
// OWN
//----------------------------------------------------------------------------
ID3D11Texture2D* TextureD3D11::GetHardwareTexture() const
{
	return mTexture;
}

//----------------------------------------------------------------------------
ID3D11ShaderResourceView* TextureD3D11::GetHardwareResourceView()
{
	if (!mSRView && mSRViewParent && *mSRViewParent)
	{
		(*mSRViewParent)->AddRef();
		mSRView = *mSRViewParent;
		mSRViewParent = 0;
	}
	return mSRView;
}

//----------------------------------------------------------------------------
ID3D11SamplerState* TextureD3D11::GetSamplerState() const
{
	return mSamplerState;
}

//----------------------------------------------------------------------------
void TextureD3D11::SetHardwareTexture(ID3D11Texture2D* pTexture)
{
	SAFE_RELEASE(mTexture);
	mTexture = pTexture;
}
//----------------------------------------------------------------------------
void TextureD3D11::SetHardwareResourceView(ID3D11ShaderResourceView* pResourceView)
{
	SAFE_RELEASE(mSRView);
	mSRView = pResourceView;
}

//----------------------------------------------------------------------------
void TextureD3D11::AddRenderTargetView(ID3D11RenderTargetView* pRenderTargetView)
{
	mRTViews.push_back(pRenderTargetView);
}

void TextureD3D11::ClearRenderTargetViews()
{
	for each(auto v in mRTViews)
	{
		RendererD3D11* pRenderer = static_cast<RendererD3D11*>(gFBEnv->pRenderer);
		pRenderer->OnReleaseRenderTarget(v);
		SAFE_RELEASE(v);
	}
	mRTViews.clear();
}

void TextureD3D11::AddDepthStencilView(ID3D11DepthStencilView* pDepthStencilView)
{
	mDSViews.push_back(pDepthStencilView);
}

void TextureD3D11::ClearDepthStencilViews()
{
	for each(auto v in mDSViews)
	{
		RendererD3D11* pRenderer = static_cast<RendererD3D11*>(gFBEnv->pRenderer);
		pRenderer->OnReleaseDepthStencil(v);
		SAFE_RELEASE(v);
	}

	mDSViews.clear();
}

//----------------------------------------------------------------------------
void TextureD3D11::SetSamplerState(ID3D11SamplerState* pSamplerState)
{
	SAFE_RELEASE(mSamplerState);
	mSamplerState = pSamplerState;
}

void TextureD3D11::SetSize(const Vec2I& size)
{
	mImageInfo.Width = size.x;
	mImageInfo.Height = size.y;
}

ITexture* TextureD3D11::Clone() const
{
	// we don't clone render target and depth stencil.
	assert(mRTViews.empty() && mDSViews.empty());

	TextureD3D11* pNewTexture = TextureD3D11::CreateInstance();
	pNewTexture->mTexture = mTexture; if (mTexture) mTexture->AddRef();
	pNewTexture->mSRView = mSRView; if (mSRView) mSRView->AddRef();
	const TextureD3D11* pAdam = this;
	while(pAdam->mAdamTexture)
	{
		pAdam = pAdam->mAdamTexture;
	}
	pNewTexture->mAdamTexture = (TextureD3D11*)pAdam;
	
	if (!pNewTexture->mSRView)
	{
		pNewTexture->mSRViewParent = (ID3D11ShaderResourceView**)&pAdam->mSRView;
	}
	pNewTexture->mSamplerState = mSamplerState; if (mSamplerState) mSamplerState->AddRef();
	pNewTexture->mPixelFormat = mPixelFormat;
	pNewTexture->mName = mName;
	pNewTexture->mType = mType;
	pNewTexture->mSamplerDesc = mSamplerDesc;
	pNewTexture->mHr = mHr;
	pNewTexture->mImageInfo = mImageInfo;
	pNewTexture->mLoadInfo = mLoadInfo;
	
	return pNewTexture;
}

void TextureD3D11::CopyToStaging(ITexture* dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ,
			UINT srcSubresource, Box3D* srcBox)
{
	gFBEnv->pRenderer->CopyToStaging(dst, dstSubresource, dstX, dstY, dstZ,
		this, srcSubresource, srcBox);
		
}

void TextureD3D11::SaveToFile(const char* filename)
{
	gFBEnv->pRenderer->SaveTextureToFile(this, filename);
}

ID3D11RenderTargetView* TextureD3D11::GetRenderTargetView(int idx) const
{
	assert((size_t)idx < mRTViews.size());
	return mRTViews[idx];
}
ID3D11DepthStencilView* TextureD3D11::GetDepthStencilView(int idx) const
{
	assert((size_t)idx < mDSViews.size());
	return mDSViews[idx];
}

void TextureD3D11::GenerateMips()
{
	gFBEnv->pRenderer->GenerateMips(this);
}