#include <Engine/StdAfx.h>
#include <Engine/StarDef.h>

namespace fastbird
{	
	Color StarDef::ms_avChromaticAberrationColor[];
	STARDEF s_aLibStarDef[NUM_BASESTARLIBTYPES] =
	{
		//  star name               lines   passes  length  attn    rotate          bRotate
		{ TEXT("Disable"), 0, 0, 0.0f, 0.0f, Radian(00.0f), false, },  // STLT_DISABLE

		{ TEXT("Cross"), 4, 3, 1.0f, 0.85f, Radian(0.0f), true, },  // STLT_CROSS
		{ TEXT("CrossFilter"), 4, 3, 1.0f, 0.95f, Radian(0.0f), true, },  // STLT_CROSS
		{ TEXT("snowCross"), 6, 3, 1.0f, 0.96f, Radian(20.0f), true, },  // STLT_SNOWCROSS
		{ TEXT("Vertical"), 2, 3, 1.0f, 0.96f, Radian(00.0f), false, },  // STLT_VERTICAL
	};
	int                      s_nLibStarDefs = sizeof(s_aLibStarDef) / sizeof(STARDEF);

	StarDef* StarDef::s_pStarLib[NUM_STARLIBTYPES] = { 0 };

	//-----------------------------------------------------------------------
	void StarDef::InitializeStatic()
	{
		for (int i = 0; i < NUM_BASESTARLIBTYPES; ++i)
		{
			s_pStarLib[i] = FB_NEW(StarDef);
			s_pStarLib[i]->Initialize(s_aLibStarDef[i]);
		}
		s_pStarLib[STLT_SUNNYCROSS] = FB_NEW(StarDef);
		s_pStarLib[STLT_SUNNYCROSS]->Initialize_SunnyCrossFilter();
	}

	//-----------------------------------------------------------------------
	void StarDef::FinalizeStatic()
	{
		for (int i = 0; i < NUM_BASESTARLIBTYPES; ++i)
			FB_DELETE(s_pStarLib[i]);

		FB_DELETE(s_pStarLib[STLT_SUNNYCROSS]);
	}
	
	//-----------------------------------------------------------------------
	StarDef::StarDef()
	{
		Construct();
	}

	//-----------------------------------------------------------------------
	StarDef::StarDef(const StarDef& src)
	{
		Construct();
		Initialize(src);
	}

	//-----------------------------------------------------------------------
	StarDef::~StarDef()
	{
		Destruct();
	}

	//-----------------------------------------------------------------------
	bool StarDef::Construct()
	{
		ZeroMemory(m_strStarName, sizeof(m_strStarName));

		m_nStarLines = 0;
		m_pStarLine = NULL;
		m_fInclination = 0.0f;

		m_bRotation = false;

		return true;
	}

	void StarDef::Destruct()
	{
		Release();
	}

	void StarDef::Release()
	{
		FB_ARRDELETE(m_pStarLine);
		m_nStarLines = 0;
	}


	bool StarDef::Initialize(const StarDef& src)
	{
		if (&src == this)
		{
			return true;
		}

		// Release the data
		Release();

		// Copy the data from source
		strcpy_s(m_strStarName, 256, src.m_strStarName);
		m_nStarLines = src.m_nStarLines;
		m_fInclination = src.m_fInclination;
		m_bRotation = src.m_bRotation;

		assert(!m_pStarLine);
		m_pStarLine = FB_ARRNEW(STARLINE, m_nStarLines);
		if (m_pStarLine == NULL)
			return false;

		for (int i = 0; i < m_nStarLines; i++)
		{
			m_pStarLine[i] = src.m_pStarLine[i];
		}

		return true;
	}

	bool StarDef::Initialize(ESTARLIBTYPE eType)
	{

		return Initialize(*s_pStarLib[eType]);
	}

	bool StarDef::Initialize(const STARDEF& starDef)
	{
		return Initialize(starDef.szStarName,
			starDef.nStarLines,
			starDef.nPasses,
			starDef.fSampleLength,
			starDef.fAttenuation,
			starDef.fInclination,
			starDef.bRotation);
	}


	/// generic simple star generation
	bool StarDef::Initialize(const char* szStarName,
		int nStarLines,
		int nPasses,
		float fSampleLength,
		float fAttenuation,
		float fInclination,
		bool bRotation)
	{
		// Release the data
		Release();

		// Copy from parameters
		strcpy_s(m_strStarName, 256, szStarName);
		m_nStarLines = nStarLines;
		m_fInclination = fInclination;
		m_bRotation = bRotation;

		assert(!m_pStarLine);
		m_pStarLine = FB_ARRNEW(STARLINE, m_nStarLines);
		if (m_pStarLine == NULL)
			return false;

		float fInc = Radian(360.0f / (float)m_nStarLines);
		for (int i = 0; i < m_nStarLines; i++)
		{
			m_pStarLine[i].nPasses = nPasses;
			m_pStarLine[i].fSampleLength = fSampleLength;
			m_pStarLine[i].fAttenuation = fAttenuation;
			m_pStarLine[i].fInclination = fInc * (float)i;
		}

		return true;
	}


	/// Specific start generation
	//  Sunny cross filter
	bool StarDef::Initialize_SunnyCrossFilter(const char* szStarName,
		float fSampleLength,
		float fAttenuation,
		float fLongAttenuation,
		float fInclination)
	{
		// Release the data
		Release();

		// Create parameters
		strcpy_s(m_strStarName, 256, szStarName);
		m_nStarLines = 8;
		m_fInclination = fInclination;
		//  m_bRotation     = true ;
		m_bRotation = false;
		assert(!m_pStarLine);
		m_pStarLine = FB_ARRNEW(STARLINE, m_nStarLines);
		if (m_pStarLine == NULL)
			return false;

		float fInc = Radian(360.0f / (float)m_nStarLines);
		for (int i = 0; i < m_nStarLines; i++)
		{
			m_pStarLine[i].fSampleLength = fSampleLength;
			m_pStarLine[i].fInclination = fInc * (float)i + Radian(0.0f);

			if (0 == (i % 2))
			{
				m_pStarLine[i].nPasses = 3;
				m_pStarLine[i].fAttenuation = fLongAttenuation;    // long
			}
			else
			{
				m_pStarLine[i].nPasses = 3;
				m_pStarLine[i].fAttenuation = fAttenuation;
			}
		}

		return true;
	}

	bool StarDef::InitializeStaticStarLibs()
	{
		if (s_pStarLib)
		{
			return true;
		}

		// Create basic form
		for (int i = 0; i < NUM_BASESTARLIBTYPES; i++)
		{
			s_pStarLib[i]->Initialize(s_aLibStarDef[i]);
		}

		// Create special form
		// Sunny cross filter
		s_pStarLib[STLT_SUNNYCROSS]->Initialize_SunnyCrossFilter();

		// Initialize color aberration table
		/*
		{
		D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),
		D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),
		D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),
		D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),
		} ;
		*/
		Color avColor[8] =
		{
			/*
			D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),
			D3DXCOLOR(0.3f, 0.3f, 0.8f,  0.0f),
			D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),
			D3DXCOLOR(0.2f, 0.4f, 0.5f,  0.0f),
			D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),
			D3DXCOLOR(0.5f, 0.4f, 0.2f,  0.0f),
			D3DXCOLOR(0.7f, 0.3f, 0.2f,  0.0f),
			D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),
			*/

			Color(0.5f, 0.5f, 0.5f, 0.0f), // w
			Color(0.8f, 0.3f, 0.3f, 0.0f),
			Color(1.0f, 0.2f, 0.2f, 0.0f), // r
			Color(0.5f, 0.2f, 0.6f, 0.0f),
			Color(0.2f, 0.2f, 1.0f, 0.0f), // b
			Color(0.2f, 0.3f, 0.7f, 0.0f),
			Color(0.2f, 0.6f, 0.2f, 0.0f), // g
			Color(0.3f, 0.5f, 0.3f, 0.0f),
		};

		memcpy(ms_avChromaticAberrationColor, avColor, sizeof(Color)* 8);
		/*
		ms_avChromaticAberrationColor[0] = D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f) ;
		ms_avChromaticAberrationColor[1] = D3DXCOLOR(0.7f, 0.3f, 0.3f,  0.0f) ;
		ms_avChromaticAberrationColor[2] = D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f) ;
		ms_avChromaticAberrationColor[3] = D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f) ;
		ms_avChromaticAberrationColor[4] = D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f) ;
		ms_avChromaticAberrationColor[5] = D3DXCOLOR(0.2f, 0.4f, 0.5f,  0.0f) ;
		ms_avChromaticAberrationColor[6] = D3DXCOLOR(0.2f, 0.3f, 0.8f,  0.0f) ;
		ms_avChromaticAberrationColor[7] = D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f) ;
		*/
		return true;
	}

	bool StarDef::DeleteStaticStarLibs()
	{
		// Delete all libraries
		FB_ARRDELETE(s_pStarLib);

		return true;
	}

	StarDef& StarDef::GetLib(DWORD dwType)
	{
		return *s_pStarLib[dwType];
	}
}