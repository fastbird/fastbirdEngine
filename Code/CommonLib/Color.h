#pragma once
#include <CommonLib/Math/Vec4.h>
#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	class Color
	{
	public:
		const static Color White;
		const static Color Black;
		const static Color Red;
		const static Color DarkGray;
		const static Color Gray;
		const static Color Silver;
		const static Color Green;
		const static Color Yellow;
		const static Color Blue;
		const static Color SkyBlue;
		const static Color Zero;

		struct RGBA
		{
			BYTE r;
			BYTE g;
			BYTE b;
			BYTE a;
		};

		Color(){}
		Color(const Vec3& color)
			: mValue(color, 1.0f)
		{
		}
		Color(const Vec4& color)
			: mValue(color)
		{
		}
		Color(float r, float g, float b, float a)
			: mValue(r, g, b, a)
		{
		}

		Color(float r, float g, float b)
			: mValue(r, g, b, 1.f)
		{
		}

		Color(unsigned int c)
		{
			RGBA* rgba = (RGBA*)&c;
			mValue.x = rgba->r / 255.0f;
			mValue.y = rgba->g / 255.0f;
			mValue.z = rgba->b / 255.0f;
			mValue.w = rgba->a / 255.0f;
		}

		unsigned int Get4Byte() const
		{
			RGBA color;
			color.r = BYTE(mValue.x * 255.f);
			color.g = BYTE(mValue.y * 255.f);
			color.b = BYTE(mValue.z * 255.f);
			color.a = BYTE(mValue.w * 255.f);
			return *(unsigned int*)&color;
		}

		inline operator unsigned int() const { return Get4Byte(); }

		const Vec4& GetVec4() const { return mValue; }

		inline Color operator* (float scalar) const
		{
			return Color(mValue*scalar);
		}

		inline Color operator*(const Color& other) const
		{
			return Color(mValue*other.mValue);
		}

		inline Color& operator*= (float scalar)
		{
			mValue *= scalar;
			return *this;
		}

		inline Color& operator*= (const Color& c)
		{
			mValue *= c.mValue;
			return *this;
		}

		inline Color operator+ (const Color& r) const
		{
			return mValue + r.GetVec4();
		}

		bool operator== (const Color& other) const
		{
			return mValue==other.mValue;
		}

		void SetColor(float r, float g, float b, float a=1.f)
		{
			mValue.x = r;
			mValue.y = g;
			mValue.z = b;
			mValue.w = a;
		}

		float r() const {return mValue.x;}
		float g() const {return mValue.y;}
		float b() const {return mValue.z;}
		float a() const {return mValue.w;}

		float& r(){ return mValue.x; }
		float& g(){ return mValue.y; }
		float& b(){ return mValue.z; }
		float& a(){ return mValue.w; }

	private:
		Vec4 mValue;
	};
}