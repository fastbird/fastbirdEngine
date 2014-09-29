#include <CommonLib/StdAfx.h>
#include <CommonLib/CurveImporter.h>
#include <CommonLib/Curve.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{
	CurveImporter::CurveImporter()
	{}
	CurveImporter::~CurveImporter()
	{}

	bool CurveImporter::Import(Curve* curve, const char* filepath)
	{
		assert(curve);
		std::ifstream file(filepath);
		if (!file.is_open())
			return false;

		const unsigned BUFFER_SIZE = 512;
		char line[BUFFER_SIZE];
		bool start = false;;
		while (!file.eof())
		{
			memset(line, 0, BUFFER_SIZE);
			file.getline(line, BUFFER_SIZE);

			if (strlen(line) == 0)
				continue;
			if (strcmp(line, "o BezierCurve") == 0)
			{
				start = true;
				continue;
			}
			if (start)
			{
				if (line[0] != 'v')
				{
					return curve->GetNumPoints()!=0;
				}
				curve->AddPoints(StringConverter::parseVec3(line + 2));
			}
		}
		return curve->GetNumPoints() != 0;
	}
}