#include <Engine/StdAfx.h>
#include <Engine/TextureD3D11.h>
#include <Engine/RendererD3D11.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>

DEFINE_GUID(WKPDID_D3DDebugObjectName, 0x429b8c22, 0x9188, 0x4b0c, 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00);

using namespace fastbird;

//----------------------------------------------------------------------------
TextureD3D11* TextureD3D11::CreateInstance()
{
	TextureD3D11* pTexture = FB_NEW(TextureD3D11)();
	return pTexture;
}

void TextureD3D11::Delete()
{
	FB_DELETE(this);
}

//----------------------------------------------------------------------------
TextureD3D11::TextureD3D11()
	: mTexture(0)
	, mSRView(0)
	//, mSamplerState(0)
	, mSlot(0)
	, mSRViewParent(0)
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
		for (auto v : mRTViews)
		{
			pRenderer->OnReleaseRenderTarget(v);
			SAFE_RELEASE(v);
		}		
		for (auto v : mDSViews)
		{
			pRenderer->OnReleaseDepthStencil(v);
			SAFE_RELEASE(v);
		}
	}

	//SAFE_RELEASE(mSamplerState);
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
unsigned TextureD3D11::GetWidth() const
{
	return mImageInfo.Width;
}

//----------------------------------------------------------------------------
unsigned TextureD3D11::GetHeight() const
{
	return mImageInfo.Height;
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
		mSRView = *mSRViewParent;
		mSRView->AddRef();
		mSRViewParent = 0;
	}
	return mSRView;
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
	for (auto v : mRTViews)
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
	for (auto v : mDSViews)
	{
		RendererD3D11* pRenderer = static_cast<RendererD3D11*>(gFBEnv->pRenderer);
		pRenderer->OnReleaseDepthStencil(v);
		SAFE_RELEASE(v);
	}

	mDSViews.clear();
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
	Texture* pAdam = (Texture*)this;
	while (pAdam->GetAdamTexture())
	{
		pAdam = pAdam->GetAdamTexture();
	}
	pNewTexture->SetAdamTexture(pAdam);
	
	if (!pNewTexture->mSRView)
	{
		TextureD3D11* pT = (TextureD3D11*)pAdam;
		assert(pT);
		pNewTexture->mSRViewParent = (ID3D11ShaderResourceView**)&pT->mSRView;
	}
	/*pNewTexture->mSamplerState = mSamplerState; if (mSamplerState) mSamplerState->AddRef();
	pNewTexture->mSamplerDesc = mSamplerDesc;*/
	pNewTexture->mPixelFormat = mPixelFormat;
	pNewTexture->mName = mName;
	pNewTexture->mType = mType;
	
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

void TextureD3D11::SetDebugName(const char* name)
{
	if (mTexture)
		mTexture->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	if (mSRView)
	{
		char buff[255];
		sprintf_s(buff, "%s SRV", name);
		mSRView->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buff), buff);
	}
	int i = 0;
	for (auto it : mRTViews)
	{
		char buff[255];
		sprintf_s(buff, "%s RTV %d", name, i++);
		it->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buff), buff);
	}

	i = 0;
	for (auto it : mDSViews)
	{
		char buff[255];
		sprintf_s(buff, "%s DSV %d", name, i++);
		it->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(buff), buff);
	}
}

void TextureD3D11::GenerateMips()
{
	gFBEnv->pRenderer->GenerateMips(this);
}

void TextureD3D11::OnReloaded()
{
	// only called for adam texture
	assert(!mAdamTexture);

	FB_FOREACH(it, Texture::mTextures)
	{
		TextureD3D11* pT = (TextureD3D11*)*it;
		if (pT->GetAdamTexture() == this)
		{
			SAFE_RELEASE(pT->mSRView);
			pT->mSRViewParent = (ID3D11ShaderResourceView**)&mSRView;
		}
	}
}