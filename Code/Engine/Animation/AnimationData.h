#pragma once

namespace fastbird
{

	class AnimationData
	{
	public:
		enum PosComp
		{
			X,
			Y,
			Z
		};
		typedef PosComp ScaleComp;
		typedef PosComp RotComp;

		struct Action
		{
			Action()
			{
				mPosStartEnd[0] = mPosStartEnd[1] = 0;
				mRotStartEnd[0] = mRotStartEnd[1] = 0;
				mLength = 0.f;
				mStartTime = mEndTime = 0.f;
				mLoop = false;
			}
			std::string mName;
			float mStartTime;
			float mEndTime;
			float mLength;
			bool mLoop;

			const Vec3* mPosStartEnd[2];
			const Quat* mRotStartEnd[2];
		};
	private:

		friend class Animation;
		fastbird::VectorMap<float, Vec3> mScale;
		fastbird::VectorMap<float, Quat> mRot;
		fastbird::VectorMap<float, Vec3> mEuler;
		fastbird::VectorMap<float, Vec3> mPos;

		fastbird::VectorMap<std::string, Action> mActions;

		std::string mName;

	public:
		void AddPosition(float time, float v, PosComp comp);
		void AddScale(float time, float v, PosComp comp);
		void AddRotEuler(float time, float v, PosComp comp);
		bool HasPosAnimation() const;
		bool HasRotAnimation() const;
		bool HasScaleAnimation() const;
		void SetName(const char* name) { mName = name; }
		const char* GetName() const { return mName.c_str(); }
		void PickRot(float time, bool cycled, const Quat** prev, const Quat** next, float& interpol);

	private:
		friend class SpatialObject;
		void GenerateQuatFromEuler();
		bool ParseAction(const char* filename);

		const Vec3* FindPos(float time);
		const Vec3* FindScale(float time);
		const Quat* FindRot(float time);
		const Vec3* FindRotEuler(float time);
	};
}