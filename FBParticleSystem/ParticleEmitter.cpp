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

#include "stdafx.h"
#include "ParticleEmitter.h"
#include "ParticleEnum.h"
#include "ParticleStruct.h"
#include "ParticleRenderObject.h"
#include "ParticleRenderKey.h"
#include "ParticleSystem.h"
#include "FBCommonHeaders/CowPtr.h"
#include "FBRenderer/PointLight.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Renderer.h"
#include "FBSceneManager/SceneManager.h"
#include "FBSceneManager/Scene.h"
#include "FBRenderer/Camera.h"
#include "FBSceneObjectFactory/MeshObject.h"
#include "FBSceneObjectFactory/SceneObjectFactory.h"

using namespace fb;
class ParticleEmitter::Impl
{
public:
	ParticleEmitter* mSelf;
	ParticleEmitterWeakPtr mSelfPtr;
	typedef std::vector<ParticleTemplate> PARTICLE_TEMPLATES;
	CowPtr<PARTICLE_TEMPLATES> mTemplates; // currently support only one template per emitter.

	// internal
	typedef VectorMap<const ParticleTemplate*, float> NEXT_EMITS;
	NEXT_EMITS mNextEmits;

	// pos interpolation
	typedef VectorMap<const ParticleTemplate*, Vec3> LAST_EMIT_POS;
	LAST_EMIT_POS mLastEmitPos;

	typedef CircularBuffer<Particle> PARTICLES;
	typedef std::shared_ptr<PARTICLES> PARTICLES_PTR;
	typedef VectorMap<const ParticleTemplate*, PARTICLES_PTR> ParticlesPerTemplate;
	ParticlesPerTemplate mParticlesPerTemplate;

	VectorMap<const ParticleTemplate*, unsigned> mAliveParticles;
	VectorMap<const ParticleTemplate*, unsigned> mMaxParticles;

	// not using currently
	// Assuming Y is the direction.
	fb::Vec3 mEmitterDirection;

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
	bool mVisible;

	VectorMap<std::string, std::string>  mShaderDefines;

	//---------------------------------------------------------------------------
	Impl(ParticleEmitter* self)
		: mSelf(self)
		, mLifeTime(2), mCurLifeTime(0)
		, mEmitterID(0), mInActiveList(false)		
		, mMaxSize(0.0f)
		, mStop(false)
		, mStopImmediate(false)
		, mEmitterDirection(0, 1, 0)		
		, mManualEmitter(false), mMoveToCam(false)
		, mEmitterColor(1, 1, 1)
		, mLength(0.0f)
		, mRelativeVelocity(0)
		, mRelativeVelocityDir(Vec3::ZERO)
		, mFinalAlphaMod(1.0f)
		, mVisible(true)
	{
	}

	Impl(ParticleEmitter* self, const Impl& other)
		:mSelf(self)
		, mTemplates(other.mTemplates)
		, mEmitterDirection(other.mEmitterDirection)
		, mEmitterID(other.mEmitterID)
		, mLifeTime(other.mLifeTime)
		, mCurLifeTime(0)
		, mMaxSize(other.mMaxSize)
		, mInActiveList(false)
		, mStop(false), mStopImmediate(false)
		, mMoveToCam(other.mMoveToCam), mManualEmitter(other.mManualEmitter)
		, mEmitterColor(other.mEmitterColor), mLength(other.mLength)
		, mRelativeVelocity(other.mRelativeVelocity), mRelativeVelocityDir(other.mRelativeVelocityDir)
		, mFinalAlphaMod(other.mFinalAlphaMod), mVisible(other.mVisible)
	{
		for (const auto& it : *(mTemplates.const_get())){
			PARTICLES_PTR particles(new PARTICLES, [](PARTICLES* obj){delete obj; });

			mMaxParticles.Insert({ &it, it.mMaxParticle });
			particles->Init(it.mMaxParticle * 2);
			mParticlesPerTemplate.Insert({ &it, particles });
			mNextEmits.Insert({ &it, (float)it.mInitialParticles });
			mAliveParticles.Insert({ &it, 0 });
		}
	}

	ParticleEmitterPtr Clone(){
		return ParticleEmitter::Create(*mSelf);
	}

	bool Load(const char* filepath, bool reload){
		tinyxml2::XMLDocument doc;
		int err = doc.LoadFile(filepath);
		if (err)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Error while parsing particle(%s)!", filepath).c_str());			
			if (doc.ErrorID() == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("particle %s is not found!", filepath).c_str());				
			}
			const char* errMsg = doc.GetErrorStr1();
			if (errMsg)
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("\t%s", errMsg).c_str());				
			errMsg = doc.GetErrorStr2();
			if (errMsg)
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("\t%s", errMsg).c_str());
			return false;
		}

		tinyxml2::XMLElement* pPE = doc.FirstChildElement("ParticleEmitter");
		if (!pPE)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Invaild particle file. %s", filepath).c_str());			
			return false;
		}

		const char* sz = pPE->Attribute("emitterLifeTime");
		if (sz)
			mLifeTime = StringConverter::ParseReal(sz);
		sz = pPE->Attribute("emitterID");
		if (sz)
			mEmitterID = StringConverter::ParseUnsignedInt(sz);
		else
		{
			assert(0);
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Emitter id omitted! (%s)", filepath).c_str());			
		}

		sz = pPE->Attribute("moveToCam");
		if (sz)
		{
			mMoveToCam = StringConverter::ParseBool(sz);
		}

		sz = pPE->Attribute("manualControl");
		if (sz)
			mManualEmitter = StringConverter::ParseBool(sz);

		PARTICLE_TEMPLATES* templates = 0;
		if (reload && mTemplates.const_get())
		{
			templates = const_cast<PARTICLE_TEMPLATES*>(mTemplates.const_get());
		}
		else{
			mTemplates = new PARTICLE_TEMPLATES;
			templates = mTemplates.get();
		}
		templates->clear();
		tinyxml2::XMLElement* pPT = pPE->FirstChildElement("ParticleTemplate");
		while (pPT)
		{
			templates->push_back(ParticleTemplate());
			ParticleTemplate& pt = templates->back();

			sz = pPT->Attribute("texture");
			if (sz)
			{
				pt.mTexturePath = sz;
				ToLowerCase(pt.mTexturePath);
			}



			sz = pPT->Attribute("pointLightRangeMinMax");
			if (sz)
				pt.mPLRangeMinMax = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("geometry");
			if (sz)
			{
				pt.mGeometryPath = sz;
				assert(pt.mTexturePath.empty());
				if (!pt.mGeometryPath.empty())
				{					
					pt.mMeshObject = SceneObjectFactory::GetInstance().CreateMeshObject(pt.mGeometryPath.c_str());
				}
			}

			sz = pPT->Attribute("particleEmitter");
			if (sz)
			{
				pt.mParticleEmitter = StringConverter::ParseUnsignedInt(sz);
				assert(pt.mTexturePath.empty());
			}

			sz = pPT->Attribute("startAfter");
			if (sz)
				pt.mStartAfter = StringConverter::ParseReal(sz);

			sz = pPT->Attribute("emitPerSec");
			if (sz)
				pt.mEmitPerSec = StringConverter::ParseReal(sz);

			sz = pPT->Attribute("numInitialParticle");
			if (sz)
				pt.mInitialParticles = StringConverter::ParseUnsignedInt(sz);

			sz = pPT->Attribute("maxParticle");
			if (sz)
				pt.mMaxParticle = StringConverter::ParseUnsignedInt(sz);

			sz = pPT->Attribute("deleteWhenFull");
			if (sz)
				pt.mDeleteWhenFull = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("cross");
			if (sz)
				pt.mCross = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("preMultiAlpha");
			if (sz)
				pt.mPreMultiAlpha = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("blendMode");
			if (sz)
				pt.mBlendMode = ParticleBlendMode::ConvertToEnum(sz);

			float glow = 0.f;
			sz = pPT->Attribute("glow");
			if (sz)
				glow = StringConverter::ParseReal(sz);

			sz = pPT->Attribute("posOffset");
			if (sz)
				pt.mPosOffset = StringMathConverter::ParseVec3(sz);

			sz = pPT->Attribute("lifeTimeMinMax");
			if (sz)
				pt.mLifeMinMax = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("deleteWhenStop");
			if (sz)
				pt.mDeleteWhenStop = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("align");
			if (sz)
				pt.mAlign = ParticleAlign::ConvertToEnum(sz);

			sz = pPT->Attribute("stretchMax");
			if (sz)
				pt.mStretchMax = StringConverter::ParseReal(sz);

			sz = pPT->Attribute("useRelativeVelocity");
			if (sz)
				pt.mUseRelativeVelocity = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("DefaultDirection");
			if (sz)
				pt.mDefaultDirection = StringMathConverter::ParseVec3(sz);

			sz = pPT->Attribute("emitTo");
			if (sz)
				pt.mEmitTo = ParticleEmitTo::ConvertToEnum(sz);

			sz = pPT->Attribute("cameraPulling");
			if (sz)
			{
				pt.mCameraPulling = StringConverter::ParseReal(sz);
			}

			sz = pPT->Attribute("range");
			if (sz)
				pt.mRangeType = ParticleRangeType::ConvertToEnum(sz);

			sz = pPT->Attribute("rangeRadius");
			if (sz)
				pt.mRangeRadius = StringConverter::ParseReal(sz);

			sz = pPT->Attribute("rangeRadiusMin");
			if (sz)
				pt.mRangeRadiusMin = StringConverter::ParseReal(sz);

			sz = pPT->Attribute("posInterpolation");
			if (sz)
				pt.mPosInterpolation = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("sizeMinMax");
			if (sz)
				pt.mSizeMinMax = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("sizeRatioMinMax");
			if (sz)
				pt.mSizeRatioMinMax = StringMathConverter::ParseVec2(sz);

			mMaxSize = std::max(mMaxSize,
				std::max(
				pt.mSizeMinMax.x * std::max(pt.mSizeRatioMinMax.x, pt.mSizeRatioMinMax.y),
				pt.mSizeMinMax.y));

			sz = pPT->Attribute("pivot");
			if (sz)
				pt.mPivot = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("scaleVelMinMax");
			if (sz)
				pt.mScaleVelMinMax = StringMathConverter::ParseVec2(sz);
			sz = pPT->Attribute("scaleVelRatio");
			if (sz)
				pt.mScaleVelRatio = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("scaleAccel");
			if (sz)
				pt.mScaleAccel.x = StringConverter::ParseReal(sz);
			sz = pPT->Attribute("scaleAccelUntil");
			if (sz)
				pt.mScaleAccel.y = StringConverter::ParseReal(sz) * 0.01f;

			sz = pPT->Attribute("scaleDeaccel");
			if (sz)
				pt.mScaleDeaccel.x = StringConverter::ParseReal(sz);
			sz = pPT->Attribute("scaleDeaccelAfter");
			if (sz)
				pt.mScaleDeaccel.y = StringConverter::ParseReal(sz) * 0.01f;

			sz = pPT->Attribute("velocityMinMax");
			if (sz)
				pt.mVelocityMinMax = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("velocityDirectionMin");
			if (sz)
				pt.mVelocityDirMin = StringMathConverter::ParseVec3(sz);
			sz = pPT->Attribute("velocityDirectionMax");
			if (sz)
				pt.mVelocityDirMax = StringMathConverter::ParseVec3(sz);

			sz = pPT->Attribute("velocityToCenter");
			if (sz)
				pt.mVelocityToCenter = StringConverter::ParseBool(sz);

			sz = pPT->Attribute("accel");
			if (sz)
				pt.mAccel.x = StringConverter::ParseReal(sz);
			sz = pPT->Attribute("accelUntil");
			if (sz)
				pt.mAccel.y = StringConverter::ParseReal(sz) * 0.01f;
			sz = pPT->Attribute("deaccel");
			if (sz)
				pt.mDeaccel.x = StringConverter::ParseReal(sz);
			sz = pPT->Attribute("deaccelAfter");
			if (sz)
				pt.mDeaccel.y = StringConverter::ParseReal(sz) * 0.01f;

			sz = pPT->Attribute("rotMinMax");
			if (sz)
				pt.mRotMinMax = Radian(StringMathConverter::ParseVec2(sz));
			sz = pPT->Attribute("rotSpeedMin");
			if (sz)
				pt.mRotSpeedMinMax.x = Radian(StringConverter::ParseReal(sz));
			sz = pPT->Attribute("rotSpeedMax");
			if (sz)
				pt.mRotSpeedMinMax.y = Radian(StringConverter::ParseReal(sz));
			sz = pPT->Attribute("rotAccel");
			if (sz)
				pt.mRotAccel.x = Radian(StringConverter::ParseReal(sz));
			sz = pPT->Attribute("rotAccelUntil");
			if (sz)
				pt.mRotAccel.y = StringConverter::ParseReal(sz) * 0.01f;
			sz = pPT->Attribute("rotDeaccel");
			if (sz)
				pt.mRotDeaccel.x = Radian(StringConverter::ParseReal(sz));
			sz = pPT->Attribute("rotDeaccelAfter");
			if (sz)
				pt.mRotDeaccel.y = StringConverter::ParseReal(sz) * 0.01f;

			sz = pPT->Attribute("fadeInUntil");
			if (sz)
				pt.mFadeInOut.x = StringConverter::ParseReal(sz) * 0.01f;
			sz = pPT->Attribute("fadeOutAfter");
			if (sz)
				pt.mFadeInOut.y = StringConverter::ParseReal(sz) * 0.01f;

			sz = pPT->Attribute("Intensity");
			if (sz)
				pt.mIntensityMinMax = StringMathConverter::ParseVec2(sz);

			sz = pPT->Attribute("color");
			if (sz)
				pt.mColor = StringMathConverter::ParseColor(sz);
			pt.mColorEnd = pt.mColor;

			sz = pPT->Attribute("colorEnd");
			if (sz)
				pt.mColorEnd = StringMathConverter::ParseColor(sz);

			sz = pPT->Attribute("uvAnimColRow");
			if (sz)
				pt.mUVAnimColRow = StringMathConverter::ParseVec2I(sz);

			sz = pPT->Attribute("uvAnimFramesPerSec");
			if (sz)
			{
				pt.mUVAnimFramesPerSec = StringConverter::ParseReal(sz);
				if (pt.mUVAnimFramesPerSec != 0.f)
				{
					pt.mUV_INV_FPS = 1.0f / pt.mUVAnimFramesPerSec;
				}
				else
				{
					if ((pt.mUVAnimColRow.x != 1 || pt.mUVAnimColRow.y != 1) && (pt.mLifeMinMax.x == -1 || pt.mLifeMinMax.y == -1))
					{
						pt.mUV_INV_FPS = 1.0f / 4.0f;
					}
				}
			}

			sz = pPT->Attribute("pendulum");
			if (sz)
			{
				pt.mAnimPendulum = StringConverter::ParseBool(sz);
			}

			sz = pPT->Attribute("uvFlow");
			if (sz)
			{
				pt.mUVFlow = StringMathConverter::ParseVec2(sz);
			}

			sz = pPT->Attribute("depthFade");
			if (sz)
			{
				pt.mDepthFade = StringConverter::ParseBool(sz);
			}

			if (!pt.mTexturePath.empty())
			{
				BLEND_DESC desc;
				switch (pt.mBlendMode)
				{
				case ParticleBlendMode::Additive:
				{
					desc.RenderTarget[0].BlendEnable = true;
					desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
					desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
					desc.RenderTarget[0].DestBlend = BLEND_ONE;
				}
				break;
				case ParticleBlendMode::AlphaBlend:
				{
					// desc.AlphaToCoverageEnable = true;
					desc.RenderTarget[0].BlendEnable = true;
					desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
					desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
					desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;

				}
				break;
				case ParticleBlendMode::InvColorBlend:
				{
					desc.RenderTarget[0].BlendEnable = true;
					desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
					desc.RenderTarget[0].SrcBlend = BLEND_INV_DEST_COLOR;
					desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
				}
				break;
				case ParticleBlendMode::Replace:
				{
					// desc.AlphaToCoverageEnable = true;
					desc.RenderTarget[0].BlendEnable = true;
					desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
					desc.RenderTarget[0].SrcBlend = BLEND_ONE;
					desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
				}
				break;
				default:
					assert(0);
				}
				bool created = false;
				ParticleRenderKey key(pt.mTexturePath.c_str(), desc, glow>0.f, pt.mDepthFade);
				auto pro = ParticleRenderObject::GetRenderObject(key, created);
				pt.mParticleRenderObject = pro;
				assert(pro);
				auto material = pro->GetMaterial();
				assert(material);
				bool defineChanged = false;
				if (created)
				{
					switch (pt.mBlendMode)
					{
					case ParticleBlendMode::InvColorBlend:
					{
						defineChanged = material->AddShaderDefine("_INV_COLOR_BLEND", "1") || defineChanged;
						break;
					}
					case ParticleBlendMode::Replace:
					{
						defineChanged = material->AddShaderDefine("_PRE_MULTIPLIED_ALPHA", "1") || defineChanged;
						break;
					}
					}

					if (pt.mPreMultiAlpha)
					{
						defineChanged = material->AddShaderDefine("_PRE_MULTIPLIED_ALPHA", "1") || defineChanged;
					}

					if (glow == 0.f)
					{
						defineChanged = material->AddShaderDefine("_NO_GLOW", "1") || defineChanged;
						material->SetGlow(false);
					}
					else
					{
						defineChanged = material->RemoveShaderDefine("_NO_GLOW") || defineChanged;
						material->SetGlow(true);
					}

					if (!pt.mDepthFade)
					{
						defineChanged = material->AddShaderDefine("_NO_DEPTH_FADE", "1") || defineChanged;
					}
					/*if (defineChanged)
						pro->GetMaterial()->ApplyShaderDefines();*/
					material->SetMaterialParameter(0, Vec4(glow, 0, 0, 0));
				}
			}

			pPT = pPT->NextSiblingElement();
		}

		/*
		Prototype doens't need to have this.
		FB_FOREACH(it, mPTemplates)
		{
		mParticlesPerTemplate.Insert(ParticlesPerTemplate::value_type(&(*it), FB_NEW(PARTICLES)));
		mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 0.f));
		}
		*/

		return true;
	}

	bool UpdateEmitter(float elapsedTime){
		mCurLifeTime += elapsedTime;
		if ((!IsInfinite() && mCurLifeTime > mLifeTime) || mStop)
		{
			mStop = true;
			int numAlive = 0;
			if (!mStopImmediate)
			{
				for(auto& it: mParticlesPerTemplate)
				{
					const ParticleTemplate* pt = it.first;
					auto particles = it.second;
					PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
					for (; itParticle != itEndParticle; ++itParticle)
					{
						if (itParticle->IsInfinite())
						{
							itParticle->mLifeTime = 0.0f;
						}
						if (itParticle->IsAlive())
							++numAlive;
					}
				}
			}
			else
			{
				for(auto& it: mParticlesPerTemplate)
				{
					const ParticleTemplate* pt = it.first;
					auto particles = it.second;
					PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
					for (; itParticle != itEndParticle; ++itParticle)
					{
						itParticle->mCurLifeTime = itParticle->mLifeTime;
					}
				}
			}
			if (numAlive == 0)
			{
				ProcessDeleteOnStop(true, true);
				mInActiveList = false;
				return false;
			}
		}

		auto& renderer = Renderer::GetInstance();
		//mBoundingVolumeWorld->Invalidate();
		// update existing partices
		for(auto& it: mParticlesPerTemplate)
		{
			const ParticleTemplate* pt = it.first;
			auto particles = it.second;
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle != itEndParticle; ++itParticle)
			{
				Particle& p = *itParticle;
				if (p.IsAlive() && p.mLifeTime != -2.f)
				{
					if (!p.IsInfinite())
					{
						p.mCurLifeTime += elapsedTime;
						if (p.mCurLifeTime >= p.mLifeTime)
						{
							if (p.mMeshObject){
								p.mMeshObject->DetachFromScene();
							}
							if (p.mPointLight)
							{
								// delete point light by assinging null;
								p.mPointLight = 0;
							}
							if (p.mParticleEmitter)
							{
								p.mParticleEmitter->Stop();
								p.mParticleEmitter = 0;
							}

							p.mLifeTime = 0.0f; // mark dead.
							p.mCurLifeTime = 0.0f;
							continue;
						}
					}
					// change velocity
					float normTime = 0;
					if (!p.IsInfinite())
					{
						normTime = p.mCurLifeTime / p.mLifeTime;
						if (normTime < pt->mAccel.y)
						{
							p.mVelocity += pt->mAccel.x * elapsedTime;
						}
						if (normTime > pt->mDeaccel.y)
						{
							p.mVelocity -= pt->mDeaccel.x * elapsedTime;
						}
					}
					if (pt->mVelocityToCenter)
					{
						if (pt->mEmitTo == ParticleEmitTo::WorldSpace)
							p.mVelDir = mSelf->GetPosition() - p.mPos;
						else
							p.mVelDir = -p.mPos;
					}

					// change pos
					p.mPos += p.mVelDir * (p.mVelocity * elapsedTime);

					if (pt->IsLocalSpace())
					{
						p.mPosWorld = mSelf->GetLocation().ApplyForward(p.mPos);
					}
					else
					{
						p.mPosWorld = p.mPos;
					}
					if (pt->mCameraPulling != 0)
					{
						p.mPosWorld -= renderer.GetMainCamera()->GetDirection() * pt->mCameraPulling;
					}

					//change rot speed
					if (!p.IsInfinite())
					{
						float sign = Sign(p.mRotSpeed);
						if (normTime < pt->mRotAccel.y)
						{
							p.mRotSpeed += sign * pt->mRotAccel.x * elapsedTime;
						}
						if (normTime > pt->mRotDeaccel.y)
						{
							p.mRotSpeed -= sign * pt->mRotDeaccel.x * elapsedTime;
						}
					}
					// change rot
					p.mRot += p.mRotSpeed * elapsedTime;


					float scale = mSelf->GetScale().x;
					// change scale speed
					if (!p.IsInfinite())
					{
						if (normTime < pt->mScaleAccel.y)
						{
							p.mScaleSpeed += pt->mScaleAccel.x * scale * elapsedTime;
						}
						if (normTime > pt->mScaleDeaccel.y)
						{
							p.mScaleSpeed -= pt->mScaleDeaccel.x * scale * elapsedTime;
						}
					}
					// change scale
					p.mSize += p.mScaleSpeed * elapsedTime;
					if (p.mSize.x < 0.0f)
						p.mSize.x = 0.0f;
					if (p.mSize.y < 0.0f)
						p.mSize.y = 0.0f;

					Vec3 toSidePos = mSelf->GetDirection() * p.mSize.x;
					/*mBoundingVolumeWorld->Merge(p.mPosWorld - pt->mPivot.x * toSidePos);
					mBoundingVolumeWorld->Merge(p.mPosWorld + (1.0f - pt->mPivot.x) * toSidePos);*/

					// update alpha
					if (!p.IsInfinite())
					{
						if (normTime < pt->mFadeInOut.x)
							p.mAlpha = SmoothStep(0, pt->mFadeInOut.x, normTime) * mFinalAlphaMod;
						else if (normTime > pt->mFadeInOut.y)
							p.mAlpha = (1.0f - SmoothStep(pt->mFadeInOut.y, 1.0, normTime)) * mFinalAlphaMod;
						else
							p.mAlpha = 1.0f * mFinalAlphaMod;

						// update color
						if (pt->mColor != pt->mColorEnd)
						{
							p.mColor = Lerp(pt->mColor, pt->mColorEnd, normTime) * mEmitterColor;
						}
					}
					else
					{
						p.mAlpha = 1.0f * mFinalAlphaMod;
					}


					// uv frame
					if ((pt->mUVAnimColRow.x > 1 || pt->mUVAnimColRow.y > 1))
					{
						p.mUVFrame += elapsedTime;
						while (p.mUVFrame > p.mUV_SPF)
						{
							p.mUVFrame -= p.mUV_SPF;
							if (p.mPendulumBackward)
							{
								p.mUVIndex.x -= 1;
								if (p.mUVIndex.x < 0)
								{
									p.mUVIndex.x = pt->mUVAnimColRow.x - 1.f;
									p.mUVIndex.y -= 1;
									if (p.mUVIndex.y < 0)
									{
										p.mUVIndex.y = 0;
										p.mPendulumBackward = false;
									}
								}
							}
							else
							{
								p.mUVIndex.x += 1;
								if (p.mUVIndex.x >= pt->mUVAnimColRow.x)
								{
									p.mUVIndex.x = 0;
									p.mUVIndex.y += 1;
									if (p.mUVIndex.y >= pt->mUVAnimColRow.y)
									{
										if (pt->mAnimPendulum)
										{
											p.mPendulumBackward = true;
											p.mUVIndex.y = pt->mUVAnimColRow.y - 1.f;
											p.mUVIndex.x = pt->mUVAnimColRow.x - 1.f;
										}
										else
										{
											p.mUVIndex.y = 0;
										}

									}
								}
							}
						}
					}

					if (p.mPointLight)
					{
						p.mPointLight->SetAlpha(p.mAlpha * mFinalAlphaMod);
					}
				}
			}
		}

		if (mMoveToCam)
		{
			auto cam = renderer.GetMainCamera();
			if (cam)
			{
				auto& camPos = cam->GetPosition();
				float emitterNormTime = std::min((mCurLifeTime / mLifeTime) * 2.0f, 0.85f);
				auto toStartPos = mStartPos - camPos;
				float dist = toStartPos.Length();
				float d = cam->GetDirection().Dot(toStartPos.NormalizeCopy());
				if (d < 0.2f)
				{
					auto oppositeStart = cam->GetDirection() * dist;
					float angle = oppositeStart.AngleBetween(toStartPos);
					float rotAngle = cam->GetFOV()*.5f;
					auto axis = oppositeStart.Cross(toStartPos).NormalizeCopy();
					auto desiredLocation = camPos + (Quat(rotAngle, axis) * oppositeStart);
					auto dest = Lerp(desiredLocation, camPos, emitterNormTime);
					mSelf->SetPosition(dest);
					mFinalAlphaMod += emitterNormTime *.5f;
					mFinalAlphaMod = std::min(1.0f, mFinalAlphaMod);
				}
				else
				{
					auto dest = Lerp(mStartPos, camPos, emitterNormTime);
					mSelf->SetPosition(dest);					
					mFinalAlphaMod = 1.0f;
				}
			}

		}
		/*assert(mBoundingVolumeWorld->GetBVType() == BoundingVolume::BV_AABB);
		BVaabb* pAABB = (BVaabb*)mBoundingVolumeWorld.get();
		pAABB->Expand(mMaxSize);*/

		if (!mStop)
			UpdateEmit(elapsedTime);

		CopyDataToRenderer(elapsedTime);

		return true;
	}

	unsigned GetEmitterID() const{
		return mEmitterID;
	}

	void Active(bool a, bool pending = false){
		if (a && (!mInActiveList || mStop))
		{
			if (!mInActiveList)
			{
				if (!pending)
					ParticleSystem::GetInstance().AddActiveParticle(mSelfPtr.lock());
				else
					ParticleSystem::GetInstance().AddActiveParticlePending(mSelfPtr.lock());
			}
			mInActiveList = true;
			mStop = false;
			mStopImmediate = false;
			mCurLifeTime = 0.f;
			if (mTemplates)
			{
				for (const auto& it : *(mTemplates.const_get()))
				{
					mNextEmits.Insert(NEXT_EMITS::value_type(&it, 1.f));
					mNextEmits[&it] = (float)it.mInitialParticles;
				}
			}
			mStartPos = mSelf->GetPosition();
			ProcessDeleteOnStop(false);
		}
		else if (!a && mInActiveList)
		{
			ProcessDeleteOnStop(true, true);

			ParticleSystem::GetInstance().RemoveDeactiveParticle(mSelfPtr.lock());
			mInActiveList = false;
		}
	}

	void Stop(){
		mStop = true;
		ProcessDeleteOnStop(true);
	}

	void ProcessDeleteOnStop(bool stopping, bool force = false){
		for (const auto& pt : *(mTemplates.const_get()))
		{
			if (pt.mDeleteWhenStop || force)
			{
				auto it = mParticlesPerTemplate.Find(&pt);
				if (it == mParticlesPerTemplate.end())
					continue;
				PARTICLES& particles = *(it->second);
				if (stopping)
				{
					for (auto& p : particles)
					{
						p.mCurLifeTime = p.mLifeTime;
						if (p.mMeshObject){
							p.mMeshObject->DetachFromScene();
						}
						if (p.mPointLight)
							p.mPointLight->SetEnabled(false);
						if (p.mParticleEmitter)
							p.mParticleEmitter->Stop();
					}
				}
				else
				{
					for (auto& p : particles)
					{
						if (p.mMeshObject){
							p.mMeshObject->DetachFromScene();
						}
						if (p.mPointLight)
							p.mPointLight->SetEnabled(true);
						if (p.mParticleEmitter)
							p.mParticleEmitter->Active(true, true);
					}
				}
			}
		}
	}

	void StopImmediate(){
		mStop = true;
		mStopImmediate = true;
	}

	void SetVisible(bool visible){
		mVisible = visible;		
		for (auto& pt : *(mTemplates.const_get()))
		{
			PARTICLES& particles = *(mParticlesPerTemplate[&pt]);
			for (auto& p : particles)
			{
				if (p.mMeshObject){
					p.mMeshObject->SetVisible(visible);
					p.mMeshObject->DetachFromScene();
				}
				if (p.mPointLight)
					p.mPointLight->SetEnabled(visible);
				if (p.mParticleEmitter)
					p.mParticleEmitter->SetVisible(visible);
			}
		}
	}

	bool IsAlive(){
		return mInActiveList;
	}

	bool IsActive() const{
		return mInActiveList;
	}

	void SetEmitterDirection(const fb::Vec3& dir){
		mEmitterDirection = dir;
	}

	void SetEmitterColor(const Color& c){
		mEmitterColor = c;
	}

	void SetAlpha(float alpha){
		mFinalAlphaMod = alpha;
		for(auto& it : mParticlesPerTemplate)
		{
			auto& particles = it.second;
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle != itEndParticle; ++itParticle)
			{
				if (itParticle->mMeshObject)
				{
					auto mat = itParticle->mMeshObject->GetMaterial();
					if (!mat->IsTransparent()){
						itParticle->mMeshObject->SetForceAlphaBlending(true, alpha);
					}
					else{
						auto diffuse = mat->GetDiffuseColor();
						diffuse.w = alpha;
						mat->SetDiffuseColor(diffuse);
					}
				}
			}
		}
	}

	float GetAlpha() const{
		return mFinalAlphaMod;
	}

	void UpdateEmit(float dt){
		// emit
		int i = 0;
		for (const auto& pt : *(mTemplates.const_get()))
		{
			if (pt.mStartAfter > mCurLifeTime)
				continue;

			unsigned alives = mAliveParticles[&pt];
			unsigned& maxParticles = mMaxParticles[&pt];
			if (alives >= maxParticles && !pt.mDeleteWhenFull)
			{
				Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("ParticleEmitter(%u) doubled its buffer.", mEmitterID).c_str());				
				mParticlesPerTemplate[&pt]->DoubleSize();
				maxParticles *= 2;
			}

			float& nextEmit = mNextEmits[&pt];
			nextEmit += dt * pt.mEmitPerSec;
			float integral;
			nextEmit = modf(nextEmit, &integral);
			int num = (int)integral;
			auto itFind = mLastEmitPos.Find(&pt);
			Particle* p = 0;
			for (int i = 0; i<num; i++)
			{
				p = Emit(pt);
				if (p && itFind != mLastEmitPos.end())
				{
					Vec3 toNew = p->mPos - itFind->second;
					float length = toNew.Normalize();
					p->mPos = itFind->second + toNew * length*((i + 1) / (float)num);
				}
			}

			if (pt.mPosInterpolation && p)
			{
				mLastEmitPos[&pt] = p->mPos;
			}
		}
	}

	void CopyDataToRenderer(float dt){
		if (!mVisible)
			return;
		CameraPtr pCamera = Renderer::GetInstance().GetMainCamera();
		assert(pCamera);
		//if (pCamera->IsCulled(mBoundingVolumeWorld))
		//return;

		for(auto& it : mParticlesPerTemplate)
		{
			auto particles = (it.second);
			const ParticleTemplate* pt = it.first;

			ParticleRenderObjectPtr pro = pt->mParticleRenderObject;

			if (pro && pt->mAlign)
			{
				pro->SetDoubleSided(true);
			}
			unsigned& aliveParticle = mAliveParticles[pt];
			aliveParticle = 0;
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle != itEndParticle; ++itParticle)
			{
				if (itParticle->IsAlive())
					aliveParticle++;
			}

			if (aliveParticle > 0)
			{
				ParticleRenderObject::Vertex* dest = 0;
				unsigned numVertices = pt->mCross ? aliveParticle * 2 : aliveParticle;
				unsigned numWritable = numVertices;
				if (pro)
				{
					dest = pro->Map(numVertices, numWritable);
					if (numVertices != numWritable)
					{
						Logger::Log(FB_ERROR_LOG_ARG, FormatString("Emitter(%u) tried lock %u but only %u locked.", mEmitterID, numVertices, numWritable).c_str());						
					}

				}

				PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
				for (; itParticle != itEndParticle; ++itParticle)
				{
					Particle& p = *itParticle;
					if (p.IsAlive())
					{
						int iteration = pt->mCross ? 2 : 1;
						Vec3 vdirBackup;
						Vec3 udirBackup;
						for (int i = 0; i < iteration; i++)
						{
							Vec3 udir = p.mUDirection;
							Vec3 vdir = p.mVDirection;
							if (pt->IsLocalSpace())
							{
								if (pt->IsAlignDirection())
								{
									Mat33 toViewRot = pCamera->GetMatrix(Camera::View).To33();
									if (i == 0)
									{
										Vec3 worldForward = (mSelf->GetRotation() * udir);

										udir = toViewRot * worldForward;
										udirBackup = udir;
										vdir = toViewRot  * pCamera->GetDirection().Cross(worldForward).NormalizeCopy();
										vdirBackup = vdir;
									}
									else
									{
										// crossed additional plane
										udir = udirBackup;
										vdir = vdirBackup.Cross(udirBackup).NormalizeCopy();
									}
								}
							}
							else
							{
								Vec3 worldForward;
								if (pt->mUseRelativeVelocity && !IsEqual(mRelativeVelocity, 0.0f, 0.001f)) // camera relative
								{
									worldForward = mRelativeVelocityDir;
									Mat33 toViewRot = pCamera->GetMatrix(Camera::View).To33();
									udir = toViewRot * worldForward;
									vdir = toViewRot  * pCamera->GetDirection().Cross(worldForward).NormalizeCopy();
								}
								else
								{
									if (pt->IsAlignDirection() && p.mVelocity != 0)
									{
										Mat33 toViewRot = pCamera->GetMatrix(Camera::View).To33();
										if (i == 0)
										{
											Vec3 worldForward = p.mVelDir;

											udir = toViewRot * worldForward;
											udirBackup = udir;
											vdir = toViewRot  * pCamera->GetDirection().Cross(worldForward).NormalizeCopy();
											vdirBackup = vdir;
										}
										else
										{
											// crossed additional plane
											udir = udirBackup;
											vdir = vdirBackup.Cross(udirBackup).NormalizeCopy();
										}
									}
								}
							}
							Vec2 size = p.mSize;
							if (pt->mStretchMax > 0.f)
							{
								auto pos = mSelf->GetPosition();
								auto prevPos = mSelf->GetPreviousPosition();
								if (!IsEqual(mRelativeVelocity, 0.f, 0.001f))
								{
									size.x += std::min(size.x * pt->mStretchMax, std::max(0.f, mRelativeVelocity));
								}
								else if (!IsEqual(pos, prevPos, 0.001f))
								{
									auto cam = Renderer::GetInstance().GetCamera();
									auto distToCam = cam->GetPosition().DistanceTo(mSelf->GetPosition());
									size.x += std::min(size.x*pt->mStretchMax, std::max(0.f, (pos - prevPos).Length() / dt*0.1f - distToCam*.1f));
								}
							}
							if (dest && numWritable)
							{
								dest->mPos = p.mPosWorld;
								dest->mUDirection_Intensity = Vec4(udir.x, udir.y, udir.z, p.mIntensity);
								dest->mVDirection = vdir;
								dest->mPivot_Size = Vec4(pt->mPivot.x, pt->mPivot.y, size.x, size.y);
								dest->mRot_Alpha_uv = Vec4(p.mRot, p.mAlpha,
									p.mUVIndex.x - pt->mUVFlow.x * p.mCurLifeTime, p.mUVIndex.y - pt->mUVFlow.y * p.mCurLifeTime);
								dest->mUVStep = p.mUVStep;
								dest->mColor = p.mColor;
								dest++;
								numWritable--;

							}

							// geometry or point light
						{
							if (p.mMeshObject)
							{
								p.mMeshObject->SetPosition(p.mPosWorld);
								p.mMeshObject->SetRotation(mSelf->GetRotation());
							}
							if (p.mPointLight)
							{
								p.mPointLight->SetPosition(p.mPosWorld);
							}
							if (p.mParticleEmitter)
							{
								p.mParticleEmitter->SetPosition(p.mPosWorld);
							}

						}

						}
					}
				}
			}
		}
	}

	bool IsInfinite() const{
		return mLifeTime == -1.0f;
	}

	void SetBufferSize(unsigned size){
		for( const auto& it : *(mTemplates.const_get()))
		{
			mMaxParticles[&it] = size;
			mParticlesPerTemplate[&it]->Init(size);
		}
	}

	void SetLength(float length){
		if (mLength == length)
			return;
		mLength = length;
		for(auto& it : mParticlesPerTemplate)
		{
			auto particles = it.second;
			const ParticleTemplate* pt = it.first;
			if (pt->mAlign == ParticleAlign::Direction)
			{
				float size = Random(pt->mSizeMinMax.x, pt->mSizeMinMax.y);
				float ratio = Random(pt->mSizeRatioMinMax.x, pt->mSizeRatioMinMax.y);
				PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
				for (; itParticle != itEndParticle; ++itParticle)
				{
					itParticle->mSize = Vec2(size * ratio, size);
					if (mLength != 0)
					{
						itParticle->mSize.x = itParticle->mSize.x * (mLength / size);
					}
				}
			}
		}
	}

	void SetRelativeVelocity(const Vec3& dir, float speed){
		if (speed > 0)
			mRelativeVelocityDir = dir;

		mRelativeVelocity = speed;
	}

	void RemoveShaderDefine(const char* def){
		auto it = mShaderDefines.Find(def);
		if (it != mShaderDefines.end())
			mShaderDefines.erase(it);

		for (const auto& pt : *(mTemplates.const_get()))
		{
			PARTICLES& particles = *(mParticlesPerTemplate[&pt]);
			for (auto& p : particles)
			{
				if (p.mMeshObject){					
					auto mat = p.mMeshObject->GetMaterial();
					if (mat){
						mat->RemoveShaderDefine(def);
					}
				}
			}
		}
	}

	void AddShaderDefine(const char* def, const char* val){
		auto it = mShaderDefines.Find(std::string(def));
		if (it != mShaderDefines.end())
		{
			it->second = val;
		}
		else{
			mShaderDefines[def] = val;
		}

		for (const auto& pt : *(mTemplates.const_get()))
		{
			PARTICLES& particles = *(mParticlesPerTemplate[&pt]);
			for (auto& p : particles)
			{
				if (p.mMeshObject){					
					auto mat = p.mMeshObject->GetMaterial();
					if (mat){
						mat->AddShaderDefine(def, val);
					}
				}
			}
		}
	}

	/*void ApplyShaderDefine(){
		for (auto& pt : *(mTemplates.const_get()))
		{
			PARTICLES& particles = *(mParticlesPerTemplate[&pt]);
			for (auto& p : particles)
			{
				if (p.mMeshObject){					
					auto mat = p.mMeshObject->GetMaterial();
					if (mat){
						mat->ApplyShaderDefines();
					}
				}
			}
		}
	}*/


	Particle* Emit(unsigned templateIdx){
		assert(templateIdx < mTemplates.const_get()->size());
		const ParticleTemplate& pt = (*(mTemplates.const_get()))[templateIdx];
		return Emit(pt);
	}

	Particle* Emit(const ParticleTemplate& pt){
		PARTICLES& particles = *(mParticlesPerTemplate[&pt]);
		size_t addedPos = particles.push_back(Particle());
		Particle& p = particles.GetAt(addedPos);
		const auto& vScale = mSelf->GetScale();
		float scale = vScale.x;
		switch (pt.mRangeType)
		{
		case ParticleRangeType::Point:
		{
			if (pt.IsLocalSpace())
			{
				p.mPos = Vec3(0.0f);
			}
			else
			{
				p.mPos = mSelf->GetPosition();
			}
		}
		break;
		case ParticleRangeType::Box:
		{
			p.mPos = Random(Vec3(-pt.mRangeRadius), Vec3(pt.mRangeRadius))*scale;
			if (!pt.IsLocalSpace())
			{
				p.mPos += mSelf->GetPosition();
			}
		}
		break;
		case ParticleRangeType::Sphere:
		{
			float r = Random(pt.mRangeRadiusMin, pt.mRangeRadius)*scale;
			float theta = Random(0.0f, PI);
			float phi = Random(0.0f, TWO_PI);
			p.mPos = SphericalToCartesian(r, theta, phi);
			if (!pt.IsLocalSpace())
			{
				p.mPos += mSelf->GetPosition();
			}
		}
		break;
		case ParticleRangeType::Hemisphere:
		{
			float r = Random(0.0f, pt.mRangeRadius)*scale;
			float theta = Random(0.0f, HALF_PI);
			float phi = Random(0.0f, TWO_PI);
			p.mPos = SphericalToCartesian(theta, phi) * r;
			if (!pt.IsLocalSpace())
			{
				p.mPos += mSelf->GetPosition();
			}
		}
		break;
		case ParticleRangeType::Cone:
		{
			float tanS = Random(0.0f, PI);
			float cosT = Random(0.0f, TWO_PI);
			float sinT = Random(0.0f, TWO_PI);
			float height = Random(0.0f, pt.mRangeRadius)*scale;
			p.mPos = Vec3(height*tanS*cosT, height*tanS*sinT, height);
			if (!pt.IsLocalSpace())
			{
				p.mPos += mSelf->GetPosition();
			}
		}
		break;
		}
		float angle = pt.mDefaultDirection.AngleBetween(mSelf->GetDirection());
		Vec3 posOffset = pt.mPosOffset;
		if (pt.mPosOffset != Vec3::ZERO && pt.mEmitTo == ParticleEmitTo::WorldSpace)
		{
			if (angle > 0.01f)
			{
				Vec3 axis = pt.mDefaultDirection.Cross(mSelf->GetDirection());
				axis.Normalize();
				Quat matchRot(angle, axis);
				posOffset = matchRot * posOffset;
			}
		}
		p.mPos += posOffset;

		Vec3 velDir;
		if (pt.mVelocityToCenter)
		{
			if (pt.mEmitTo == ParticleEmitTo::WorldSpace)
				velDir = mSelf->GetPosition() - p.mPos;
			else
				velDir = -p.mPos;
		}
		else
		{
			velDir = Random(pt.mVelocityDirMin, pt.mVelocityDirMax).NormalizeCopy();
		}

		if (angle > 0.01f && pt.mEmitTo == ParticleEmitTo::WorldSpace)
		{
			Vec3 axis = pt.mDefaultDirection.Cross(mSelf->GetDirection());
			axis.Normalize();
			Quat matchRot(angle, axis);
			velDir = matchRot * velDir;
		}

		p.mVelDir = velDir;
		p.mVelocity = Random(pt.mVelocityMinMax.x, pt.mVelocityMinMax.y)*scale;
		if (pt.mAlign == ParticleAlign::Billboard)
		{
			p.mUDirection = Vec3::UNIT_X;
			p.mVDirection = -Vec3::UNIT_Z;
		}
		else
		{
			p.mUDirection = Vec3::UNIT_Y;
			p.mVDirection = -Vec3::UNIT_Z;
		}

		p.mUVIndex = Vec2(0, 0);
		p.mUVStep = Vec2(1.0f / pt.mUVAnimColRow.x, 1.0f / pt.mUVAnimColRow.y);
		p.mUVFrame = 0.f;
		p.mLifeTime = Random(pt.mLifeMinMax.x, pt.mLifeMinMax.y);
		p.mUV_SPF = pt.mUV_INV_FPS;
		if (pt.mUVAnimFramesPerSec == 0.f && (pt.mUVAnimColRow.x != 1 || pt.mUVAnimColRow.y != 1))
		{
			int numFrames = pt.mUVAnimColRow.x * pt.mUVAnimColRow.y;
			p.mUV_SPF = p.mLifeTime / numFrames;
			assert(p.mUV_SPF > 0);

		}
		p.mCurLifeTime = 0.0f;
		float size = Random(pt.mSizeMinMax.x, pt.mSizeMinMax.y)*scale;
		float ratio = Random(pt.mSizeRatioMinMax.x, pt.mSizeRatioMinMax.y);
		p.mSize = Vec2(size * ratio, size);
		if (mLength != 0 && pt.mAlign == ParticleAlign::Direction)
		{
			p.mSize.x = p.mSize.x * (mLength / size);
		}

		float scalevel = Random(pt.mScaleVelMinMax.x, pt.mScaleVelMinMax.y)*scale;
		float svratio = Random(pt.mScaleVelRatio.x, pt.mScaleVelRatio.y);
		p.mScaleSpeed = Vec2(scalevel * svratio, scalevel);
		p.mRot = Random(pt.mRotMinMax.x, pt.mRotMinMax.y);
		p.mRotSpeed = Random(pt.mRotSpeedMinMax.x, pt.mRotSpeedMinMax.y);
		p.mIntensity = Random(pt.mIntensityMinMax.x, pt.mIntensityMinMax.y);
		p.mColor = pt.mColor * mEmitterColor;

		if (pt.IsLocalSpace())
		{
			p.mPosWorld = mSelf->GetLocation().ApplyForward(p.mPos);
		}
		else
		{
			p.mPosWorld = p.mPos;
		}

		if (!pt.mGeometryPath.empty())
		{
			if (!p.mMeshObject){
				p.mMeshObject = pt.mMeshObject->Clone();
				if (!mShaderDefines.empty()){					
					auto mat = p.mMeshObject->GetMaterial();
					if (mat){
						for (auto& it : mShaderDefines){
							mat->AddShaderDefine(it.first.c_str(), it.second.c_str());
						}
						//mat->ApplyShaderDefines();
					}
				}
			}
			if (mVisible){
				p.mMeshObject->DetachFromScene();
			}
			p.mMeshObject->SetPosition(p.mPosWorld);
			p.mMeshObject->SetScale(Vec3(scale));
			p.mMeshObject->SetDirection(mSelf->GetDirection());
		}

		if (pt.mParticleEmitter != -1)
		{
			if (!p.mParticleEmitter)
			{
				p.mParticleEmitter = ParticleSystem::GetInstance().GetParticleEmitter(pt.mParticleEmitter);					
			}
			p.mParticleEmitter->Active(true, true);
			p.mParticleEmitter->SetPosition(p.mPosWorld);
			p.mParticleEmitter->SetScale(Vec3(scale));
		}

		if (pt.mPLRangeMinMax != Vec2::ZERO)
		{
			if (!p.mPointLight)
			{
				Vec4 color(pt.mColor.GetVec4());
				Vec3 color3(color.x, color.y, color.z);
				p.mPointLight = Renderer::GetInstance().CreatePointLight(p.mPosWorld, pt.mPLRangeMinMax.y*scale, color3, pt.mIntensityMinMax.y,
					p.mLifeTime, true);
			}
		}
		return &p;
	}

	Particle& GetParticle(unsigned templateIdx, unsigned particleIdx){
		if (!mManualEmitter)
		{
			assert(0);
		}
		const ParticleTemplate& pt = (*(mTemplates.const_get()))[templateIdx];
		PARTICLES& particles = *(mParticlesPerTemplate[&pt]);
		return particles.GetAt(particleIdx);
	}
};

//---------------------------------------------------------------------------
ParticleEmitterPtr ParticleEmitter::Create(){
	ParticleEmitterPtr p(new ParticleEmitter(), [](ParticleEmitter* obj){ delete obj; });
	p->mImpl->mSelfPtr = p;
	return p;
}

ParticleEmitterPtr ParticleEmitter::Create(const ParticleEmitter& other){
	ParticleEmitterPtr p(new ParticleEmitter(other), [](ParticleEmitter* obj){ delete obj; });
	p->mImpl->mSelfPtr = p;
	return p;
}

ParticleEmitter::ParticleEmitter()
	:mImpl(new Impl(this))
{
}

ParticleEmitter::ParticleEmitter(const ParticleEmitter& other)
	: mImpl(new Impl(this, *other.mImpl))
{

}
ParticleEmitter::~ParticleEmitter(){
}

ParticleEmitterPtr ParticleEmitter::Clone(){
	return mImpl->Clone();
}

bool ParticleEmitter::Load(const char* filepath, bool reload) {
	return mImpl->Load(filepath, reload);
}

bool ParticleEmitter::UpdateEmitter(float elapsedTime) {
	return mImpl->UpdateEmitter(elapsedTime);
}

unsigned ParticleEmitter::GetEmitterID() const {
	return mImpl->GetEmitterID();
}

void ParticleEmitter::Active(bool a, bool pending) {
	mImpl->Active(a, pending);
}

void ParticleEmitter::Stop() {
	mImpl->Stop();
}

void ParticleEmitter::ProcessDeleteOnStop(bool stopping, bool force) {
	mImpl->ProcessDeleteOnStop(stopping, force);
}

void ParticleEmitter::StopImmediate() {
	mImpl->StopImmediate();
}

void ParticleEmitter::SetVisible(bool visible) {
	mImpl->SetVisible(visible);
}

bool ParticleEmitter::GetVisible() const{
	return mImpl->mVisible;
}

bool ParticleEmitter::IsAlive() {
	return mImpl->IsAlive();
}

bool ParticleEmitter::IsActive() const {
	return mImpl->IsActive();
}

void ParticleEmitter::SetEmitterDirection(const fb::Vec3& dir) {
	mImpl->SetEmitterDirection(dir);
}

void ParticleEmitter::SetEmitterColor(const Color& c) {
	mImpl->SetEmitterColor(c);
}

void ParticleEmitter::SetAlpha(float alpha) {
	mImpl->SetAlpha(alpha);
}

float ParticleEmitter::GetAlpha() const {
	return mImpl->GetAlpha();
}

//void ParticleEmitter::UpdateEmit(float dt) {
//	mImpl->UpdateEmit(dt);
//}

void ParticleEmitter::CopyDataToRenderer(float dt) {
	mImpl->CopyDataToRenderer(dt);
}

bool ParticleEmitter::IsInfinite() const {
	return mImpl->IsInfinite();
}

void ParticleEmitter::SetBufferSize(unsigned size) {
	mImpl->SetBufferSize(size);
}

void ParticleEmitter::SetLength(float length) {
	mImpl->SetLength(length);
}

void ParticleEmitter::SetRelativeVelocity(const Vec3& dir, float speed) {
	mImpl->SetRelativeVelocity(dir, speed);
}

void ParticleEmitter::RemoveShaderDefine(const char* def) {
	mImpl->RemoveShaderDefine(def);
}

void ParticleEmitter::AddShaderDefine(const char* def, const char* val) {
	mImpl->AddShaderDefine(def, val);
}

//void ParticleEmitter::ApplyShaderDefine() {
//	mImpl->ApplyShaderDefine();
//}

