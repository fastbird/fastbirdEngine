#pragma once
#include <CommonLib/SmartPtr.h>
namespace fastbird
{
	//------------------------------------------------------------------------
	class IRasterizerState : public ReferenceCounter
	{
	public:
		virtual ~IRasterizerState() {}
		virtual void Bind() = 0;

		virtual void SetDebugName(const char* name) = 0;
	};

	//------------------------------------------------------------------------
	class IBlendState : public ReferenceCounter
	{
	public:
		virtual ~IBlendState() {}
		virtual void Bind() = 0;
		virtual void SetDebugName(const char* name) = 0;
	};

	//------------------------------------------------------------------------
	class IDepthStencilState : public ReferenceCounter
	{
	public:
		virtual ~IDepthStencilState() {}
		virtual void Bind(unsigned stencilRef) = 0;
		virtual void SetDebugName(const char* name) = 0;
	};


	//------------------------------------------------------------------------
	class ISamplerState : public ReferenceCounter
	{
	public:
		virtual ~ISamplerState() {}
		virtual void Bind(BINDING_SHADER shader, int slot) = 0;
		virtual void SetDebugName(const char* name) = 0;
	};

	//------------------------------------------------------------------------
	class RenderStates : public ReferenceCounter
	{
	private:
		
		SmartPtr<IRasterizerState> mRasterizerState;
		SmartPtr<IBlendState> mBlendState;
		SmartPtr<IDepthStencilState> mDepthStencilState;

		RASTERIZER_DESC mRDesc;
		BLEND_DESC mBDesc;
		DEPTH_STENCIL_DESC mDDesc;


	public:		

		RenderStates();

	protected:
		virtual void FinishSmartPtr();

	public:
		void Reset();
		void ResetRasterizerState();
		void ResetBlendState();
		void ResetDepthStencilState();
		void CreateRasterizerState(const RASTERIZER_DESC& desc);
		void CreateBlendState(const BLEND_DESC& desc);
		void CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		void Bind(unsigned stencilRef = 0);

		RenderStates* Clone() const;

	}; 
}