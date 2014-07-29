#include <Engine/StdAfx.h>
#include <Engine/Renderer/ParticleEmitter.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/ICamera.h>
#include <Engine/RenderObjects/ParticleRenderObject.h>
#include <CommonLib/Math/BVaabb.h>

namespace fastbird
{

IParticleEmitter* IParticleEmitter::CreateParticleEmitter()
{
	return new ParticleEmitter;
}

static const float INFINITE_LIFE_TIME = -1.0f;

//-----------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter()
	: mLifeTime(2), mCurLifeTime(0)
	, mEmitterID(0), mInActiveList(false)
	, mClonedTemplates(0)
	, mMaxSize(0.0f)
	, mStop(false)
{
}

ParticleEmitter::~ParticleEmitter()
{
	FB_FOREACH(it, mParticles)
	{
		SAFE_DELETE(it->second);
	}

}

IObject* ParticleEmitter::Clone() const
{
	ParticleEmitter* cloned = new ParticleEmitter();
	cloned->mClonedTemplates = &mPTemplates;
	cloned->mEmitterID = mEmitterID;
	cloned->mLifeTime = mLifeTime;

	FB_FOREACH(it, mPTemplates)
	{
		PARTICLES* particles = new PARTICLES();
		particles->Init(500);
		cloned->mParticles.Insert(PARTICLESS::value_type(&(*it), particles));
		cloned->mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 1.f));
	}
	cloned->mBoundingVolumeWorld = BoundingVolume::Create(BoundingVolume::BV_AABB);
	cloned->mBoundingVolume = 0;

	return cloned;
}

void ParticleEmitter::Active(bool a)
{
	if (a && !mInActiveList)
	{
		ParticleManager::GetParticleManager().AddActiveParticle(this);
		mInActiveList = true;
	}
	else if (!a && mInActiveList)
	{
		ParticleManager::GetParticleManager().RemoveDeactiveParticle(this);
		mInActiveList = false;
	}
}

void ParticleEmitter::Stop()
{
	mStop = true;
}

bool ParticleEmitter::Update(float elapsedTime)
{
	mCurLifeTime+=elapsedTime;
	if ((!IsInfinite() && mCurLifeTime > mLifeTime ) || mStop)
	{
		mStop = true;
		int numAlive = 0;
		FB_FOREACH(it, mParticles)
		{
			const ParticleTemplate* pt = it->first;
			PARTICLES* particles = it->second;
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle!=itEndParticle; ++itParticle)
			{
				if (itParticle->mLifeTime == INFINITE_LIFE_TIME)
				{
					itParticle->mLifeTime = 0.0f;
				}
				if (itParticle->IsAlive())
					++numAlive;
			}
		}
		if (numAlive == 0)
		{
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
			if (p.IsAlive())
			{
				if (!p.IsInfinite())
				{
					p.mCurLifeTime += elapsedTime;
					if (p.mCurLifeTime>=p.mLifeTime)
					{
						p.mLifeTime = 0.0f; // mark dead.
						p.mCurLifeTime = 0.0f;
						continue;
					}
				}
				// change velocity
				float normTime = p.mCurLifeTime / p.mLifeTime;
				if (normTime < pt->mAccel.y)
				{
					p.mVelocity += pt->mAccel.x * elapsedTime;
				}
				if (normTime > pt->mDeaccel.y)
				{
					p.mVelocity -= pt->mDeaccel.x * elapsedTime;
				}
				// change pos
				p.mPos += p.mVelDir * (p.mVelocity * elapsedTime);
				if (pt->IsLocalSpace())
				{
					p.mPosWorld = mTransformation.ApplyForward(p.mPos);
				}
				mBoundingVolumeWorld->Merge(p.mPosWorld);

				//change rot speed
				float sign = Sign(p.mRotSpeed);
				if (normTime < pt->mRotAccel.y)
				{
					p.mRotSpeed += sign * pt->mRotAccel.x * elapsedTime;
				}
				if (normTime > pt->mRotDeaccel.y)
				{
					p.mRotSpeed -= sign * pt->mRotDeaccel.x * elapsedTime;
				}
				// change rot
				p.mRot += p.mRotSpeed * elapsedTime;
				

				// change scale speed
				if (normTime < pt->mScaleAccel.y)
				{
					p.mScaleSpeed += pt->mScaleAccel.x * elapsedTime;
				}
				if (normTime > pt->mScaleDeaccel.y)
				{
					p.mScaleSpeed -= pt->mScaleDeaccel.x * elapsedTime;
				}
				// change scale
				p.mSize += p.mScaleSpeed * elapsedTime;
				if (p.mSize.x < 0.0f)
					p.mSize.x = 0.0f;
				if (p.mSize.y < 0.0f)
					p.mSize.y = 0.0f;

				// update alpha
				if (normTime < pt->mFadeInOut.x)
					p.mAlpha = normTime / pt->mFadeInOut.x;
				else
					p.mAlpha = 1.0f;

				if (normTime > pt->mFadeInOut.y)
					p.mAlpha = (1.0f - normTime) / (1.0f - pt->mFadeInOut.y);


				// uv frame
				if (pt->mUVAnimFramesPerSec!=0.0f && (pt->mUVAnimRowCol.x > 1 || pt->mUVAnimRowCol.y > 1))
				{
					p.mUVFrame += elapsedTime;
					if (p.mUVFrame > pt->mUV_INV_FPS)
					{
						p.mUVIndex.y+=1;
						if (p.mUVIndex.y >= pt->mUVAnimRowCol.y)
						{
							p.mUVIndex.y = 0;
							p.mUVIndex.x +=1;
							if (p.mUVIndex.x >= pt->mUVAnimRowCol.x)
							{
								p.mUVIndex.x = 0;
							}
						}
					}
				}
			}
		}
	}
	assert(mBoundingVolumeWorld->GetBVType()==BoundingVolume::BV_AABB);
	BVaabb* pAABB = (BVaabb*)mBoundingVolumeWorld.get();
	pAABB->Expand(mMaxSize);

	if (!mStop)
		UpdateEmit(elapsedTime);

	CopyDataToRenderer();

	return true;
}

void ParticleEmitter::UpdateEmit(float elapsedTime)
{
	// emit
	int i=0;
	FB_FOREACH(itPT, (*mClonedTemplates) )
	{
		const ParticleTemplate& pt = *itPT;

		PARTICLES& particles = *(mParticles[&pt]);
		float& nextEmit = mNextEmits[&pt];
		nextEmit += elapsedTime * pt.mEmitPerSec;
		float integral;
		nextEmit = modf(nextEmit, &integral);
		int num = (int)integral;
		for (int i=0; i<num; i++)
		{
			particles.push_back(Particle());
			Particle& p = particles.back();
			switch(pt.mRangeType)
			{
			case ParticleRangeType::Point:
				{
					if (pt.IsLocalSpace())
					{
						p.mPos = 0.0f;
					}
					else
					{
						p.mPos = mTransformation.GetTranslation();
					}
				}
				break;
			case ParticleRangeType::Box:
				{
					p.mPos = Random(Vec3(-pt.mRangeRadius), Vec3(pt.mRangeRadius));					
					if (!pt.IsLocalSpace())
					{
						p.mPos += mTransformation.GetTranslation();
					}
				}
				break;
			case ParticleRangeType::Sphere:
				{
					float r = Random(0.0f, pt.mRangeRadius);
					float theta = Random(0.0f, PI);
					float phi = Random(0.0f, TWO_PI);
					float st = sin(theta);
					float ct = cos(theta);
					float sp = sin(phi);
					float cp = cos(phi);
					p.mPos = Vec3(r * st * cp, r*st*sp, r*ct);
					if (!pt.IsLocalSpace())
					{
						p.mPos += mTransformation.GetTranslation();
					}
				}
				break;
			case ParticleRangeType::Hemisphere:
				{
					float r = Random(0.0f, pt.mRangeRadius);
					float theta = Random(0.0f, HALF_PI);
					float phi = Random(0.0f, TWO_PI);
					float st = sin(theta);
					float ct = cos(theta);
					float sp = sin(phi);
					float cp = cos(phi);
					p.mPos = Vec3(r * st * cp, r*st*sp, r*ct);
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
					float height = Random(0.0f, pt.mRangeRadius);
					p.mPos = Vec3(height*tanS*cosT, height*tanS*sinT, height);
					if (!pt.IsLocalSpace())
					{
						p.mPos += mTransformation.GetTranslation();
					}
				}
				break;
			}
			
			p.mVelDir = Random(pt.mVelocityMin, pt.mVelocityMax);
			p.mVelocity = p.mVelDir.Normalize();
			if (pt.mAlign == ParticleAlign::Billboard)
			{
				p.mUDirection = Vec3(1.0f, 0, 0);
			}
			else
			{
				p.mUDirection = p.mVelDir.NormalizeCopy();
			}

			p.mUVIndex = Vec2(0, 0);
			p.mUVStep = Vec2(1.0f / pt.mUVAnimRowCol.x, 1.0f / pt.mUVAnimRowCol.y);
			p.mUVFrame = 0.f;
			p.mLifeTime = Random(pt.mLifeMinMax.x, pt.mLifeMinMax.y);
			p.mCurLifeTime= 0.0f;
			p.mSize = Random(pt.mSizeMin, pt.mSizeMax);
			p.mScaleSpeed = Random(pt.mScaleVelMin, pt.mScaleVelMax);
			p.mRot = Random(pt.mRotMinMax.x, pt.mRotMinMax.y);			
			p.mRotSpeed = Random(pt.mRotSpeedMinMax.x, pt.mRotSpeedMinMax.y);
			p.mIntensity = Random(pt.mIntensityMinMax.x, pt.mIntensityMinMax.y);
		}
	}
}

void ParticleEmitter::CopyDataToRenderer()
{
	ICamera* pCamera = gFBEnv->pRenderer->GetCamera();
	assert(pCamera);
	if ( mBoundingVolumeWorld->WhichSide(pCamera->GetFrustumMin(), pCamera->GetFrustumMax()) < 0 )
		return;

	FB_FOREACH(it, mParticles)
	{
		PARTICLES* particles = (it->second);
		const ParticleTemplate* pt = it->first;
		ParticleRenderObject* pro = ParticleRenderObject::GetRenderObject(pt->mTexturePath.c_str());
		assert(pro);
		size_t num = 0;
		PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
		for (; itParticle!=itEndParticle; ++itParticle)
		{
			if (itParticle->IsAlive())
				num++;
		}

		if (num>0)
		{
			ParticleRenderObject::Vertex* dest = pro->Map(num);
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle!=itEndParticle; ++itParticle)
			{
				Particle& p = *itParticle;
				if (p.IsAlive())
				{
					Vec3 dir = p.mUDirection;
					if (pt->IsLocalSpace())
					{
						if (!pt->IsBillboard())
						{
							dir = mTransformation.ApplyForward(dir);
						}
					}
					dest->mPos = p.mPosWorld;
					dest->mDirection_Intensity = Vec4(dir.x, dir.y, dir.z, p.mIntensity);				
					dest->mPivot_Size = Vec4(pt->mPivot.x, pt->mPivot.y, p.mSize.x, p.mSize.y);
					dest->mRot_Alpha_uv = Vec4(p.mRot, p.mAlpha, p.mUVIndex.x, p.mUVIndex.y);
					dest->mUVStep = p.mUVStep;
					dest++;
				}
			}
			pro->Unmap();
		}
	}
}

//-----------------------------------------------------------------------------
bool ParticleEmitter::Load(const char* filepath)
{
	assert(!mClonedTemplates); // do not load in cloned template
	tinyxml2::XMLDocument doc;
	int err = doc.LoadFile(filepath);
	if (err)
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Error while parsing particle!");
		if (doc.ErrorID()==tinyxml2::XML_ERROR_FILE_NOT_FOUND)
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

	const char* sz =  pPE->Attribute("emitterLifeTime");
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

	tinyxml2::XMLElement* pPT =pPE->FirstChildElement("ParticleTemplate");
	while(pPT)
	{
		mPTemplates.push_back(ParticleTemplate());
		ParticleTemplate& pt = mPTemplates.back();
		
		sz = pPT->Attribute("texture");
		if (sz)
			pt.mTexturePath = sz;
		
		sz = pPT->Attribute("emitPerSec");
		if (sz)
			pt.mEmitPerSec = StringConverter::parseReal(sz);

		sz = pPT->Attribute("lifeTimeMinMax");
		if (sz)
			pt.mLifeMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("align");
		if (sz)
			pt.mAlign = ParticleAlign::ConverToEnum(sz);

		sz = pPT->Attribute("emitTo");
		if (sz)
			pt.mEmitTo = ParticleEmitTo::ConverToEnum(sz);

		sz = pPT->Attribute("range");
		if (sz)
			pt.mRangeType = ParticleRangeType::ConverToEnum(sz);

		sz = pPT->Attribute("rangeRadius");
		if (sz)
			pt.mRangeRadius = StringConverter::parseReal(sz);

		sz = pPT->Attribute("sizeMin");
		if (sz)
			pt.mSizeMin = StringConverter::parseVec2(sz);
		sz = pPT->Attribute("sizeMax");
		if (sz)
			pt.mSizeMax = StringConverter::parseVec2(sz);

		mMaxSize = std::max( mMaxSize, std::max(pt.mSizeMax.x, pt.mSizeMax.y) );

		sz = pPT->Attribute("pivot");
		if (sz)
			pt.mPivot = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("scaleVelMin");
		if (sz)
			pt.mScaleVelMin = StringConverter::parseVec2(sz);
		sz = pPT->Attribute("scaleVelMax");
		if (sz)
			pt.mScaleVelMax = StringConverter::parseVec2(sz);

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

		sz = pPT->Attribute("velocityMin");
		if (sz)
			pt.mVelocityMin = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("velocityMax");
		if (sz)
			pt.mVelocityMax = StringConverter::parseVec3(sz);

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

		sz = pPT->Attribute("uvAnimRowCol");
		if (sz)
			pt.mUVAnimRowCol = StringConverter::parseVec2I(sz);

		sz = pPT->Attribute("uvAnimFramesPerSec");
		if (sz)
		{
			pt.mUVAnimFramesPerSec = StringConverter::parseReal(sz);
			if (pt.mUVAnimFramesPerSec != 0.f)
				pt.mUV_INV_FPS = 1.0f / pt.mUVAnimFramesPerSec;
		}

		sz = pPT->Attribute("uvAnimStep");
		if (sz)
			pt.mUVAnimStep = StringConverter::parseVec2(sz);
		
		pPT = pPT->NextSiblingElement();
	}

	/*
	Prototype doens't need to have this.
	FB_FOREACH(it, mPTemplates)
	{
		mParticles.Insert(PARTICLESS::value_type(&(*it), new PARTICLES()));
		mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 0.f));
	}
	*/

	return true;
}
}