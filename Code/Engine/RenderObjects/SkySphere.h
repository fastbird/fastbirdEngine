#pragma once
#include <Engine/ISkySphere.h>
namespace fastbird
{
	class IVertexBuffer;
	class IIndexBuffer;
	class SkySphere : public ISkySphere
	{
	public:		
		SkySphere();
		virtual ~SkySphere();

		static void CreateSharedEnvRT();
		static void DeleteSharedEnvRT();

		// interfaces
		virtual void DetachFromScene();
		virtual void SetMaterial(const char* name, int pass = 0);
		virtual void SetMaterial(IMaterial* pMat, int pass = 0);
		virtual IMaterial* GetMaterial(int pass =0) const;

		virtual void PreRender();
		virtual void Render();		
		virtual void PostRender();

		// ISkySphere
		virtual void UpdateEnvironmentMap(const Vec3& origin);
		virtual void SetInterpolationData(unsigned index, const Vec4& data);
		virtual void PrepareInterpolation(float time);
		virtual void SetUseAlphaBlend(bool use) {
			mUseAlphaBlend = use;
		}
		virtual void SetAlpha(float alpha);
		virtual float GetAlpha() const { return mAlpha; }

		// own
		void GenerateSphereMesh();

	private:
		void GenerateRadianceCoef(ITexture* pTex);
		static const int ENV_SIZE = 1024;

	private:
		SmartPtr<IMaterial> mMaterial;
		SmartPtr<IMaterial> mMaterialOCC;
		SmartPtr<IVertexBuffer> mVB;
		SmartPtr<IIndexBuffer> mIB;

		Vec4 mMaterialParamCur[4];
		Vec4 mMaterialParamDest[4];
		float mCurInterpolationTime;
		float mInterpolationTime;
		bool mInterpolating;
		SmartPtr<IBlendState> mAlphaBlend;
		bool mUseAlphaBlend;
		static fastbird::SmartPtr<fastbird::IRenderToTexture> mRT;
		float mAlpha;

		static const unsigned NumConsts = 7;
		Vec4 mIrradConsts[NumConsts];

		Vec4 mIrradCoeff[9];

	};
}