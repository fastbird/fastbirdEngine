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

		bool operator==(const ColorRamp& other) const;

	private:
		typedef std::vector<Bar> BAR_VECTOR;
		BAR_VECTOR mBars;
		std::vector<Color> mColors;
		bool mGenerated;
	};
}