#pragma once

namespace fastbird
{
	//---------------------------------------------------------------------------- -
	// Forward declarations
	class VolumetricCloud;
	class ParticlePool;
	class ParticleEnumerator;

	//---------------------------------------------------------------------------- -
	class CloudParticle
	{
	public:
		ParticlePool *mParticlePool;
		unsigned m_i, m_j, m_k;
		unsigned mIndex;
		Color m_cScatteringColor;
		bool mVisible;

	public:
		CloudParticle();
		CloudParticle(unsigned i, unsigned j, unsigned k, ParticlePool *pParticlePool);
		~CloudParticle();
		const Vec3& GetPosition();
		const Vec3& GetPositionFromLastBuffer();
		double		 GetViewDistance();
	};

	//---------------------------------------------------------------------------- -
	class ParticlePool
	{
		friend class ParticleEnumerator;
		friend class CloudParticle;
	public:
		ParticlePool();
		~ParticlePool();
		void SortbyViewDistances(const Vec3 &lookdir);
		void UpdateParticlePositions(double fTime);
		bool AddParticle(unsigned i, unsigned j, unsigned k);
		bool Setup(VolumetricCloud* pVolumetricCloud, unsigned uNumParticles);
		void Cleanup();

	protected:
		VolumetricCloud *mVolumetricCloud;
		std::vector< CloudParticle* >	m_v_pCloudParticles[2];
		unsigned			m_uNumParticles;
		Vec3		*m_pvPositions[2];
		double			*m_pfViewDistances;
		double          m_PreTime[2];

	public:
		Vec3     m_vWindVelocity;
		UINT            m_iCurrentBuffer;

	};

	//----------------------------------------------------------------------------
	class ParticleEnumerator
	{
	public:
		ParticleEnumerator(ParticlePool *pParticlePool);
		~ParticleEnumerator();
		CloudParticle* NextParticle();
		CloudParticle* NextParticleFromLastBuffer();
		void Reset();
	private:
		ParticlePool *mParticlePool;
		UINT mIndex;
	};
}