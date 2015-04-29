#pragma once
#include <Engine/SpatialObject.h>
namespace fastbird
{
	class IShader;
	class ParticleRenderObject : public SpatialObject
	{
	public:

		struct Key
		{
			Key()
				: mGlow(false), mDepthFade(true)
			{
				memset(mTexturePath, 0, 256);
			}
			Key(const char* texturePath, const BLEND_DESC& desc, bool glow, bool depthFade)
				:mGlow(glow), mBDesc(desc), mDepthFade(depthFade)
			{
				strcpy(mTexturePath, texturePath);
			}

			bool operator==(const Key& other) const
			{
				return memcmp(this, &other, sizeof(Key)) == 0;
			}

			bool operator<(const Key& other) const
			{
				return memcmp(this, &other, sizeof(Key)) < 0;
			}

			char mTexturePath[256];
			BLEND_DESC mBDesc;
			bool mGlow;
			bool mDepthFade;
		};
		static const int MAX_SHARED_VERTICES;
		static ParticleRenderObject* GetRenderObject(Key& key, bool& created);
		static void ClearParticles();
		static void EndUpdateParticles();
		static void FinalizeRenderObjects();
		static size_t GetNumRenderObject();
		static size_t GetNumDrawCalls();
		static size_t GetNumPrimitives();

		ParticleRenderObject();
		virtual ~ParticleRenderObject();

		virtual IMaterial* GetMaterial(int pass = 0) const;
		virtual void PreRender(){}
		virtual void Render();
		virtual void PostRender();

		void Clear();
		void EndUpdate();
		void SetDoubleSided(bool set);
		void SetTexture(const char* texturePath);

		struct Vertex
		{
			Vec3 mPos;
			Vec4 mUDirection_Intensity;
			Vec3 mVDirection;
			Vec4 mPivot_Size;
			Vec4 mRot_Alpha_uv;
			Vec2 mUVStep;
			DWORD mColor;
		};

		Vertex* Map(UINT numVertices, unsigned& canWrite);


	private:
		typedef VectorMap< Key, SmartPtr<ParticleRenderObject> > RENDER_OBJECTS;
		static RENDER_OBJECTS mRenderObjects;
		SmartPtr<IMaterial> mMaterial;
		static size_t mNumDrawCalls;
		static size_t mNumDrawPrimitives;
		unsigned mMaxVertices;
		unsigned mLastFrameNumVertices;
		std::string mTextureName;

		SmartPtr<IVertexBuffer> mVertexBuffer;
		UINT mNextMap;
		typedef std::vector< std::pair<UINT, UINT> > BATCHES;
		BATCHES mBatches;
		Vertex* mMapped;
		bool mDoubleSided;
	};
}