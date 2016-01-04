/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(MeshObject);
	FB_DECLARE_SMART_PTR(PointLight);
	FB_DECLARE_SMART_PTR(ParticleEmitter);
	FB_DECLARE_SMART_PTR(ParticleRenderObject);
	struct Particle : public ICircularData
	{
		Particle()
			: mLifeTime(0.0f)
			, mCurLifeTime(0.0f)
			, mAlpha(0.0f)
			, mIntensity(1.0f)
			, mMeshObject(0)
			, mPointLight(0)
			, mPendulumBackward(false), mParticleEmitter(0)
		{
		}
		bool IsAvailable() const
		{
			return mLifeTime == 0.0f; // not infinite
		}
		bool IsAlive() const
		{
			return (!IsAvailable() && mCurLifeTime < mLifeTime) || (mLifeTime == -1); // only -1 is infinite alive. don't render -2.
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
		MeshObjectPtr mMeshObject;
		PointLightPtr mPointLight;
		ParticleEmitterPtr mParticleEmitter;
		bool mPendulumBackward;
	};

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
			, mVelocityToCenter(false), mParticleEmitter(-1), mDepthFade(true)
			, mNeedTeamColor(false)

			// point light
			, mPLRangeMinMax(0, 0)

		{
		}

		bool IsLocalSpace() const { return mEmitTo == ParticleEmitTo::LocalSpace; }
		bool IsAlignBillboard() const { return mAlign == ParticleAlign::Billboard; }
		bool IsAlignDirection() const { return mAlign == ParticleAlign::Direction; }

		std::string mTexturePath;
		std::string mGeometryPath;
		unsigned mParticleEmitter;
		MeshObjectPtr mMeshObject;
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
		bool mVelocityToCenter;
		bool mDepthFade;
		bool mNeedTeamColor;
	};
}