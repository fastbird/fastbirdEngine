#include <Engine/StdAfx.h>
#include <Engine/ParticleEmitter.h>
#include <Engine/ParticleManager.h>
#include <Engine/ICamera.h>
#include <Engine/IMeshObject.h>
#include <Engine/ParticleRenderObject.h>
#include <CommonLib/Math/BVaabb.h>

namespace fastbird
{

IParticleEmitter* IParticleEmitter::CreateParticleEmitter()
{
	return FB_NEW(ParticleEmitter);
}

static const float INFINITE_LIFE_TIME = -1.0f;

//-----------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter()
	: mLifeTime(2), mCurLifeTime(0)
	, mEmitterID(0), mInActiveList(false)
	, mClonedTemplates(0)
	, mMaxSize(0.0f)
	, mStop(false)
	, mStopImmediate(false)
	, mEmitterDirection(0, 1, 0)
	, mAdam(0)
	, mManualEmitter(false), mMoveToCam(false)
	, mEmitterColor(1, 1, 1)
	, mLength(0.0f)
	, mRelativeVelocity(0)
	, mRelativeVelocityDir(Vec3::ZERO)
	, mFinalAlphaMod(1.0f)
{
	mObjFlag |= OF_IGNORE_ME;
}

ParticleEmitter::~ParticleEmitter()
{
	FB_FOREACH(it, mParticles)
	{
		auto particles = it->second;
		PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
		for (; itParticle != itEndParticle; ++itParticle)
		{
			if (itParticle->mPointLight)
			{
				gFBEnv->pRenderer->DeletePointLight(itParticle->mPointLight);
				itParticle->mPointLight = 0;
			}

			if (itParticle->mMeshObject)
			{
				itParticle->mMeshObject->DetachFromScene(true);
			}

			if (itParticle->mParticleEmitter)
			{
				itParticle->mParticleEmitter->StopImmediate();
				gFBEnv->pEngine->ReleaseParticleEmitter(itParticle->mParticleEmitter);
				itParticle->mParticleEmitter = 0;
			}
		}
		FB_SAFE_DEL(particles);
	}
	mParticles.clear();
}

//-----------------------------------------------------------------------------
bool ParticleEmitter::Load(const char* filepath)
{
	assert(!mClonedTemplates); // do not load in cloned template
	tinyxml2::XMLDocument doc;
	int err = doc.LoadFile(filepath);
	if (err)
	{
		Log(FB_DEFAULT_DEBUG_ARG, FormatString("Error while parsing particle(%s)!", filepath));
		if (doc.ErrorID() == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
		{
			Log("particle %s is not found!", filepath);
		}
		const char* errMsg = doc.GetErrorStr1();
		if (errMsg)
			Log("\t%s", errMsg);
		errMsg = doc.GetErrorStr2();
		if (errMsg)
			Log("\t%s", errMsg);
		return false;
	}

	tinyxml2::XMLElement* pPE = doc.FirstChildElement("ParticleEmitter");
	if (!pPE)
	{
		Error("Invaild particle file. %s", filepath);
		return false;
	}

	const char* sz = pPE->Attribute("emitterLifeTime");
	if (sz)
		mLifeTime = StringConverter::parseReal(sz);
	sz = pPE->Attribute("emitterID");
	if (sz)
		mEmitterID = StringConverter::parseUnsignedInt(sz);
	else
	{
		assert(0);
		Log("Emitter id omitted! (%s)", filepath);
	}

	sz = pPE->Attribute("moveToCam");
	if (sz)
	{
		mMoveToCam = StringConverter::parseBool(sz);
	}

	sz = pPE->Attribute("manualControl");
	if (sz)
		mManualEmitter = StringConverter::parseBool(sz);

	mPTemplates.clear();
	tinyxml2::XMLElement* pPT = pPE->FirstChildElement("ParticleTemplate");
	while (pPT)
	{
		mPTemplates.push_back(ParticleTemplate());
		ParticleTemplate& pt = mPTemplates.back();

		sz = pPT->Attribute("texture");
		if (sz)
		{
			pt.mTexturePath = sz;
			ToLowerCase(pt.mTexturePath);
		}
		
		

		sz = pPT->Attribute("pointLightRangeMinMax");
		if (sz)
			pt.mPLRangeMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("geometry");
		if (sz)
		{
			pt.mGeometryPath = sz;
			assert(pt.mTexturePath.empty());
			if (!pt.mGeometryPath.empty())
			{
				pt.mMeshObject = gFBEnv->pEngine->GetMeshObject(pt.mGeometryPath.c_str());
			}
		}

		sz = pPT->Attribute("particleEmitter");
		if (sz)
		{
			pt.mParticleEmitter = StringConverter::parseUnsignedInt(sz);
			assert(pt.mTexturePath.empty());
		}

		sz = pPT->Attribute("startAfter");
		if (sz)
			pt.mStartAfter = StringConverter::parseReal(sz);

		sz = pPT->Attribute("emitPerSec");
		if (sz)
			pt.mEmitPerSec = StringConverter::parseReal(sz);

		sz = pPT->Attribute("numInitialParticle");
		if (sz)
			pt.mInitialParticles = StringConverter::parseUnsignedInt(sz);

		sz = pPT->Attribute("maxParticle");
		if (sz)
			pt.mMaxParticle = StringConverter::parseUnsignedInt(sz);

		sz = pPT->Attribute("deleteWhenFull");
		if (sz)
			pt.mDeleteWhenFull = StringConverter::parseBool(sz);

		sz = pPT->Attribute("cross");
		if (sz)
			pt.mCross = StringConverter::parseBool(sz);

		sz = pPT->Attribute("preMultiAlpha");
		if (sz)
			pt.mPreMultiAlpha = StringConverter::parseBool(sz);
		
		sz = pPT->Attribute("blendMode");
		if (sz)
			pt.mBlendMode = ParticleBlendMode::ConvertToEnum(sz);

		float glow = 0.f;
		sz = pPT->Attribute("glow");
		if (sz)
			glow = StringConverter::parseReal(sz);

		sz = pPT->Attribute("posOffset");
		if (sz)
			pt.mPosOffset = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("lifeTimeMinMax");
		if (sz)
			pt.mLifeMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("deleteWhenStop");
		if (sz)
			pt.mDeleteWhenStop = StringConverter::parseBool(sz);

		sz = pPT->Attribute("align");
		if (sz)
			pt.mAlign = ParticleAlign::ConvertToEnum(sz);

		sz = pPT->Attribute("stretchMax");
		if (sz)
			pt.mStretchMax = StringConverter::parseReal(sz);

		sz = pPT->Attribute("useRelativeVelocity");
		if (sz)
			pt.mUseRelativeVelocity = StringConverter::parseBool(sz);

		sz = pPT->Attribute("DefaultDirection");
		if (sz)
			pt.mDefaultDirection = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("emitTo");
		if (sz)
			pt.mEmitTo = ParticleEmitTo::ConvertToEnum(sz);

		sz = pPT->Attribute("cameraPulling");
		if (sz)
		{
			pt.mCameraPulling = StringConverter::parseReal(sz);
		}

		sz = pPT->Attribute("range");
		if (sz)
			pt.mRangeType = ParticleRangeType::ConvertToEnum(sz);

		sz = pPT->Attribute("rangeRadius");
		if (sz)
			pt.mRangeRadius = StringConverter::parseReal(sz);

		sz = pPT->Attribute("rangeRadiusMin");
		if (sz)
			pt.mRangeRadiusMin = StringConverter::parseReal(sz);

		sz = pPT->Attribute("posInterpolation");
		if (sz)
			pt.mPosInterpolation = StringConverter::parseBool(sz);

		sz = pPT->Attribute("sizeMinMax");
		if (sz)
			pt.mSizeMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("sizeRatioMinMax");
		if (sz)
			pt.mSizeRatioMinMax = StringConverter::parseVec2(sz);

		mMaxSize = std::max(mMaxSize, 
			std::max(
			pt.mSizeMinMax.x * std::max(pt.mSizeRatioMinMax.x, pt.mSizeRatioMinMax.y),
				pt.mSizeMinMax.y));

		sz = pPT->Attribute("pivot");
		if (sz)
			pt.mPivot = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("scaleVelMinMax");
		if (sz)
			pt.mScaleVelMinMax = StringConverter::parseVec2(sz);
		sz = pPT->Attribute("scaleVelRatio");
		if (sz)
			pt.mScaleVelRatio = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("scaleAccel");
		if (sz)
			pt.mScaleAccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("scaleAccelUntil");
		if (sz)
			pt.mScaleAccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("scaleDeaccel");
		if (sz)
			pt.mScaleDeaccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("scaleDeaccelAfter");
		if (sz)
			pt.mScaleDeaccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("velocityMinMax");
		if (sz)
			pt.mVelocityMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("velocityDirectionMin");
		if (sz)
			pt.mVelocityDirMin = StringConverter::parseVec3(sz);
		sz = pPT->Attribute("velocityDirectionMax");
		if (sz)
			pt.mVelocityDirMax = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("velocityToCenter");
		if (sz)
			pt.mVelocityToCenter = StringConverter::parseBool(sz);

		sz = pPT->Attribute("accel");
		if (sz)
			pt.mAccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("accelUntil");
		if (sz)
			pt.mAccel.y = StringConverter::parseReal(sz) * 0.01f;
		sz = pPT->Attribute("deaccel");
		if (sz)
			pt.mDeaccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("deaccelAfter");
		if (sz)
			pt.mDeaccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("rotMinMax");
		if (sz)
			pt.mRotMinMax = Radian(StringConverter::parseVec2(sz));
		sz = pPT->Attribute("rotSpeedMin");
		if (sz)
			pt.mRotSpeedMinMax.x = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotSpeedMax");
		if (sz)
			pt.mRotSpeedMinMax.y = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotAccel");
		if (sz)
			pt.mRotAccel.x = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotAccelUntil");
		if (sz)
			pt.mRotAccel.y = StringConverter::parseReal(sz) * 0.01f;
		sz = pPT->Attribute("rotDeaccel");
		if (sz)
			pt.mRotDeaccel.x = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotDeaccelAfter");
		if (sz)
			pt.mRotDeaccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("fadeInUntil");
		if (sz)
			pt.mFadeInOut.x = StringConverter::parseReal(sz) * 0.01f;
		sz = pPT->Attribute("fadeOutAfter");
		if (sz)
			pt.mFadeInOut.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("Intensity");
		if (sz)
			pt.mIntensityMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("color");
		if (sz)
			pt.mColor = StringConverter::parseColor(sz);
		pt.mColorEnd = pt.mColor;

		sz = pPT->Attribute("colorEnd");
		if (sz)
			pt.mColorEnd = StringConverter::parseColor(sz);

		sz = pPT->Attribute("uvAnimColRow");
		if (sz)
			pt.mUVAnimColRow = StringConverter::parseVec2I(sz);

		sz = pPT->Attribute("uvAnimFramesPerSec");
		if (sz)
		{
			pt.mUVAnimFramesPerSec = StringConverter::parseReal(sz);
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
			pt.mAnimPendulum = StringConverter::parseBool(sz);
		}

		sz = pPT->Attribute("uvFlow");
		if (sz)
		{
			pt.mUVFlow = StringConverter::parseVec2(sz);
		}

		sz = pPT->Attribute("depthFade");
		if (sz)
		{
			pt.mDepthFade = StringConverter::parseBool(sz);
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
			ParticleRenderObject::Key key(pt.mTexturePath.c_str(), desc, glow>0.f, pt.mDepthFade);
			ParticleRenderObject* pro = ParticleRenderObject::GetRenderObject(key, created);
			pt.mParticleRenderObject = pro;
			assert(pro);
			auto material = pro->GetMaterial();
			assert(material);
			if (created)
			{
				switch (pt.mBlendMode)
				{
				case ParticleBlendMode::InvColorBlend:
				{
					material->AddShaderDefine("_INV_COLOR_BLEND", "1");
					break;
				}
				case ParticleBlendMode::Replace:
				{
					material->AddShaderDefine("_PRE_MULTIPLIED_ALPHA", "1");
					break;
				}
				}

				if (pt.mPreMultiAlpha)
				{
					material->AddShaderDefine("_PRE_MULTIPLIED_ALPHA", "1");
				}

				if (glow == 0.f)
				{
					material->AddShaderDefine("_NO_GLOW", "1");
					material->SetGlow(false);
				}
				else
				{
					material->RemoveShaderDefine("_NO_GLOW");
					material->SetGlow(true);
				}

				if (!pt.mDepthFade)
				{
					material->AddShaderDefine("_NO_DEPTH_FADE", "1");
				}
				pro->GetMaterial()->ApplyShaderDefines();
				material->SetMaterialParameters(0, Vec4(glow, 0, 0, 0));
			}
		}
		
		pPT = pPT->NextSiblingElement();
	}

	/*
	Prototype doens't need to have this.
	FB_FOREACH(it, mPTemplates)
	{
	mParticles.Insert(PARTICLESS::value_type(&(*it), FB_NEW(PARTICLES)));
	mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 0.f));
	}
	*/

	return true;
}

IObject* ParticleEmitter::Clone() const
{
	ParticleEmitter* cloned = FB_NEW(ParticleEmitter);
	cloned->mAdam = (ParticleEmitter*)this;
	cloned->mClonedTemplates = &mPTemplates;
	cloned->mEmitterID = mEmitterID;
	cloned->mMoveToCam = mMoveToCam;
	cloned->mLifeTime = mLifeTime;
	cloned->mMaxSize = mMaxSize;
	cloned->mManualEmitter = mManualEmitter;
	// EmitterDir is not using currently.
	cloned->mEmitterDirection = mEmitterDirection;

	FB_FOREACH(it, mPTemplates)
	{
		PARTICLES* particles = FB_NEW(PARTICLES);
		
		cloned->mMaxParticles.Insert(std::make_pair(&(*it), it->mMaxParticle));
		particles->Init(cloned->mMaxParticles[&(*it)] * 2);
		cloned->mParticles.Insert(PARTICLESS::value_type(&(*it), particles));
		cloned->mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 0.f));
		cloned->mNextEmits[&(*it)] = (float)it->mInitialParticles;
		cloned->mAliveParticles.Insert(std::make_pair(&(*it), 0));
		
	}
	cloned->mBoundingVolumeWorld = BoundingVolume::Create(BoundingVolume::BV_AABB);
	cloned->mBoundingVolume = 0;

	return cloned;
}

//// when you use pending activation, be sure you have a reference count or not to use smart pointer.
void ParticleEmitter::Active(bool a, bool pending)
{
	if (a && (!mInActiveList || mStop))
	{
		if (!mInActiveList)
		{
			if (!pending)
				ParticleManager::GetParticleManager().AddActiveParticle(this);
			else
				ParticleManager::GetParticleManager().AddActiveParticlePending(this);
		}
		mInActiveList = true;
		mStop = false;
		mStopImmediate = false;
		mCurLifeTime = 0.f;
		if (mClonedTemplates)
		{
			FB_FOREACH(it, (*mClonedTemplates))
			{
				mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 1.f));
				mNextEmits[&(*it)] = (float)it->mInitialParticles;
			}
		}
		mStartPos = mTransformation.GetTranslation();
		ProcessDeleteOnStop(false);
	}
	else if (!a && mInActiveList)
	{
		ProcessDeleteOnStop(true, true);

		ParticleManager::GetParticleManager().RemoveDeactiveParticle(this);
		mInActiveList = false;
	}
}

void ParticleEmitter::Stop()
{
	mStop = true;
	ProcessDeleteOnStop(true);
}

void ParticleEmitter::ProcessDeleteOnStop(bool stopping, bool force)
{
	for (auto& pt : *mClonedTemplates)
	{
		if (pt.mDeleteWhenStop || force)
		{
			auto it = mParticles.Find(&pt);
			if (it == mParticles.end())
				continue;
			PARTICLES& particles = *(it->second);
			if (stopping)
			{
				for (auto& p : particles)
				{
					p.mCurLifeTime = p.mLifeTime;
					if (p.mMeshObject)
						p.mMeshObject->DetachFromScene();
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
					if (p.mMeshObject)
						p.mMeshObject->AttachToScene();
					if (p.mPointLight)
						p.mPointLight->SetEnabled(true);
					if (p.mParticleEmitter)
						p.mParticleEmitter->Active(true, true);
				}
			}
		}
	}
}

void ParticleEmitter::StopImmediate()
{
	mStop = true;
	mStopImmediate = true;
}

void ParticleEmitter::SetVisibleParticle(bool visible)
{
	SetShow(visible);
	for (auto& pt : *mClonedTemplates)
	{
		PARTICLES& particles = *(mParticles[&pt]);
		for (auto& p : particles)
		{
			if (p.mMeshObject)
				p.mMeshObject->SetShow(visible);
			if (p.mPointLight)
				p.mPointLight->SetEnabled(visible);
			if (p.mParticleEmitter)
				p.mParticleEmitter->SetVisibleParticle(visible);
		}
	}
}

bool ParticleEmitter::IsAlive()
{
	return mInActiveList;
}

void ParticleEmitter::SetAlpha(float alpha){
	mFinalAlphaMod = alpha;
	FB_FOREACH(it, mParticles)
	{
		auto particles = it->second;
		PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
		for (; itParticle != itEndParticle; ++itParticle)
		{
			if (itParticle->mMeshObject)
			{
				itParticle->mMeshObject->SetForceAlphaBlending(true, alpha);
			}
		}
	}
	
}

bool ParticleEmitter::Update(float dt)
{
	mCurLifeTime+=dt;
	if ((!IsInfinite() && mCurLifeTime > mLifeTime ) || mStop)
	{
		mStop = true;
		int numAlive = 0;
		if (!mStopImmediate)
		{
			FB_FOREACH(it, mParticles)
			{
				const ParticleTemplate* pt = it->first;
				PARTICLES* particles = it->second;
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
			FB_FOREACH(it, mParticles)
			{
				const ParticleTemplate* pt = it->first;
				PARTICLES* particles = it->second;
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


	mBoundingVolumeWorld->Invalidate();
	// update existing partices
	FB_FOREACH(it, mParticles)
	{
		const ParticleTemplate* pt = it->first;
		PARTICLES* particles = it->second;
		PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
		for (; itParticle!=itEndParticle; ++itParticle)
		{
			Particle& p = *itParticle;
			if (p.IsAlive() && p.mLifeTime!=-2.f)
			{
				if (!p.IsInfinite())
				{
					p.mCurLifeTime += dt;
					if (p.mCurLifeTime>=p.mLifeTime)
					{
						if (p.mMeshObject)
							p.mMeshObject->DetachFromScene();
						if (p.mPointLight)
						{
							gFBEnv->pRenderer->DeletePointLight(p.mPointLight);
							p.mPointLight = 0;
						}
						if (p.mParticleEmitter)
						{
							p.mParticleEmitter->Stop();
							gFBEnv->pEngine->ReleaseParticleEmitter(p.mParticleEmitter);
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
						p.mVelocity += pt->mAccel.x * dt;
					}
					if (normTime > pt->mDeaccel.y)
					{
						p.mVelocity -= pt->mDeaccel.x * dt;
					}
				}
				if (pt->mVelocityToCenter)
				{
					if (pt->mEmitTo == ParticleEmitTo::WorldSpace)
						p.mVelDir = mTransformation.GetTranslation() - p.mPos;
					else
						p.mVelDir = -p.mPos;
				}
					
				// change pos
				p.mPos += p.mVelDir * (p.mVelocity * dt);
				
				if (pt->IsLocalSpace())
				{
					p.mPosWorld = mTransformation.ApplyForward(p.mPos);
				}
				else
				{
					p.mPosWorld = p.mPos;
				}
				if (pt->mCameraPulling != 0)
				{
					p.mPosWorld -= gFBEnv->pRenderer->GetMainCamera()->GetDir() * pt->mCameraPulling;
				}

				//change rot speed
				if (!p.IsInfinite())
				{
					float sign = Sign(p.mRotSpeed);
					if (normTime < pt->mRotAccel.y)
					{
						p.mRotSpeed += sign * pt->mRotAccel.x * dt;
					}
					if (normTime > pt->mRotDeaccel.y)
					{
						p.mRotSpeed -= sign * pt->mRotDeaccel.x * dt;
					}
				}
				// change rot
				p.mRot += p.mRotSpeed * dt;
				

				float scale = mTransformation.GetScale().x;
				// change scale speed
				if (!p.IsInfinite())
				{
					if (normTime < pt->mScaleAccel.y)
					{
						p.mScaleSpeed += pt->mScaleAccel.x * scale * dt;
					}
					if (normTime > pt->mScaleDeaccel.y)
					{
						p.mScaleSpeed -= pt->mScaleDeaccel.x * scale * dt;
					}
				}
				// change scale
				p.mSize += p.mScaleSpeed * dt;
				if (p.mSize.x < 0.0f)
					p.mSize.x = 0.0f;
				if (p.mSize.y < 0.0f)
					p.mSize.y = 0.0f;

				Vec3 toSidePos = GetForward() * p.mSize.x;
				mBoundingVolumeWorld->Merge(p.mPosWorld - pt->mPivot.x * toSidePos);
				mBoundingVolumeWorld->Merge(p.mPosWorld + (1.0f - pt->mPivot.x) * toSidePos);

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
					p.mUVFrame += dt;
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
		auto cam = gFBEnv->pRenderer->GetMainCamera();
		if (cam)
		{
			auto& camPos = cam->GetPos();
			float emitterNormTime = std::min((mCurLifeTime / mLifeTime) * 2.0f, 0.85f);
			auto toStartPos = mStartPos - camPos;
			float dist = toStartPos.Length();
			float d = cam->GetDir().Dot(toStartPos.NormalizeCopy());
			if (d < 0.2f)
			{
				auto oppositeStart = cam->GetDir() * dist;
				float angle = oppositeStart.AngleBetween(toStartPos);
				float rotAngle = cam->GetFOV()*.5f;
				auto axis = oppositeStart.Cross(toStartPos).NormalizeCopy();
				auto desiredLocation = camPos + (Quat(rotAngle, axis) * oppositeStart);				
				auto dest = Lerp(desiredLocation, camPos, emitterNormTime);
				mTransformation.SetTranslation(dest);
				mFinalAlphaMod += emitterNormTime *.5f;
				mFinalAlphaMod = std::min(1.0f, mFinalAlphaMod);
			}
			else
			{
				auto dest = Lerp(mStartPos, camPos, emitterNormTime);
				mTransformation.SetTranslation(dest);
				mFinalAlphaMod = 1.0f;
			}
		}

	}
	assert(mBoundingVolumeWorld->GetBVType()==BoundingVolume::BV_AABB);
	BVaabb* pAABB = (BVaabb*)mBoundingVolumeWorld.get();
	pAABB->Expand(mMaxSize);

	if (!mStop)
		UpdateEmit(dt);

	CopyDataToRenderer(dt);

	return true;
}

void ParticleEmitter::UpdateEmit(float dt)
{
	// emit
	int i=0;
	FB_FOREACH(itPT, (*mClonedTemplates) )
	{
		const ParticleTemplate& pt = *itPT;
		if (pt.mStartAfter > mCurLifeTime)
			continue;

		unsigned alives = mAliveParticles[&pt];
		unsigned& maxParticles = mMaxParticles[&pt];
		if (alives >= maxParticles && !pt.mDeleteWhenFull)
		{
			Log("ParticleEmitter(%u) doubled its buffer.", mEmitterID);
			mParticles[&pt]->DoubleSize();
			maxParticles *= 2;
		}

		float& nextEmit = mNextEmits[&pt];
		nextEmit += dt * pt.mEmitPerSec;
		float integral;
		nextEmit = modf(nextEmit, &integral);
		int num = (int)integral;
		auto itFind = mLastEmitPos.Find(&pt);
		Particle* p = 0;
		for (int i=0; i<num; i++)
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

void ParticleEmitter::CopyDataToRenderer(float dt)
{
	if (mObjFlag & IObject::OF_HIDE)
		return;
	ICamera* pCamera = gFBEnv->pRenderer->GetMainCamera();
	assert(pCamera);
	//if (pCamera->IsCulled(mBoundingVolumeWorld))
		//return;

	FB_FOREACH(it, mParticles)
	{
		PARTICLES* particles = (it->second);
		const ParticleTemplate* pt = it->first;
		
		ParticleRenderObject* pro = pt->mParticleRenderObject;

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
					Log("Emitter(%u) tried lock %u but only %u locked.", mEmitterID, numVertices, numWritable);
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
								Mat33 toViewRot = pCamera->GetViewMat().To33();
								if (i == 0)
								{
									Vec3 worldForward = (mTransformation.GetRotation() * udir);
									
									udir = toViewRot * worldForward;
									udirBackup = udir;
									vdir = toViewRot  * pCamera->GetForward().Cross(worldForward).NormalizeCopy();
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
								Mat33 toViewRot = pCamera->GetViewMat().To33();
								udir = toViewRot * worldForward;
								vdir = toViewRot  * pCamera->GetForward().Cross(worldForward).NormalizeCopy();								
							}
							else
							{
								if (pt->IsAlignDirection() && p.mVelocity != 0)
								{
									Mat33 toViewRot = pCamera->GetViewMat().To33();
									if (i == 0)
									{
										Vec3 worldForward = p.mVelDir;

										udir = toViewRot * worldForward;
										udirBackup = udir;
										vdir = toViewRot  * pCamera->GetForward().Cross(worldForward).NormalizeCopy();
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
							auto pos = GetPos();
							auto prevPos = GetPrevPos();
							if (!IsEqual(mRelativeVelocity, 0.f, 0.001f))
							{
								size.x += std::min(size.x * pt->mStretchMax, std::max(0.f, mRelativeVelocity));
							}
							else if (!IsEqual(pos, prevPos, 0.001f))
							{
								size.x += std::min(size.x*pt->mStretchMax, std::max(0.f, (pos -prevPos).Length() / dt*0.1f - mDistToCam*.1f));
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
								p.mMeshObject->SetPos(p.mPosWorld);
								p.mMeshObject->SetRot(GetRot());
							}
							if (p.mPointLight)
							{
								p.mPointLight->SetPos(p.mPosWorld);
							}
							if (p.mParticleEmitter)
							{
								p.mParticleEmitter->SetPos(p.mPosWorld);
							}
							
						}
							
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
IParticleEmitter::Particle* ParticleEmitter::Emit(const ParticleTemplate& pt)
{
	PARTICLES& particles = *(mParticles[&pt]);
	size_t addedPos = particles.push_back(Particle());
	Particle& p = particles.GetAt(addedPos);
	const auto& vScale =  mTransformation.GetScale();
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
										 p.mPos = mTransformation.GetTranslation();
									 }
	}
		break;
	case ParticleRangeType::Box:
	{
		p.mPos = Random(Vec3(-pt.mRangeRadius), Vec3(pt.mRangeRadius))*scale;
								   if (!pt.IsLocalSpace())
								   {
									   p.mPos += mTransformation.GetTranslation();
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
										  p.mPos += mTransformation.GetTranslation();
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
											  p.mPos += mTransformation.GetTranslation();
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
										p.mPos += mTransformation.GetTranslation();
									}
	}
		break;
	}
	float angle = pt.mDefaultDirection.AngleBetween(GetForward());
	Vec3 posOffset = pt.mPosOffset;
	if (pt.mPosOffset != Vec3::ZERO && pt.mEmitTo == ParticleEmitTo::WorldSpace)
	{
		if (angle > 0.01f)
		{
			Vec3 axis = pt.mDefaultDirection.Cross(GetForward());
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
			velDir = mTransformation.GetTranslation() - p.mPos;
		else
			velDir = -p.mPos;
	}
	else
	{
		velDir = Random(pt.mVelocityDirMin, pt.mVelocityDirMax).NormalizeCopy();
	}
	
	if (angle > 0.01f && pt.mEmitTo == ParticleEmitTo::WorldSpace)
	{
		Vec3 axis = pt.mDefaultDirection.Cross(GetForward());
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
		p.mPosWorld = mTransformation.ApplyForward(p.mPos);
	}
	else
	{
		p.mPosWorld = p.mPos;
	}

	if (!pt.mGeometryPath.empty())
	{
		if (!p.mMeshObject)
			p.mMeshObject = (IMeshObject*)pt.mMeshObject->Clone();
		p.mMeshObject->AttachToScene();
		p.mMeshObject->SetPos(p.mPosWorld);
		p.mMeshObject->SetScale(Vec3(scale));
		p.mMeshObject->SetDir(GetForward());
	}

	if (pt.mParticleEmitter!=-1)
	{
		if (!p.mParticleEmitter)
		{
			p.mParticleEmitter = gFBEnv->pEngine->GetParticleEmitter(pt.mParticleEmitter, false);
		}
		p.mParticleEmitter->Active(true, true);
		p.mParticleEmitter->SetPos(p.mPosWorld);
		p.mParticleEmitter->SetScale(Vec3(scale));
	}

	if (pt.mPLRangeMinMax != Vec2::ZERO)
	{
		if (!p.mPointLight)
		{
			Vec4 color(pt.mColor.GetVec4());
			Vec3 color3(color.x, color.y, color.z);
			p.mPointLight = gFBEnv->pRenderer->CreatePointLight(p.mPosWorld, pt.mPLRangeMinMax.y*scale, color3, pt.mIntensityMinMax.y, 
				p.mLifeTime, true);
		}
	}
	return &p;
}

//-----------------------------------------------------------------------------
IParticleEmitter::Particle* ParticleEmitter::Emit(unsigned templateIdx)
{
	assert(templateIdx < mClonedTemplates->size());
	const ParticleTemplate& pt = (*mClonedTemplates)[templateIdx];
	return Emit(pt);
}

//-----------------------------------------------------------------------------
IParticleEmitter::Particle& ParticleEmitter::GetParticle(unsigned templateIdx, unsigned index)
{
	if (!mManualEmitter)
	{
		assert(0);
	}
	const ParticleTemplate& pt = (*mClonedTemplates)[templateIdx];
	PARTICLES& particles = *(mParticles[&pt]);
	return particles.GetAt(index);
}

//-----------------------------------------------------------------------------
void ParticleEmitter::SetBufferSize(unsigned size)
{
	FB_FOREACH(it, (*mClonedTemplates))
	{
		mMaxParticles[&(*it)] = size;
		mParticles[&(*it)]->Init(size);
	}
}

void ParticleEmitter::SetLength(float len)
{
	if (mLength == len)
		return;
	mLength = len;
	FB_FOREACH(it, mParticles)
	{
		PARTICLES* particles = (it->second);
		const ParticleTemplate* pt = it->first;
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

////-----------------------------------------------------------------------------
//void ParticleEmitter::Sort()
//{
//	FB_FOREACH(it, (*mClonedTemplates))
//	{
//		PARTICLES& ps = *mParticles[&(*it)];
//		PARTICLES::VECTOR& v = ps.GetVector();
//		PARTICLES::iterator& begin = ps.GetRawBeginIter();
//		PARTICLES::iterator& end = ps.GetRawEndIter();
//		Vec3 pos = gFBEnv->pRenderer->GetMainCamera()->GetPos();
//		std::sort(v.begin(), v.end()-1, [&pos](const PARTICLES::value_type& v1, const PARTICLES::value_type& v2)->bool {
//			float distance1 = (v1.mPosWorld - pos).LengthSQ();
//			float distance2 = (v2.mPosWorld - pos).LengthSQ();
//			return distance2 > distance1;
//		});
//
//		begin = v.begin();
//		end = v.end()-1;
//	}
//}


void ParticleEmitter::SetRelativeVelocity(const Vec3& dir, float speed)
{
	if (speed > 0)
		mRelativeVelocityDir = dir;

	mRelativeVelocity = speed;
}

}