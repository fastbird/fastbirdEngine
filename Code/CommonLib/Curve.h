#pragma once

namespace fastbird
{
	class Curve
	{
	public:
		Curve();
		~Curve();

		size_t GetNumPoints() const;
		const fastbird::Vec3 GetPoint(size_t index, float scale) const;

		void AddPoints(const fastbird::Vec3& p);

	private:
		std::vector<fastbird::Vec3> mPoints;
	};
}