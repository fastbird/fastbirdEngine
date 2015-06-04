#pragma once
#ifndef _Terrain_header_included_
#define _Terrain_header_included_

#include <Engine/ITerrain.h>
#include <Engine/IInputListener.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Color.h>


struct FIBITMAP;

namespace fastbird
{
	class IVertexBuffer;
	class IIndexBuffer;
	class IShader;
	class TerrainPatch;
	class IMaterial;
	class IInputLayout;

	class Terrain : public ITerrain, IInputListener
	{
	public:
		enum LOD_DIFF_FLAG
		{
			LOD_DIFF_FLAG_NONE = 0,
			// one
			LOD_DIFF_FLAG_LEFT = 1,
			LOD_DIFF_FLAG_UP = 1 << 1,
			LOD_DIFF_FLAG_RIGHT = 1 << 2,
			LOD_DIFF_FLAG_DOWN = 1 << 3,
			
			// two
			LOD_DIFF_FLAG_LEFT_UP = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_UP,
			LOD_DIFF_FLAG_LEFT_RIGHT = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_RIGHT,
			LOD_DIFF_FLAG_LEFT_DOWN = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_DOWN,			
			LOD_DIFF_FLAG_UP_RIGHT = LOD_DIFF_FLAG_UP | LOD_DIFF_FLAG_RIGHT,
			LOD_DIFF_FLAG_UP_DOWN = LOD_DIFF_FLAG_UP | LOD_DIFF_FLAG_DOWN,			
			LOD_DIFF_FLAG_RIGHT_DOWN = LOD_DIFF_FLAG_RIGHT | LOD_DIFF_FLAG_DOWN,

			// three
			LOD_DIFF_FLAG_LEFT_UP_RIGHT = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_UP | LOD_DIFF_FLAG_RIGHT,
			LOD_DIFF_FLAG_LEFT_RIGHT_DOWN = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_RIGHT | LOD_DIFF_FLAG_DOWN,
			LOD_DIFF_FLAG_LEFT_UP_DOWN = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_UP | LOD_DIFF_FLAG_DOWN,
			LOD_DIFF_FLAG_UP_RIGHT_DOWN = LOD_DIFF_FLAG_UP | LOD_DIFF_FLAG_RIGHT | LOD_DIFF_FLAG_DOWN,

			// all
			LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN = LOD_DIFF_FLAG_LEFT | LOD_DIFF_FLAG_UP | LOD_DIFF_FLAG_RIGHT | LOD_DIFF_FLAG_DOWN
		};

		// number of vertices for one axis in a patch
		static const int PATCH_NUM_VERT;
		static ITerrain* sTerrain;
		static inline float ConvertHeight(int hf)
		{
			const float MAX_HEIGHT=30.0f;
			float s = hf / 255.0f;
			return s*MAX_HEIGHT;
		}

		//--------------------------------------------------------------------
		static const DWORD NORMAL_CACHE_VERSION=1;

		//--------------------------------------------------------------------
		Terrain();
		virtual ~Terrain();

	protected:
		virtual void FinishSmartPtr();

	public:
		//--------------------------------------------------------------------
		// ITerrain Interfaces
		//--------------------------------------------------------------------
		virtual void Init(int numVertX, int numVertY, float distance, const char* heightmapFile);
		virtual IIndexBuffer* GetIndexBufferLOD(int lod, int diffFlag) const;
		virtual void Update();

		//--------------------------------------------------------------------
		// IInputListener
		//--------------------------------------------------------------------
		virtual void OnInput(IMouse* pMouse, IKeyboard* pKeyboard);
		virtual void EnableInputListener(bool enable);
		virtual bool IsEnabledInputLIstener() const;

		//--------------------------------------------------------------------
		// OWN Interfaces
		//--------------------------------------------------------------------
		inline void GetData(int x, int y, Vec3& outNormal, Vec3& outTangent, Vec3& outBinormal) const
		{
			int idx = y*mNumVertX + x;
			assert(idx<(int)mNormals.size());
			outNormal = mNormals[idx];
			outTangent = mTangents[idx];
			outBinormal = mBinormals[idx];
		}

		inline void GetNormal(int x, int y, Vec3& outNormal) const
		{
			int idx = y*mNumVertX + x;
			assert(idx<(int)mNormals.size());
			outNormal = mNormals[idx];
		}
		

	private:
		void GenerateNormals(FIBITMAP* pImage, const char* filename);
		void CreateRefinedIndexBuffers();
		unsigned int ConvertDiffFlag(LOD_DIFF_FLAG flag) const;
		std::string ConvertDiffFlagToString(LOD_DIFF_FLAG flag) const;
		

	private:
		SmartPtr<IVertexBuffer> mVertexBuffer;
		SmartPtr<IIndexBuffer> mIndexBuffers[5];
		SmartPtr<IIndexBuffer> mIndexBuffersRefined[4][15];
		SmartPtr<IMaterial> mMaterial;
		std::vector< SmartPtr<TerrainPatch> > mPatches;
		std::vector<Vec3> mNormals;
		std::vector<Vec3> mTangents;
		std::vector<Vec3> mBinormals;
		Vec2I mNumPatches;
		float mDistance;
		int mNumVertX;
		int mNumVertY;
		bool mDebugging;
		int mDebuggingLOD;
		int mDebuggingFlagLOD;
		LOD_DIFF_FLAG mDebuggingFlag;
		int mDebuggingFlagIndex;

		int mSelectedBar;

		Color mDirtColor[2];
		Color mGrassColor[2];
		float mGrassLerp;
		Color mRockColor[2];

		bool mInputListenerEnabled;
	};
}


#endif //_Terrain_header_included_