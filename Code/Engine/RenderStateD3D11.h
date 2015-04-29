#pragma once
#include <Engine/IRenderState.h>
#include <d3d11.h>

namespace fastbird
{
	//-------------------------------------------------------------------------
	class RasterizerStateD3D11 : public IRasterizerState
	{
	public:
		RasterizerStateD3D11();
		virtual ~RasterizerStateD3D11();

		//--------------------------------------------------------------------
		// IRasterizerState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind();
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------
		void SetHardwareRasterizerState(ID3D11RasterizerState* pRasterizerState);
		ID3D11RasterizerState* GetHardwareRasterizerState() const { return mRasterizerState; }

	private:
		ID3D11RasterizerState* mRasterizerState;
	};

	//-------------------------------------------------------------------------
	class BlendStateD3D11 : public IBlendState
	{
	public:
		BlendStateD3D11();
		virtual ~BlendStateD3D11();

		//--------------------------------------------------------------------
		// IBlendState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind();
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------
		void SetHardwareBlendState(ID3D11BlendState* pBlendState);
		ID3D11BlendState* GetHardwareBlendState() const { return mBlendState; }
		float* GetBlendFactor() const { return 0; }
		DWORD GetBlendMask() const { return 0xffffffff;}

	private:
		ID3D11BlendState* mBlendState;
	};

	//-------------------------------------------------------------------------
	class DepthStencilStateD3D11 : public IDepthStencilState
	{
	public:
		DepthStencilStateD3D11();
		virtual ~DepthStencilStateD3D11();

		//--------------------------------------------------------------------
		// IDepthStencilState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind(unsigned stencilRef);
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------
		void SetHardwareDSState(ID3D11DepthStencilState* pDepthStencilState);
		ID3D11DepthStencilState* GetHardwareDSState() const { return mDepthStencilState; }

	private:
		ID3D11DepthStencilState* mDepthStencilState;
	};

	class SamplerStateD3D11 : public ISamplerState
	{
	public:
		SamplerStateD3D11();
		virtual ~SamplerStateD3D11();

		//--------------------------------------------------------------------
		// ISamplerState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind(BINDING_SHADER shader, int slot);
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------
		void SetHardwareSamplerState(ID3D11SamplerState* pSamplerState);
		ID3D11SamplerState* GetHardwareSamplerState() const { return mSamplerState; }

	private:
		ID3D11SamplerState* mSamplerState;
	};
}