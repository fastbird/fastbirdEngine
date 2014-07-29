#pragma once
#ifndef __Rendererd3d11_header_included__
#define __Rendererd3d11_header_included__

#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/Shaders/Constants.h>
#include <Engine/Renderer/RendererStructs.h>
#include <Engine/Renderer/D3D11/RenderStateD3D11.h>
#include <CommonLib/Color.h>
#include <D3DX11.h>

namespace fastbird
{
	class IObject;
	class ICamera;
	class TextureD3D11;

	class RendererD3D11 : public Renderer
	{
	public:
		RendererD3D11();
		virtual ~RendererD3D11();
		virtual bool Init(int threadPool);
		virtual int InitSwapChain(HWND hwnd, int width, int height);
		virtual void Deinit();
		virtual void Clear(float r, float g, float b, float a, float z, UINT8 stencil);
		virtual void Clear();
		virtual void UpdateFrameConstantsBuffer();
		virtual void UpdateObjectConstantsBuffer(void* pData);
		virtual void UpdateMaterialConstantsBuffer(void* pData);
		virtual void UpdateRareConstantsBuffer();
		virtual void* MapMaterialParameterBuffer();
		virtual void UnmapMaterialParameterBuffer();

		virtual IRenderToTexture* CreateRenderToTexture(bool everyframe);
		virtual void DeleteRenderToTexture(IRenderToTexture*);
		
		virtual void SetWireframe(bool enable);
		virtual void Present();
		virtual void SetVertexBuffer(unsigned int startSlot, unsigned int numBuffers,
			IVertexBuffer* pVertexBuffers[], unsigned int strides[], unsigned int offsets[]);
		virtual void SetIndexBuffer(IIndexBuffer* pIndexBuffer);
		virtual void SetTexture(ITexture* pTexture, BINDING_SHADER shaderType, unsigned int slot);
		virtual void GenerateMips(ITexture* pTexture);
		virtual IVertexBuffer* CreateVertexBuffer(void* data, unsigned stride, unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag);
		virtual IIndexBuffer* CreateIndexBuffer(void* data, unsigned int numIndices, INDEXBUFFER_FORMAT format);
		virtual IShader* CreateShader(const char* filepath, int shaders,
			const IMaterial::SHADER_DEFINES& defines, IShader* pReloadingShader=0);
		virtual ITexture* CreateTexture(const Vec2I& size, int mipLevels, int arraySize);
		virtual ITexture* CreateTexture(const char* file, ITexture* pReloadingTexture=0);
		virtual ITexture* CreateTexture(void* data, int width, int height, PIXEL_FORMAT format,
			BUFFER_USAGE usage, int  buffer_cpu_access, int  type);

		virtual void SetRenderTarget(ITexture* pRenderTarget[], size_t rtIndex[], int num, 
			ITexture* pDepthStencil, size_t dsIndex);
		virtual void RestoreRenderTarget();
		void OnReleaseRenderTarget(ID3D11RenderTargetView* pRTView);
		void OnReleaseDepthStencil(ID3D11DepthStencilView* pDSView);
		virtual void SetViewports(Viewport viewports[], int num);
		virtual void RestoreViewports();
		virtual void SetScissorRects(RECT rects[], int num);
		virtual void RestoreScissorRects();

		
		virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
			IMaterial* material);
		virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
			IShader* shader);

		virtual void SetShader(IShader* pShader);
		virtual void SetInputLayout(IInputLayout* pInputLayout);
		virtual void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt);
		virtual void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation);
		virtual void Draw(unsigned int vertexCount, unsigned int startVertexLocation);

		virtual MapData MapVertexBuffer(IVertexBuffer* pBuffer, UINT subResource, 
			MAP_TYPE type, MAP_FLAG flag);
		virtual void UnmapVertexBuffer(IVertexBuffer* pBuffer, UINT subResource);

		virtual MapData MapTexture(ITexture* pTexture, UINT subResource, 
			MAP_TYPE type, MAP_FLAG flag);
		virtual void UnmapTexture(ITexture* pTexture, UINT subResource);

		virtual void CopyToStaging(ITexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
			ITexture* src, UINT srcSubresource, Box3D* pBox);

		virtual void SaveTextureToFile(ITexture* texture, const char* filename);

		virtual void SetTextureSamplerState(ITexture* pTexture, const SAMPLER_DESC& desc);

		virtual IRasterizerState* CreateRasterizerState(const RASTERIZER_DESC& desc);
		virtual IBlendState* CreateBlendState(const BLEND_DESC& desc);
		virtual IDepthStencilState* CreateDepthStencilState( const DEPTH_STENCIL_DESC& desc );

		// internal
		virtual void SetRasterizerState(IRasterizerState* pRasterizerState);
		virtual void SetBlendState(IBlendState* pBlendState);
		virtual void SetDepthStencilState(IDepthStencilState* pDepthStencilState, unsigned stencilRef);

		virtual void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color);

	private:
		IInputLayout* RendererD3D11::CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,
			void* byteCode, int byteLength);
		MapData MapBuffer(ID3D11Resource* pResource, 
			UINT subResource, MAP_TYPE type, MAP_FLAG flag);

	private:
		ID3D11Device*			m_pDevice; // free-threaded
		IDXGIFactory1*			m_pFactory;
		IDXGISwapChain*			m_pSwapChain;
		ID3D11DeviceContext*	m_pImmediateContext; //  not free-threaded
		ID3D11RenderTargetView* m_pRenderTargetView;
		ID3D11Texture2D*		m_pDepthStencil;
		ID3D11DepthStencilView* m_pDepthStencilView;
		D3D11_VIEWPORT			mViewPort;
		ID3D11Buffer*			m_pFrameConstantsBuffer;
		ID3D11Buffer*			m_pObjectConstantsBuffer;
		ID3D11Buffer*			m_pMaterialConstantsBuffer;
		ID3D11Buffer*			m_pMaterialParametersBuffer;
		ID3D11Buffer*			m_pRareConstantsBuffer;
		ID3D11Buffer*			m_pImmutableConstantsBuffer;
		ID3D11RasterizerState*	m_pWireframeRasterizeState;
		ID3DX11ThreadPump*		m_pThreadPump;

		DXGI_SAMPLE_DESC		mMultiSampleDesc;

		D3D_DRIVER_TYPE			mDriverType;
		D3D_FEATURE_LEVEL		mFeatureLevel;
		DXGI_FORMAT				mDepthStencilFormat;

		FRAME_CONSTANTS			mFrameConstants;
		
		std::map<SAMPLER_DESC, ID3D11SamplerState*> mSamplerMap;

		typedef std::map<RASTERIZER_DESC, SmartPtr<RasterizerStateD3D11> > RASTERIZER_MAP;
		RASTERIZER_MAP mRasterizerMap;
		typedef std::map<BLEND_DESC, SmartPtr<BlendStateD3D11> > BLEND_MAP;
		BLEND_MAP mBlendMap;
		typedef std::map<DEPTH_STENCIL_DESC, SmartPtr<DepthStencilStateD3D11> > DEPTH_STENCIL_MAP;
		DEPTH_STENCIL_MAP mDepthStencilMap;

		std::vector<IDXGISwapChain*> mSwapChains;
		std::vector<ID3D11RenderTargetView*> mRenderTargetViews;
		std::vector<ID3D11RenderTargetView*> mCurrentRTViews;
		std::vector<ID3D11DepthStencilView*> mDepthStencilViews;
		ID3D11DepthStencilView* mCurrentDSView;
		std::vector<D3D11_VIEWPORT> mViewports;

		std::vector<TextureD3D11*> mCheckTextures;
		
		std::vector<ID3D11Texture2D*> mRenderTargetTextures;
	};
}

#endif //__Rendererd3d11_header_included__