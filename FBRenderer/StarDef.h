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

#pragma once
#include "FBMathLib/Color.h"
namespace fb
{
	//-----------------------------------------------------------------------
	typedef struct STARDEF
	{
		char* szStarName;
		int nStarLines;
		int nPasses;
		Real fSampleLength;
		Real fAttenuation;
		Real fInclination;
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
	struct STARLINE
	{
		int nPasses;
		Real fSampleLength;
		Real fAttenuation;
		Real fInclination;
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
		Real m_fInclination;
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
		void                Release();

		bool             Initialize(const StarDef& src);

		bool             Initialize(ESTARLIBTYPE eType);


		/// Generic simple star generation
		bool             Initialize(const char* szStarName,
			int nStarLines,
			int nPasses,
			Real fSampleLength,
			Real fAttenuation,
			Real fInclination,
			bool bRotation);

		bool             Initialize(const STARDEF& starDef);


		/// Specific star generation
		//  Sunny cross filter
		bool             Initialize_SunnyCrossFilter(const char* szStarName = "SunnyCross",
			Real fSampleLength = 1.0f,
			Real fAttenuation = 0.88f,
			Real fLongAttenuation = 0.95f,
			Real fInclination = 0.0f);


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