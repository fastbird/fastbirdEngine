#include <CommonLib/StdAfx.h>
#include <CommonLib/Color.h>
#include <CommonLib/StringUtils.h>
namespace fastbird
{
const Color Color::White(1, 1, 1);
const Color Color::Black(0, 0, 0);
const Color Color::Red(1, 0, 0);
const Color Color::DarkGray(0.15f, 0.15f, 0.15f);
const Color Color::Gray(0.5f, 0.5f, 0.5f);
const Color Color::Silver(0.8f, 0.8f, 0.8f);
const Color Color::Green(0, 1, 0);
const Color Color::Yellow(1, 1, 0);
const Color Color::Blue(0, 0, 1);
const Color Color::SkyBlue(0, 0.5f, 1);
const Color Color::Zero(0, 0, 0, 0);

unsigned Color::FixColorByteOrder(unsigned c)
{
	RGBA color;
	color.r = c >> 24;
	color.g = c >> 16 & 0xff;
	color.b = c >> 8 & 0xff;
	color.a = c & 0xff;
	return *(unsigned*)&color;
}

Color::Color(const char* str)
{
	if (str[0] == '0' && str[1] == 'x')
	{	
		if (strlen(str) == 8)
		{
			std::string strColor = str;
			strColor += "ff";
			*this = Color(Color::FixColorByteOrder(StringConverter::parseHexa(strColor.c_str())));
		}
		else
		{
			*this = Color(Color::FixColorByteOrder(StringConverter::parseHexa(str)));
		}
	}
	else
	{
		mValue =  StringConverter::parseVec4(str);
	}
}
}
