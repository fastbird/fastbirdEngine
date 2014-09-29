#pragma once

namespace fastbird
{
	//-----------------------------------------------------------------------
	typedef struct STARDEF
	{
		char* szStarName;
		int nStarLines;
		int nPasses;
		float fSampleLength;
		float fAttenuation;
		float fInclination;
		bool bRotation;

	}*LPSTARDEF;

	//-----------------------------------------------------------------------
	enum ESTARLIBTYPE
	{
		STLT_DISABLE = 0,
		STLT_CROSS,
		STLT_CROSSFILTER,
		STLT_SNOWCROSS,
		STLT_VERTICAL,

		NUM_BASESTARLIBTYPES,

		STLT_SUNNYCROSS = NUM_BASESTARLIBTYPES,

		NUM_STARLIBTYPES,
	};

	//-----------------------------------------------------------------------
	typedef struct STARLINE
	{
		int nPasses;
		float fSampleLength;
		float fAttenuation;
		float fInclination;

	};

	//-----------------------------------------------------------------------
	class StarDef
	{
	public:
		static StarDef* s_pStarLib[NUM_STARLIBTYPES];
		static void InitializeStatic();
		static void FinalizeStatic();
		char               m_strStarName[256];

		int m_nStarLines;
		STARLINE* m_pStarLine;   // [m_nStarLines]
		float m_fInclination;
		bool m_bRotation;   // Rotation is available from outside ?

		// Static library
	public:
		static Color ms_avChromaticAberrationColor[8];

		// Public method
	public:
		StarDef();
		StarDef(const StarDef& src);
		~StarDef();

		StarDef& operator =(const StarDef& src)
		{
			Initialize(src);
			return *this;
		}

		bool	            Construct();
		void                Destruct();
		void                Release();

		bool             Initialize(const StarDef& src);

		bool             Initialize(ESTARLIBTYPE eType);


		/// Generic simple star generation
		bool             Initialize(const char* szStarName,
			int nStarLines,
			int nPasses,
			float fSampleLength,
			float fAttenuation,
			float fInclination,
			bool bRotation);

		bool             Initialize(const STARDEF& starDef);
		

		/// Specific star generation
		//  Sunny cross filter
		bool             Initialize_SunnyCrossFilter(const char* szStarName = "SunnyCross",
			float fSampleLength = 1.0f,
			float fAttenuation = 0.88f,
			float fLongAttenuation = 0.95f,
			float fInclination = 0.0f);


		// Public static method
	public:
		/// Create star library
		static bool      InitializeStaticStarLibs();
		static bool      DeleteStaticStarLibs();

		/// Access to the star library
		static StarDef& GetLib(DWORD dwType);
		static const Color& GetChromaticAberrationColor(DWORD dwID)
		{
			return ms_avChromaticAberrationColor[dwID];
		}
	};
}