#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(ParticleTest);
	class ParticleTest{
		FB_DECLARE_PIMPL_NON_COPYABLE(ParticleTest);
		ParticleTest();
		~ParticleTest();

	public:
		static ParticleTestPtr Create();
		void Update(TIME_PRECISION dt);
	};
}