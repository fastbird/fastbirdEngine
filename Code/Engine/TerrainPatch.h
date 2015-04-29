#pragma once 
#ifndef _fastbird_TerrainPatch_header_included_
#define _fastbird_TerrainPatch_header_included_

#include <Engine/SpatialObject.h>
#include <CommonLib/Math/Vec2I.h>

struct FIBITMAP;
namespace fastbird
{
	class IVertexBuffer;
	class IIndexBuffer;
	class IShader;
	class Terrain;

	//------------------------------------------------------------------------
	class TerrainPatch : public SpatialObject
	{
	public:
		TerrainPatch();
		virtual ~TerrainPatch();

		//------------------------------------------------------------------------
		// IObject
		//------------------------------------------------------------------------
		virtual void Render();
		virtual void PreRender();
		virtual void PostRender();
		virtual void SetMaterial(IMaterial* pMat, int pass = 0)
		{ 
			mMaterial = pMat; 
		}

		//------------------------------------------------------------------------
		// OWN
		//------------------------------------------------------------------------
		void Build(int idx, int idy, int numVertX, float distance, FIBITMAP* pImage, Terrain* pTerrain);
		int GetLOD() const { return mLOD; }
		void SetUpPatch(TerrainPatch* pPatch);
		void SetRightPatch(TerrainPatch* pPatch);
		void SetDownPatch(TerrainPatch* pPatch);
		void SetLeftPatch(TerrainPatch* pPatch);
		inline unsigned GetLastUpdatedFrame() const { return mLastUpdatedFrame; }

	private:
		int mLOD;
		SmartPtr<IVertexBuffer> mVertexBuffer;
		SmartPtr<IVertexBuffer> mVertexBuffer2;
		SmartPtr<IIndexBuffer> mIndexBuffer;
		SmartPtr<IMaterial> mMaterial;
		TerrainPatch* mUpPatch;
		TerrainPatch* mRightPatch;
		TerrainPatch* mDownPatch;
		TerrainPatch* mLeftPatch;
		unsigned mLastUpdatedFrame;

	};
}
#endif //_fastbird_TerrainPatch_header_included_