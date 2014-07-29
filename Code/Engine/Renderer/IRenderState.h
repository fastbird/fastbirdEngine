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
	};

	//------------------------------------------------------------------------
	class IBlendState : public ReferenceCounter
	{
	public:
		virtual ~IBlendState() {}
		virtual void Bind() = 0;
	};

	//------------------------------------------------------------------------
	class IDepthStencilState : public ReferenceCounter
	{
	public:
		virtual ~IDepthStencilState() {}
		virtual void Bind(unsigned stencilRef) = 0;
	};
}