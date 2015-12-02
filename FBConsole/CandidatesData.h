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
#include <string>
#include "FBCommonHeaders/Types.h"
namespace fb
{
	struct FB_DLL_CONSOLE Candidate
	{
		bool operator< (const Candidate& other) const
		{
			return mName < other.mName;
		}

		bool operator== (const std::string& otherName) const
		{
			return mName == otherName;
		}

		void AddCandidate(const StringVector& candidates);

		std::string mName;
		std::vector<Candidate> mChildren;
	};

	typedef std::vector<Candidate> CANDIDATES;

	class FB_DLL_CONSOLE CandidatesData
	{
	public:
		CandidatesData();
		~CandidatesData();

		void AddCandidate(const char* name);
		void AddCandidatesTo(const char* parent, const StringVector& candidates);

		StringVector GetCandidates(const char* input, int& outDepth);

	private:
		CANDIDATES mCandidates;

	};

}