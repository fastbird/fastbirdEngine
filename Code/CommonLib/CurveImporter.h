#pragma once

namespace fastbird
{
	class Curve;
	class CurveImporter
	{
	public:
		CurveImporter();
		~CurveImporter();

		bool Import(Curve* curve, const char* filepath);
	};
}