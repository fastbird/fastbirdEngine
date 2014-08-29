#include <Engine/StdAfx.h>
#include <Engine/Renderer/CloudParticle.h>
#include <Engine/Renderer/VolumetricCloud.h>
namespace fastbird
{
	CloudParticle::CloudParticle()
	{
		memset(this, 0, sizeof(CloudParticle));
	}

	CloudParticle::CloudParticle(UINT i, UINT j, UINT k, ParticlePool *pParticlePool) 
		: m_i(i)
		, m_j(j)
		, m_k(k)
		, mParticlePool(pParticlePool)
		, mVisible(false)
		, m_cScatteringColor(1, 1, 1)
	{
	}

	CloudParticle::~CloudParticle()
	{
	}


	const Vec3& CloudParticle::GetPosition()
	{
		//Update thread, get data from current buffer
		assert(mParticlePool && mParticlePool->m_pvPositions[mParticlePool->m_iCurrentBuffer]);
		return mParticlePool->m_pvPositions[mParticlePool->m_iCurrentBuffer][mIndex];
	}

	const Vec3& CloudParticle::GetPositionFromLastBuffer()
	{
		//Render thread, get data from last buffer
		assert(mParticlePool && mParticlePool->m_pvPositions[1 - mParticlePool->m_iCurrentBuffer]);
		return mParticlePool->m_pvPositions[1 - mParticlePool->m_iCurrentBuffer][mIndex];
	}

	double CloudParticle::GetViewDistance()
	{
		assert(mParticlePool && mParticlePool->m_pfViewDistances);
		return mParticlePool->m_pfViewDistances[mIndex];
	}

	ParticlePool::ParticlePool() :
		m_uNumParticles(0),
		m_pfViewDistances(NULL),
		mVolumetricCloud(NULL),
		m_iCurrentBuffer(0)
	{
		//these are for double buffer implementation
		m_pvPositions[0] = NULL;
		m_pvPositions[1] = NULL;
		m_PreTime[0] = 0.0;
		m_PreTime[1] = 0.0;
	}

	ParticlePool::~ParticlePool()
	{
	}



	bool  CompareViewDistance(CloudParticle* pElem1, CloudParticle* pElem2)
	{
		return (pElem1->GetViewDistance() > pElem2->GetViewDistance());
	}


	void ParticlePool::SortbyViewDistances(const Vec3 &vLookDir)
	{
		Vec3 vToParticle;

		for (UINT i = 0; i < m_uNumParticles; i++)
		{
			vToParticle = m_pvPositions[m_iCurrentBuffer][i] - mVolumetricCloud->GetPos();
			m_pfViewDistances[i] = vLookDir.Dot(vToParticle);
		}

		sort(m_v_pCloudParticles[m_iCurrentBuffer].begin(), m_v_pCloudParticles[m_iCurrentBuffer].end(), CompareViewDistance);
	}

	void ParticlePool::UpdateParticlePositions(double fTime)
	{
		if ((m_PreTime[0] == 0.0) || (m_PreTime[1] == 0.0))
		{
			//The first frame, init two time members
			m_PreTime[0] = fTime;
			m_PreTime[1] = fTime;
		}
		float fElapsedTime = (float)(fTime - m_PreTime[m_iCurrentBuffer]);
		Vec3 vDisplacement = m_vWindVelocity * fElapsedTime;
		for (UINT i = 0; i < m_uNumParticles; i++)
		{
			m_pvPositions[m_iCurrentBuffer][i] += vDisplacement;
			//Change the cloud pos to make it loop moving
			if (m_pvPositions[m_iCurrentBuffer][i].z < -1000) m_pvPositions[m_iCurrentBuffer][i].z += 1500;
		}
		m_PreTime[m_iCurrentBuffer] = fTime;
	}

	bool ParticlePool::Setup(VolumetricCloud *pVolumetricCloud, UINT uNumParticles)
	{
		mVolumetricCloud = pVolumetricCloud;

		m_pfViewDistances = FB_ARRNEW(double, uNumParticles);
		if (!m_pfViewDistances) return false;

		m_pvPositions[0] = FB_ARRNEW(Vec3, uNumParticles);
		if (!m_pvPositions[0]) return false;

		m_pvPositions[1] = FB_ARRNEW(Vec3, uNumParticles);
		if (!m_pvPositions[1]) return false;

		m_uNumParticles = uNumParticles;
		return true;
	}

	void ParticlePool::Cleanup()
	{
		if (m_pfViewDistances)
			FB_ARRDELETE(m_pfViewDistances);

		//cleanup double buffer data
		if (m_pvPositions[0])
			FB_ARRDELETE(m_pvPositions[0]);
		if (m_pvPositions[1])
			FB_ARRDELETE(m_pvPositions[1]);

		std::vector< CloudParticle* >::iterator itCurCP, itEndCP = m_v_pCloudParticles[0].end();
		for (itCurCP = m_v_pCloudParticles[0].begin(); itCurCP != itEndCP; ++itCurCP)
		{
			FB_SAFE_DEL(*itCurCP);
		}
		m_v_pCloudParticles[0].clear();
		std::vector< CloudParticle* >::iterator itCurCP2, itEndCP2 = m_v_pCloudParticles[1].end();
		for (itCurCP2 = m_v_pCloudParticles[1].begin(); itCurCP2 != itEndCP2; ++itCurCP2)
		{
			FB_SAFE_DEL(*itCurCP2);
		}
		m_v_pCloudParticles[1].clear();
	}

	bool ParticlePool::AddParticle(UINT i, UINT j, UINT k)
	{
		UINT index = m_v_pCloudParticles[0].size();
		if (index >= m_uNumParticles) return false;

		CloudParticle *pPreParticleBuffer = FB_NEW(CloudParticle)(i, j, k, this);
		if (pPreParticleBuffer == NULL) return false;

		pPreParticleBuffer->mIndex = index;
		m_pvPositions[0][index] = mVolumetricCloud->GetPos() + mVolumetricCloud->GetCellSize() * Vec3((float)i, (float)j, (float)k);

		//add to double buffer 1
		try
		{
			m_v_pCloudParticles[0].push_back(pPreParticleBuffer);
		}
		catch (...)
		{
			return false;
		}

		CloudParticle *pCurrentParticleBuffer = FB_NEW(CloudParticle)(i, j, k, this);
		if (pCurrentParticleBuffer == NULL) return false;

		pCurrentParticleBuffer->mIndex = index;
		m_pvPositions[1][index] = mVolumetricCloud->GetPos() + mVolumetricCloud->GetCellSize()* Vec3((float)i, (float)j, (float)k);

		//add to double buffer 2
		try
		{
			m_v_pCloudParticles[1].push_back(pCurrentParticleBuffer);
		}
		catch (...)
		{
			return false;
		}
		return true;
	}


	ParticleEnumerator::ParticleEnumerator(ParticlePool *pParticlePool)
	{
		mParticlePool = pParticlePool;
		mIndex = 0;
	}

	ParticleEnumerator::~ParticleEnumerator()
	{
	}


	CloudParticle* ParticleEnumerator::NextParticle()
	{
		//Update thread, get data from current buffer
		if (mIndex >= mParticlePool->m_uNumParticles)
			return NULL;
		else
			return mParticlePool->m_v_pCloudParticles[mParticlePool->m_iCurrentBuffer][mIndex++];
	}

	CloudParticle* ParticleEnumerator::NextParticleFromLastBuffer()
	{
		//Render thread, get data from last buffer
		if (mIndex >= mParticlePool->m_uNumParticles)
			return NULL;
		else
			return mParticlePool->m_v_pCloudParticles[1 - mParticlePool->m_iCurrentBuffer][mIndex++];
	}

	void ParticleEnumerator::Reset()
	{
		mIndex = 0;
	}

}