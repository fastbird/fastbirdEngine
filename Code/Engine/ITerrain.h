#pragma once
#ifndef _ITerrain_header_included_
#define _ITerrain_header_included_

#include <CommonLib/SmartPtr.h>

namespace fastbird
{
	class IIndexBuffer;
	class ITerrain : public ReferenceCounter
	{
	public:
		// numVertX * numVertY = (2^n+1) * (2^m+1)
		static ITerrain* CreateTerrainInstance(int numVertX, int numVertY, float distance, const char* heightmapFile);

		virtual ~ITerrain() {}

		virtual IIndexBuffer* GetIndexBufferLOD(int lod, int diffFlag) const = 0;

		virtual void Update() = 0;
	};
}

#endif //_ITerrain_header_included_