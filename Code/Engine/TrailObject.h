#pragma once
#include <Engine/ITrailObject.h>

namespace fastbird{
	class TrailObject : public ITrailObject{
	public:
		struct TrailVertex{
			TrailVertex(const Vec4& pos, const Color& color)
				: mPos(pos)
			{
				mColor = color.Get4Byte();
			}
			Vec4 mPos;
			unsigned mColor;
		};
		TrailObject();

	private:
		std::vector<TrailVertex> mPoints;
		std::queue<std::pair<Vec3, Vec3>> mPairedPoints;

		bool mDirty;
		float mWidth;
		unsigned mMaxPoints;

		SmartPtr<IMaterial> mMaterial;
		SmartPtr<IVertexBuffer> mVB;
		

	public:

		//------------------------------------------------------------------------
		// IObject
		//------------------------------------------------------------------------
		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();
		virtual void SetMaterial(const char* name, int pass = 0);
		virtual void SetMaterial(IMaterial* pMat, int pass = 0);
		virtual IMaterial* GetMaterial(int pass = 0) const;


		//------------------------------------------------------------------------
		// ITrailObject
		//------------------------------------------------------------------------
		//for billboard trail - automatically face to the camera
		virtual void AddPoint(const Vec3& worldPos);
		virtual void SetWidth(float width);

		// for manual trail
		virtual void AddPoint(const Vec3& worldPosA, const Vec3& worldPosB);
		
		virtual void SetMaxPoints(unsigned num);
		virtual void Clear();

	protected:
		void RefreshVertexBuffer();
	};
}