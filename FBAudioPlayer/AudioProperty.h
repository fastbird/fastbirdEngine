#pragma once
namespace fb{
	struct AudioProperty{
		AudioProperty()
			: mPosition{ 0.f, 0.f, 0.f }
			, mReferenceDistance(10.f)
			, mRolloffFactor(1.f)
			, mRelative(false)
			, mGain(1.f)
		{
		}
		Vec3Tuple mPosition;
		/// The distance under which the volume for the audio
		/// would normally drop by half.
		float mReferenceDistance;
		/// higher value will decrease the volume for the audio
		/// faster.
		float mRolloffFactor;
		float mGain;
		bool mRelative;
	};
}