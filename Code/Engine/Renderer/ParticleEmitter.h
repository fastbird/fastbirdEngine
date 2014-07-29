#pragma once
#include <Engine/IParticleEmitter.h>
#include <Engine/Renderer/ParticleEnum.h>
#include <CommonLib/CircularBuffer.h>

namespace fastbird
{
	class ParticleEmitter : public IParticleEmitter
	{
	public:
		ParticleEmitter();
		virtual ~ParticleEmitter();

		virtual bool Load(const char* filepath);
		virtual bool Update(float elapsedTime);
		virtual unsigned GetEmitterID() const { return mEmitterID; }
		virtual IObject* Clone() const;
		virtual void Active(bool a);
		virtual void Stop();

		virtual void PreRender() {}
		virtual void Render() {}
		virtual void PostRender() {}

		void UpdateEmit(float elapsedTime);
		void CopyDataToRenderer();
		bool IsInfinite() const { return mLifeTime == -1.0f;  }

		struct ParticleTemplate
		{
			ParticleTemplate()
				: mEmitPerSec(2.0f), mLifeMinMax(1.0f, 2.0f)
				, mAlign(ParticleAlign::Billboard)
				, mEmitTo(ParticleEmitTo::LocalSpace)
				, mRangeType(ParticleRangeType::Sphere), mRangeRadius(1.0f)
				, mSizeMin(1.0f, 1.0f), mSizeMax(2.0f, 2.0f), mPivot(0.5f, 0.5f)
				, mScaleVelMin(0.f, 0.f), mScaleVelMax(0.f, 0.f)
				, mScaleAccel(0.1f, 0.1f), mScaleDeaccel(0.0f, .01f)
				, mVelocityMin(-1.0f, -1.0f, -1.0f), mVelocityMax(1.0f, 1.0f, 1.0f)
				, mAccel(0.1f, 0.1f), mDeaccel(0.1f, 0.9f)
				, mRotMinMax(0, TWO_PI), mRotSpeedMinMax(-HALF_PI, HALF_PI)
				, mRotAccel(0.872f, 0.1f), mRotDeaccel(0, 0.1f)
				, mFadeInOut(0.1f, 0.9f)
				, mUVAnimRowCol(1, 1), mUVAnimFramesPerSec(0), mUVAnimStep(1, 1)

			{
			}

			bool IsLocalSpace() const { return mEmitTo == ParticleEmitTo::LocalSpace; }
			bool IsBillboard() const { return mAlign == ParticleAlign::Billboard; }

			std::string mTexturePath;
			float mEmitPerSec;
			Vec2 mLifeMinMax;
			ParticleAlign::Enum mAlign;
			ParticleEmitTo::Enum mEmitTo;
			ParticleRangeType::Enum mRangeType;
			float mRangeRadius;
			Vec2 mSizeMin;
			Vec2 mSizeMax;
			Vec2 mPivot;
			Vec2 mScaleVelMin;
			Vec2 mScaleVelMax;
			Vec2 mScaleAccel; // x: velocity / secs, y: until %
			Vec2 mScaleDeaccel; // x: velocity / secs, y: until %
			Vec3 mVelocityMin;
			Vec3 mVelocityMax;
			Vec2 mAccel; // x: velocity / secs, y: until %
			Vec2 mDeaccel; // x: velocity / secs, y: until %
			Vec2 mRotMinMax;
			Vec2 mRotSpeedMinMax;
			Vec2 mRotAccel; // x: velocity / secs, y: until %
			Vec2 mRotDeaccel; // x: velocity / secs, y: until %
			Vec2 mFadeInOut; // percentage. 0.9( = 90% in xml)			
			Vec2 mIntensityMinMax;
			Vec2I mUVAnimRowCol;
			float mUVAnimFramesPerSec;
			float mUV_INV_FPS;
			Vec2 mUVAnimStep;
		};

		struct Particle : public ICircularData
		{
			Particle()
				: mLifeTime(0.0f)
				, mCurLifeTime(0.0f)
				, mAlpha(0.0f)
				, mIntensity(1.0f)
			{
			}
			bool IsAvailable() const
			{
				return mLifeTime==0.0f; // not infinite
			}
			bool IsAlive() const
			{
				return !IsAvailable();
			}
			bool IsInfinite() const
			{
				return mLifeTime==-1.0f;
			}

			Vec3 mPos;
			Vec3 mPosWorld;
			Vec3 mVelDir;
			float mVelocity;
			Vec3 mUDirection;			
			Vec2 mUVIndex;
			Vec2 mUVStep;
			Vec2 mSize;
			Vec2 mScaleSpeed;
			float mLifeTime;
			float mCurLifeTime;
			float mRot;
			float mRotSpeed;
			float mUVFrame;
			float mAlpha;
			float mIntensity;
		};

	private:
		typedef std::vector<ParticleTemplate> PARTICLE_TEMPLATES;
		PARTICLE_TEMPLATES mPTemplates; // currently support only one template per emitter.
		const PARTICLE_TEMPLATES* mClonedTemplates;

		// internal
		typedef VectorMap<const ParticleTemplate*, float> NEXT_EMITS;
		NEXT_EMITS mNextEmits;

		typedef CircularBuffer<Particle> PARTICLES;
		typedef VectorMap<const ParticleTemplate*, PARTICLES*> PARTICLESS;
		PARTICLESS mParticles;

		unsigned mEmitterID;
		float mLifeTime;
		float mCurLifeTime;		
		float mMaxSize;
		friend class ParticleManager;
		bool mInActiveList;
		bool mStop;
	};
}