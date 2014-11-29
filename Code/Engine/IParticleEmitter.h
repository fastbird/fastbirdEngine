#pragma once
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/IMeshObject.h>
#include <CommonLib/CircularBuffer.h>
namespace fastbird
{
	class IMeshObject;
	class IParticleEmitter : public SpatialObject
	{
	public:
		struct Particle : public ICircularData
		{
			Particle()
			: mLifeTime(0.0f)
			, mCurLifeTime(0.0f)
			, mAlpha(0.0f)
			, mIntensity(1.0f)
			, mMeshObject(0)
			{
			}
			bool IsAvailable() const
			{
				return mLifeTime == 0.0f; // not infinite
			}
			bool IsAlive() const
			{
				return (!IsAvailable() && mCurLifeTime < mLifeTime) || (mLifeTime ==-1); // only -1 is infinite alive. don't render -2.
			}
			bool IsInfinite() const
			{
				return mLifeTime < 0.f;
			}

			Vec3 mPos;
			Vec3 mPosWorld;
			Vec3 mVelDir;
			float mVelocity;
			Vec3 mUDirection;
			Vec3 mVDirection;
			Vec2 mUVIndex;
			Vec2 mUVStep;
			Vec2 mSize;
			Vec2 mScaleSpeed;
			float mLifeTime;
			float mCurLifeTime;
			float mRot;
			float mRotSpeed;
			float mUVFrame;
			float mUV_SPF;
			float mAlpha;
			float mIntensity;
			DWORD mColor;
			SmartPtr<IMeshObject> mMeshObject;
		};

		virtual IObject* Clone() const{assert(0); return 0;}
		virtual unsigned GetEmitterID() const = 0;
		virtual bool Load(const char* filepath) = 0;
		virtual bool Update(float elapsedTime)= 0;
		virtual void Active(bool a) = 0;
		virtual void Stop() = 0;
		virtual void StopImmediate() = 0;
		virtual bool IsAlive() = 0;
		virtual void SetEmitterDirection(const fastbird::Vec3& dir) = 0;
		virtual void SetEmitterColor(const Color& c) = 0;

		// only for manual control particles - for clouds.
		virtual Particle* Emit(unsigned templateIdx) = 0;
		virtual Particle& GetParticle(unsigned templateIdx, unsigned index) = 0;
		virtual void SetBufferSize(unsigned size) = 0;
		virtual void SetLength(float length) = 0;
		//virtual void Sort() = 0;
		virtual void SetRelativeVelocity(const Vec3& dir, float speed) = 0;
		
	private:
		friend class ParticleManager;
		static IParticleEmitter* CreateParticleEmitter();

		friend class Engine;
	};
}