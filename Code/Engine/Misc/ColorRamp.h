#pragma once
#include <CommonLib/Color.h>

namespace fastbird
{
	class ColorRamp
	{
	public:

		struct Bar
		{
			Bar(float pos, const Color& color)
			{
				position = pos;
				this->color = color;
			}
			float position;
			Color color;

			bool operator < (const Bar& other) const
			{
				return position < other.position;
			}

			bool operator== (const Bar& other) const
			{
				return (position == other.position && color == other.color);
			}

		};

		ColorRamp();
		~ColorRamp();

		void InsertBar(float pos, const Color& color);
		unsigned NumBars() const { return mBars.size(); }
		Bar& GetBar(int idx) { assert((unsigned)idx < mBars.size()); return mBars[idx]; }
		void GenerateColorRampTextureData(int textureWidth);
		const Color& operator[] (int idx) const;
		void UpperAlign(float gap = 0.01f)
		{
			for (size_t i = 1; i < mBars.size(); ++i)
			{
				if (mBars[i].position - mBars[i - 1].position< gap)
				{
					mBars[i].position = mBars[i - 1].position + gap;
					mBars[i].position = std::min(mBars[i].position, 1.0f);
				}
			}
		}
		void LowerAlign(float gap = 0.01f)
		{
			for (int i = (int)mBars.size() - 2; i >= 0; --i)
			{
				if (mBars[i + 1].position - mBars[i].position < gap)
				{
					mBars[i].position = mBars[i + 1].position - gap;
					mBars[i].position = std::max(0.f, mBars[i].position);
				}
			}
		}

		bool operator==(const ColorRamp& other) const;

	private:
		typedef std::vector<Bar> BAR_VECTOR;
		BAR_VECTOR mBars;
		std::vector<Color> mColors;
		bool mGenerated;
	};
}