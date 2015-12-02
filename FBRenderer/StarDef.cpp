/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "StarDef.h"

namespace fb
{
	Color StarDef::ms_avChromaticAberrationColor[];
	STARDEF s_aLibStarDef[NUM_BASESTARLIBTYPES] =
	{
		//  star name               lines   passes  length  attn    rotate          bRotate
		{ ("Disable"), 0, 0, 0.0f, 0.0f, Radian(00.0f), false, },  // STLT_DISABLE

		{ ("Cross"), 4, 3, 1.0f, 0.85f, Radian(0.0f), true, },  // STLT_CROSS
		{ ("CrossFilter"), 4, 3, 1.0f, 0.95f, Radian(0.0f), true, },  // STLT_CROSS
		{ ("snowCross"), 6, 3, 1.0f, 0.96f, Radian(20.0f), true, },  // STLT_SNOWCROSS
		{ ("Vertical"), 2, 3, 1.0f, 0.96f, Radian(00.0f), false, },  // STLT_VERTICAL
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
		Release();
	}

	//-----------------------------------------------------------------------
	bool StarDef::Construct()
	{
		memset(m_strStarName, 0, sizeof(m_strStarName));

		m_nStarLines = 0;
		m_pStarLine = NULL;
		m_fInclination = 0.0f;

		m_bRotation = false;

		return true;
	}

	void StarDef::Release()
	{
		FB_ARRAY_DELETE(m_pStarLine);
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
		m_pStarLine = FB_ARRAY_NEW(STARLINE, m_nStarLines);
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
		Real fSampleLength,
		Real fAttenuation,
		Real fInclination,
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
		m_pStarLine = FB_ARRAY_NEW(STARLINE, m_nStarLines);
		if (m_pStarLine == NULL)
			return false;

		Real fInc = Radian(360.0f / (Real)m_nStarLines);
		for (int i = 0; i < m_nStarLines; i++)
		{
			m_pStarLine[i].nPasses = nPasses;
			m_pStarLine[i].fSampleLength = fSampleLength;
			m_pStarLine[i].fAttenuation = fAttenuation;
			m_pStarLine[i].fInclination = fInc * (Real)i;
		}

		return true;
	}


	/// Specific start generation
	//  Sunny cross filter
	bool StarDef::Initialize_SunnyCrossFilter(const char* szStarName,
		Real fSampleLength,
		Real fAttenuation,
		Real fLongAttenuation,
		Real fInclination)
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
		m_pStarLine = FB_ARRAY_NEW(STARLINE, m_nStarLines);
		if (m_pStarLine == NULL)
			return false;

		Real fInc = Radian(360.0f / (Real)m_nStarLines);
		for (int i = 0; i < m_nStarLines; i++)
		{
			m_pStarLine[i].fSampleLength = fSampleLength;
			m_pStarLine[i].fInclination = fInc * (Real)i + Radian(0.0);

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

		memcpy(ms_avChromaticAberrationColor, avColor, sizeof(Color) * 8);
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
		FB_ARRAY_DELETE(s_pStarLib);

		return true;
	}

	StarDef& StarDef::GetLib(DWORD dwType)
	{
		return *s_pStarLib[dwType];
	}
}