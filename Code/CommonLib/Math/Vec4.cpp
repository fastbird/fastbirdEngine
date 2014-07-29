#include <CommonLib/StdAfx.h>
#include "Vec4.h"
#include "../StringUtils.h"

namespace fastbird
{

const Vec4 Vec4::UNIT_X(1.f, 0, 0, 0);
const Vec4 Vec4::UNIT_Y(0, 1.f, 0, 0);
const Vec4 Vec4::UNIT_Z(0, 0, 1.f, 0);
const Vec4 Vec4::ZERO(0, 0, 0, 0);

Vec4::Vec4(const char* s)
{
	char* next;
	x = 0;
	y = 0;
	z = 0;
	w = 1.0f;
	if (s!=0)
	{
		x = (float)strtod(s, &next);
		if (next!=0)
		{
			StepToDigit(&next);
			y = (float)strtod(next, &next);
			if (next!=0)
			{
				StepToDigit(&next);
				z = (float)strtod(next, &next);
				if (next!=0)
				{
					StepToDigit(&next);
					if (next!=0)
						w = (float)strtod(next, 0);
				}

			}
		}
	}			
}

}