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
		void ParticleEmitter::ProcessDeleteOnStop(bool stopping);
		virtual void StopImmediate();
		virtual void SetVisibleParticle(bool visible);
		virtual bool IsAlive();
		virtual bool IsActive() const { return mInActiveList; }
		virtual void SetEmitterDirection(const fastbird::Vec3& dir){mEmitterDirection = dir;}
		virtual void SetEmitterColor(const Color& c){ mEmitterColor = c; }

		virtual void PreRender() {}
		virtual void Render() {}
		virtual void PostRender() {}

		void UpdateEmit(float dt);
		void CopyDataToRenderer(float dt);
		bool IsInfinite() const { return mLifeTime == -1.0f;  }

		struct ParticleTemplate
		{
			ParticleTemplate()
			: mEmitPerSec(2.0f), mInitialParticles(0), mMaxParticle(100)
			, mLifeMinMax(1.0f, 2.0f)
			, mAlign(ParticleAlign::Billboard)
			, mEmitTo(ParticleEmitTo::LocalSpace)
			, mStretchMax(0.f)
			, mRangeType(ParticleRangeType::Sphere), mRangeRadius(1.0f), mRangeRadiusMin(0.f)
			, mSizeMinMax(1.0f, 1.0f), mSizeRatioMinMax(1.0f, 1.0f), mPivot(0.5f, 0.5f)
			, mScaleVelMinMax(0.f, 0.f), mScaleVelRatio(1.f, 1.f)
			, mScaleAccel(0.1f, 0.1f), mScaleDeaccel(0.0f, .01f)
			, mVelocityMinMax(0.0f, 0.0f), mVelocityDirMin(-1.0f, -1.0f, -1.0f), mVelocityDirMax(1.0f, 1.0f, 1.0f)
			, mAccel(0.1f, 0.1f), mDeaccel(0.1f, 0.9f)
			, mRotMinMax(0, TWO_PI), mRotSpeedMinMax(-HALF_PI, HALF_PI)
			, mRotAccel(0.872f, 0.1f), mRotDeaccel(0, 0.1f)
			, mFadeInOut(0.1f, 0.9f)
			, mUVAnimColRow(1, 1), mUVAnimFramesPerSec(0), mUV_INV_FPS(1.f)
			, mDefaultDirection(0, 1, 0), mCross(false), mColor(1, 1, 1), mColorEnd(1, 1, 1)
			, mBlendMode(), mGlow(1.f), mPreMultiAlpha(false), mUVFlow(0, 0), mPosOffset(0, 0, 0)
			, mPosInterpolation(false), mDeleteWhenFull(false), mDeleteWhenStop(false)
			, mStartAfter(0), mUseRelativeVelocity(false), mAnimPendulum(false), mCameraPulling(0)

			// point light
			, mPLRangeMinMax(0, 0)

			{
			}

			bool IsLocalSpace() const { return mEmitTo == ParticleEmitTo::LocalSpace; }
			bool IsAlignBillboard() const { return mAlign == ParticleAlign::Billboard; }
			bool IsAlignDirection() const { return mAlign == ParticleAlign::Direction; }

			std::string mTexturePath;
			std::string mGeometryPath;
			SmartPtr<IMeshObject> mMeshObject;
			float mStartAfter;
			float mEmitPerSec;
			unsigned mInitialParticles;
			unsigned mMaxParticle;
			bool mDeleteWhenFull;
			bool mDeleteWhenStop;
			bool mPreMultiAlpha;
			ParticleBlendMode::Enum mBlendMode;
			ParticleAlign::Enum mAlign;			
			ParticleEmitTo::Enum mEmitTo;
			ParticleRangeType::Enum mRangeType;
			float mGlow;
			Vec2 mLifeMinMax;
			Vec3 mDefaultDirection;
			float mCameraPulling;
			float mRangeRadius;
			float mRangeRadiusMin;
			Vec3 mPosOffset;
			Vec2 mSizeMinMax;
			Vec2 mSizeRatioMinMax;
			Vec2 mPivot;
			Vec2 mScaleVelMinMax;
			Vec2 mScaleVelRatio;
			Vec2 mScaleAccel; // x: velocity / secs, y: until %
			Vec2 mScaleDeaccel; // x: velocity / secs, y: until %
			Vec2 mVelocityMinMax;
			Vec3 mVelocityDirMin;
			Vec3 mVelocityDirMax;
			Vec2 mAccel; // x: velocity / secs, y: until %
			Vec2 mDeaccel; // x: velocity / secs, y: until %
			Vec2 mRotMinMax;
			Vec2 mRotSpeedMinMax;
			Vec2 mRotAccel; // x: velocity / secs, y: until %
			Vec2 mRotDeaccel; // x: velocity / secs, y: until %
			Vec2 mFadeInOut; // percentage. 0.9( = 90% in xml)			
			Vec2 mIntensityMinMax;
			Vec2I mUVAnimColRow;
			Color mColor;
			Color mColorEnd;
			Vec2 mPLRangeMinMax;
			float mUVAnimFramesPerSec;
			float mUV_INV_FPS;
			float mStretchMax;
			bool mCross;
			bool mPosInterpolation;
			Vec2 mUVFlow;
			bool mUseRelativeVelocity;
			bool mAnimPendulum;
		};

		virtual Particle* Emit(unsigned templateIdx);
		Particle* Emit(const ParticleTemplate& pTemplate);
		virtual Particle& GetParticle(unsigned templateIdx, unsigned particleIdx);
		virtual void SetBufferSize(unsigned size);
		//virtual void Sort();
		virtual void SetLength(float length);

		virtual void SetRelativeVelocity(const Vec3& dir, float speed);

	private:
		ParticleEmitter* mAdam;
		typedef std::vector<ParticleTemplate> PARTICLE_TEMPLATES;
		PARTICLE_TEMPLATES mPTemplates; // currently support only one template per emitter.
		const PARTICLE_TEMPLATES* mClonedTemplates;

		// internal
		typedef VectorMap<const ParticleTemplate*, float> NEXT_EMITS;
		NEXT_EMITS mNextEmits;

		// pos interpolation
		typedef VectorMap<const ParticleTemplate*, Vec3> LAST_EMIT_POS;
		LAST_EMIT_POS mLastEmitPos;

		typedef CircularBuffer<Particle> PARTICLES;
		typedef VectorMap<const ParticleTemplate*, PARTICLES*> PARTICLESS;
		PARTICLESS mParticles;

		VectorMap<const ParticleTemplate*, unsigned> mAliveParticles;
		VectorMap<const ParticleTemplate*, unsigned> mMaxParticles;

		// not using currently
		// Assuming Y is the direction.
		fastbird::Vec3 mEmitterDirection;

		unsigned mEmitterID;
		float mLifeTime;
		float mCurLifeTime;		
		float mMaxSize;
		friend class ParticleManager;
		bool mInActiveList;
		bool mStop;
		bool mStopImmediate;
		bool mMoveToCam;
		bool mManualEmitter;
		Color mEmitterColor;
		float mLength;
		Vec3 mRelativeVelocityDir;
		float mRelativeVelocity;
		Vec3 mStartPos;
		float mFinalAlphaMod; // for glares in the opposite side of camera.
	};
}