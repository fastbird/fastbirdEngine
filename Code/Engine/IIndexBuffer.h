#pragma once
#ifndef _IIndexBuffer_header_included_
#define _IIndexBuffer_header_included_

#include <CommonLib/SmartPtr.h>

namespace fastbird
{
	enum INDEXBUFFER_FORMAT
	{
		INDEXBUFFER_FORMAT_16BIT,
		INDEXBUFFER_FORMAT_32BIT,
	};

	class IIndexBuffer : public ReferenceCounter
	{
	public:
		virtual ~IIndexBuffer(){}
		virtual bool IsReady() const = 0;
		virtual inline unsigned GetNumIndices() const = 0;
		virtual INDEXBUFFER_FORMAT GetFormat() const = 0;
		virtual void Bind() = 0;
	};
}
#endif//_IIndexBuffer_header_included_