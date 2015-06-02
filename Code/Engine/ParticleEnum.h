#pragma once

namespace fastbird
{
	namespace ParticleAlign
	{
		enum Enum
		{
			Billboard,
			Direction, // emitter direction
			ParticleDirection, 
			NUM
		};

		static const char* strings[] = 
		{
			"Billboard",
			"Direction",
			"ParticleDirection",
		};

		inline const char* ConvertToString(Enum a)
		{
			assert(a>=0 && a<NUM);
			return strings[a];
		}

		inline Enum ConvertToEnum(const char* str)
		{
			for (unsigned i = 0; i < NUM; ++i)
			{
				if (_stricmp(str, strings[i]) == 0)
					return Enum(i);
			}
			assert(0); 
			return NUM;
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

		inline const char* ConvertToString(Enum a)
		{
			assert(a>=0 && a<NUM);
			return strings[a];
		}

		inline Enum ConvertToEnum(const char* str)
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

		inline const char* ConvertToString(Enum a)
		{
			assert(a>=0 && a<NUM);
			return strings[a];
		}

		inline Enum ConvertToEnum(const char* str)
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

	namespace ParticleBlendMode
	{
		enum Enum{ Additive, AlphaBlend, InvColorBlend, Replace, NoBlend, NUM };
		static const char* strings[] ={ "Additive", "AlphaBlend", "InvColorBlend", "Replace", "NoBlend" };
		inline const char* ConvertToString(Enum a){
			assert(a >= 0 && a < NUM);
			return strings[a];
		}
		inline Enum ConvertToEnum(const char* str){
			if (_stricmp(str, "Additive") == 0)
				return Additive;
			if (_stricmp(str, "AlphaBlend") == 0)
				return AlphaBlend;
			if (_stricmp(str, "InvColorBlend") == 0)
				return InvColorBlend;
			if (_stricmp(str, "Replace") == 0)
				return Replace;
			if (_stricmp(str, "NoBlend") == 0)
				return NoBlend;
			else{
				assert(0);
				return NUM;
			}
		}

	}
}