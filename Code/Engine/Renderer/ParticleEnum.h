#pragma once

namespace fastbird
{
	namespace ParticleAlign
	{
		enum Enum
		{
			Billboard,
			Direction,
			NUM
		};

		static const char* strings[] = 
		{
			"Billboard",
			"Direction",
		};

		inline std::string ConvertToString(Enum a)
		{
			assert(a>=0 && a<NUM);
			return strings[a];
		}

		inline Enum ConverToEnum(const char* str)
		{
			if (_stricmp(str, "Billboard")==0)
				return Billboard;
			else if (_stricmp(str, "Direction")==0)
				return Direction;
			else
			{
				assert(0); 
				return NUM;
			}
		}
	}

	namespace ParticleEmitTo
	{
		enum Enum
		{
			LocalSpace,
			WorldSpace,
			NUM
		};

		static const char* strings[] = 
		{
			"LocalSpace",
			"WorldSpace",
		};

		inline std::string ConvertToString(Enum a)
		{
			assert(a>=0 && a<NUM);
			return strings[a];
		}

		inline Enum ConverToEnum(const char* str)
		{
			if (_stricmp(str, "LocalSpace")==0)
				return LocalSpace;
			else if (_stricmp(str, "WorldSpace")==0)
				return WorldSpace;
			else
			{
				assert(0); 
				return NUM;
			}
		}
	}

	namespace ParticleRangeType
	{
		enum Enum
		{
			Point,
			Sphere,
			Box,
			Cone,
			Hemisphere,
			NUM
		};

		static const char* strings[] = 
		{
			"Point",
			"Sphere",
			"Box",
			"Cone",
			"Hemisphere",
		};

		inline std::string ConvertToString(Enum a)
		{
			assert(a>=0 && a<NUM);
			return strings[a];
		}

		inline Enum ConverToEnum(const char* str)
		{
			if (_stricmp(str, "Point")==0)
				return Point;
			else if (_stricmp(str, "Sphere")==0)
				return Sphere;
			else if (_stricmp(str, "Box")==0)
				return Box;
			else if (_stricmp(str, "Cone")==0)
				return Cone;
			else if (_stricmp(str, "Hemisphere")==0)
				return Hemisphere;
			else
			{
				assert(0); 
				return NUM;
			}
		}
	}
}