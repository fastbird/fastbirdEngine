#pragma once
#include <Engine/Shaders/Constants.h>
#include <Engine/SpatialObject.h>
namespace fastbird
{
	class UI3DObj : public SpatialObject
	{
		const static float NotDefined;
		SmartPtr<IMaterial> mMaterial;
		SmartPtr<IVertexBuffer> mVertexBuffer;
		OBJECT_CONSTANTS mObjectConstants;
		Vec3 mPos;
		Vec2 mSizeInWorld;
		Vec3 mLastPos;
		Vec3 mLastCameraPos;
		Vec3 mLastCameraUp;

	public:
		UI3DObj();
		virtual ~UI3DObj();

		//-------------------------------------------------------------------------
		// IObject interfaces
		//-------------------------------------------------------------------------
		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();

		virtual void SetMaterial(const char* name, int pass = 0);
		virtual IMaterial* GetMaterial(int pass = 0) const { return mMaterial; }


		void SetPosSize(const Vec3& pos, const Vec2& sizeInWorld);
		void Reset3DUI();
		void SetTexture(ITexture* texture);

		
	};
}