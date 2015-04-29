#pragma once
#include <Engine/CloudSimulation.h>
#include <Engine/CloudParticle.h>
#include <Engine/ParticleEmitter.h>

#define CLOUD_SAMPLE_LENGTH			0.5f 
#define CLOUD_OPTICAL_LENGTH_FACTOR	1.5f // determine the white and black contrast.more big more contrast.
#define CLOUD_SCATTER_FACTOR			0.6f // According to the scattering model used, to have the intensity of scattering color less than the incident color,
// the maximum of SCATTER_FACTOR should be 2/3.
#define CLOUD_MIN_DENSITY				0.05f

namespace fastbird
{
	class IParticleEmitter;

	class VolumetricCloud
	{
	private:
		float mWLength, mWWidth, mWHeight;
		float mCellSize;
		int mLength, mWidth, mHeight;
		Vec3 mWind;
		Vec3 mPos;
		Vec3 mCamPos;
		Vec3 mLightDir;
		Color mLightColor;
		float mLightIntensity;
		float mTimeA;
		float mTime;
		float mEvolvingSpeed;
		ParticlePool		m_ParticlePool;
		std::vector<SmartPtr<IParticleEmitter>> mParticleEmitters;
		int mColorUpdateInterval[2];
		float mViewDistance;

		CloudSimulation mSimulator;
		unsigned mCurrentBuffer;
		unsigned mParticleID;


	public:
		VolumetricCloud();
		~VolumetricCloud();
		bool Setup(const CloudProperties& prop);
		void Cleanup();
		void AdvanceTime(float time, int interval);
		void PrepareRender();

		bool GenerateCloudParticles();
		void UpdateCloudParticleColors();
		void SortCloudParticles(const Vec3& lookdir);
		void UpdateCloudPosition(float time);

		void SetEvolvingSpeed(float speed);
		void SetLightColor(const Color& color);
		void SetLightIntensity(float i);
		void SetWind(const Vec3& windDir, float v);

		float GetViewDistance() { return mViewDistance; }
		void UpdateViewDistance();

		const Vec3& GetPos() const { return mPos;}
		float GetCellSize() const { return mCellSize; }

		Color CalculateParticleIncidentColor(CloudParticle* p);
		Color CalculateParticleScatteringColor(CloudParticle* p);
	};
}