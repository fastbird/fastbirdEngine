#pragma once
#include <Engine/SceneGraph/SpatialObject.h>
namespace fastbird
{
	class IShader;
	class ParticleRenderObject : public SpatialObject
	{
	public:
		static const int MAX_SHARED_VERTICES;
		static ParticleRenderObject* GetRenderObject(const char* texturePath);
		static void ClearParticles();
		static void FinalizeRenderObjects();

		ParticleRenderObject();
		virtual ~ParticleRenderObject();

		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();

		void Clear();

		struct Vertex
		{
			Vec3 mPos;
			Vec4 mDirection_Intensity;
			Vec4 mPivot_Size;
			Vec4 mRot_Alpha_uv;
			Vec2 mUVStep;
		};

		Vertex* Map(UINT numVertices);
		void Unmap();


	private:
		typedef VectorMap< std::string, SmartPtr<ParticleRenderObject> > RENDER_OBJECTS;
		static RENDER_OBJECTS mRenderObjects;
		static SmartPtr<IMaterial> mMaterial;

		SmartPtr<IVertexBuffer> mVertexBuffer;
		UINT mNextMap;
		typedef std::vector< std::pair<UINT, UINT> > BATCHES;
		BATCHES mBatches;
		Vertex* mMapped;
	};
}