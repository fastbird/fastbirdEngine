#include <Engine/StdAfx.h>
#include <Engine/VolumetricCloud.h>
#include <Engine/ILight.h>
#include <Engine/ICamera.h>
#include <Engine/IScene.h>
#include <Engine/ParticleManager.h>

namespace fastbird
{
	inline float OpticalLength(float fLineDensity)
	{
		return CLOUD_OPTICAL_LENGTH_FACTOR * fLineDensity;
	}

	float Phase(const Vec3& vIn, const Vec3& vOut)
	{
		float d = vIn.Dot(vOut);
		return (float)((1.0 + (d*d)) * 3.0 / 4.0);
	}

	//-----------------------------------------------------------------------
	VolumetricCloud::VolumetricCloud()
		: mWLength(0.f), mWWidth(0.f), mWHeight(0.f)
		, mCellSize(0.f)
		, mLength(0), mWidth(0), mHeight(0)
		, mTimeA(0.f), mCurrentBuffer(0)
		, mTime(0.f)
	{
		mColorUpdateInterval[0] = 0;
		mColorUpdateInterval[1] = 1;
	}
	//-----------------------------------------------------------------------
	VolumetricCloud::~VolumetricCloud()
	{
		Cleanup();
	}

	//-----------------------------------------------------------------------
	bool VolumetricCloud::Setup(const CloudProperties& prop)
	{
		mWLength = prop.fLength; // x
		mWWidth = prop.fWidth;	// y
		mWHeight = prop.fHigh;	// z
		mCellSize = prop.fCellSize;
		mLength = (int)(mWLength / mCellSize + 0.5);
		mWidth = (int)(mWWidth / mCellSize + 0.5);
		mHeight = (int)(mWHeight / mCellSize + 0.5);
		mEvolvingSpeed = prop.fEvolvingSpeed;
		mPos = prop.vCloudPos;
		mParticleID = prop.particleID;

		bool b = mSimulator.Setup(mLength, mWidth, mHeight);
		assert(b);
		b = GenerateCloudParticles();
		assert(b);

		auto scene = gFBEnv->pRenderer->GetMainScene();
		if (scene) {
			mWind = scene->GetWindVector();
		}
		else {
			Error(FB_DEFAULT_DEBUG_ARG, "No scene found!");
		}
		mLightIntensity = gFBEnv->pRenderer->GetDirectionalLight(0)->GetIntensity();
		mLightColor = gFBEnv->pRenderer->GetDirectionalLight(0)->GetDiffuse() * mLightIntensity;
		mLightDir = gFBEnv->pRenderer->GetDirectionalLight(0)->GetPosition();
		mCamPos = gFBEnv->pRenderer->GetCamera()->GetPos();
		m_ParticlePool.m_vWindVelocity = mWind;

		return true;
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::Cleanup()
	{
		unsigned num = mSimulator.GetNumCellInVolume();
		for (unsigned i = 0; i < num; ++i)
		{
			mParticleEmitters[i]->StopImmediate();
			mParticleEmitters[i] = 0;
		}
		
		m_ParticlePool.Cleanup();
		mSimulator.Cleanup();
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::AdvanceTime(float time, int interval)
	{
		//Double buffer: switch buffer index between 0 and 1. 
		mCurrentBuffer = 1 - mCurrentBuffer;

		mCamPos = gFBEnv->pRenderer->GetCamera()->GetPos();

		// change cloud density
		if (mEvolvingSpeed != 1.0f) // if not pause evolving
		{
			float fAlpha = (float)(time - mTimeA) / mEvolvingSpeed;
			mSimulator.InterpolateDensitySpace(fAlpha);
			if ((time < mTimeA) || (time >(mTimeA + mEvolvingSpeed)))
			{
				mTimeA = time;
			}
		}

		UpdateCloudPosition(time);
		//SortCloudParticles(gFBEnv->pRenderer->GetCamera()->GetDir());
		static const int iMaxColorInterval = 1000;
		if ((mColorUpdateInterval[mCurrentBuffer] % interval) == 0)
			UpdateCloudParticleColors();
		++mColorUpdateInterval[mCurrentBuffer];
		if (mColorUpdateInterval[mCurrentBuffer] == iMaxColorInterval)
			mColorUpdateInterval[mCurrentBuffer] = 0;
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::PrepareRender()
	{
		auto scene = gFBEnv->pRenderer->GetMainScene();
		if (scene) {
			mWind = scene->GetWindVector();			
		}
		
		mLightIntensity = gFBEnv->pRenderer->GetDirectionalLight(0)->GetIntensity();
		mLightColor = gFBEnv->pRenderer->GetDirectionalLight(0)->GetDiffuse() * mLightIntensity;
		mLightDir = gFBEnv->pRenderer->GetDirectionalLight(0)->GetPos();
		mCamPos = gFBEnv->pRenderer->GetCamera()->GetPos();
		m_ParticlePool.m_vWindVelocity = mWind;
		// copy to emitter
		ParticleEnumerator Enumerator(&m_ParticlePool);
		CloudParticle *pCurParticle = Enumerator.NextParticleFromLastBuffer();
		unsigned idx=0;
		while (pCurParticle)
		{
			Color color = pCurParticle->m_cScatteringColor;
			if (color == Color::Zero)
			{
				mParticleEmitters[idx]->Stop();
			}
			else
			{
				mParticleEmitters[idx]->SetPos(pCurParticle->GetPositionFromLastBuffer());
				mParticleEmitters[idx]->Active(true);
				mParticleEmitters[idx]->SetEmitterColor(color);
			}			
			pCurParticle = Enumerator.NextParticle();
			++idx;
		}
	}

	//-----------------------------------------------------------------------
	bool VolumetricCloud::GenerateCloudParticles()
	{
		bool ret;
		ret = m_ParticlePool.Setup(this, mSimulator.GetNumCellInVolume());
		if (!ret)
			return false;
		// +1 is needed because of the nature of circualr buffer.
		for (unsigned i = 0; i < mSimulator.GetNumCellInVolume(); ++i)
		{
			mParticleEmitters.push_back(gFBEnv->pEngine->GetParticleEmitter(mParticleID, true));
			mParticleEmitters[i]->Active(true);
		}

		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{					
					if (mSimulator.IsCellInVolume(i, j, k))
					{
						ret = m_ParticlePool.AddParticle(i, j, k);
						if (!ret)
							return false;
					}
				}
			}
		}

		return true;
	}

	//-----------------------------------------------------------------------
	void VolumetricCloud::UpdateCloudParticleColors()
	{
		ParticleEnumerator Enumerator(&m_ParticlePool);
		CloudParticle *pCurParticle = Enumerator.NextParticle();
		while (pCurParticle)
		{
			CalculateParticleScatteringColor(pCurParticle);
			pCurParticle = Enumerator.NextParticle();
		}
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::SortCloudParticles(const Vec3& lookdir)
	{
		m_ParticlePool.SortbyViewDistances(lookdir);
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::UpdateCloudPosition(float time)
	{
		if (mTime == 0.0)  
			mTime = time; //first frame

		Vec3 vDisplacement = mWind * (float)(time - mTime);
		mPos += vDisplacement;
		m_ParticlePool.UpdateParticlePositions(time);

		//Change the cloud pos to let it loop moving
		if (mPos.z < -1000) mPos.z += 1500;

		mTime = time;
	}

	//-----------------------------------------------------------------------
	void VolumetricCloud::SetEvolvingSpeed(float speed)
	{
		mEvolvingSpeed = speed;
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::SetLightColor(const Color& color)
	{
		mLightColor = color;
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::SetLightIntensity(float i)
	{
		mLightIntensity = i;
	}
	//-----------------------------------------------------------------------
	void VolumetricCloud::SetWind(const Vec3& windDir, float v)
	{
		mWind = windDir*v;
	}

	//-----------------------------------------------------------------------
	void VolumetricCloud::UpdateViewDistance()
	{
		Vec3 relation_pos = mPos + Vec3((float)(mLength * .5f), (float)(mWidth * .5f), (float)(mHeight * .5f)) - mCamPos;
		mViewDistance = relation_pos.Length();
	}

	//-----------------------------------------------------------------------
	Color VolumetricCloud::CalculateParticleIncidentColor(CloudParticle* p)
	{
		float fDensity;
		float fOpticalDepth;
		float fTransparency;

		Vec3 vCurPt = Vec3((float)p->m_i, (float)p->m_j, (float)p->m_k)
			+ (mLightDir) * CLOUD_SAMPLE_LENGTH;

		Color cIncidentColor = mLightColor;
		float fAlpha = 0.8f;
		while (mSimulator.IsPointInSpace(vCurPt))
		{
			fDensity = mSimulator.GetPointDensity(vCurPt);
			fOpticalDepth = OpticalLength(fDensity);
			fTransparency = (float)exp(-fOpticalDepth);
			float dec = ((1 - fAlpha) * fTransparency + fAlpha);
			cIncidentColor *= dec;
			vCurPt += (mLightDir) * CLOUD_SAMPLE_LENGTH;
		}

		return cIncidentColor;
	}
	//-----------------------------------------------------------------------
	Color VolumetricCloud::CalculateParticleScatteringColor(CloudParticle* p)
	{
		UINT i = p->m_i;
		UINT j = p->m_j;
		UINT k = p->m_k;
		Color cScatteringColor;

		float fDensity = mSimulator.GetCellDensity(i, j, k);

		if (fDensity < CLOUD_MIN_DENSITY)
		{
			p->mVisible = false;
			cScatteringColor = Color(0, 0, 0, 0);
			p->m_cScatteringColor = cScatteringColor;
			return cScatteringColor;
		}
		else
		{
			p->mVisible = true;

			Color cIncidentColor;

			//if (g_SSE4On == true)
			//	// use SSE to optimize CalculateParticleIncidentColor
			//	cIncidentColor = CalculateParticleIncidentColor_SSE(p);
			//else
				cIncidentColor = CalculateParticleIncidentColor(p);

			float fOpticalDepth = OpticalLength(fDensity);
			float fTransparency = (float)exp(-fOpticalDepth);

			Vec3 vViewDir = mCamPos - p->GetPosition();
			cScatteringColor = cIncidentColor + cIncidentColor*
				((float)((1.f - fTransparency) * Phase(mLightDir, vViewDir.NormalizeCopy()) * CLOUD_SCATTER_FACTOR));

			cScatteringColor.a() = 1 - fTransparency; //particle's alpha value = 1 - m_fAttenuation

			p->m_cScatteringColor = cScatteringColor;

			return cScatteringColor;
		}
	}
}