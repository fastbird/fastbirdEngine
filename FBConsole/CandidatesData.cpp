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
#include "CandidatesData.h"
#include "FBStringLib/StringLib.h"
using namespace fb;

CandidatesData::CandidatesData()
{
}

CandidatesData::~CandidatesData()
{
}

void CandidatesData::AddCandidate(const char* name)
{
	std::string candiName = name;
	ToLowerCase(candiName);
	for (const auto& c : mCandidates)
	{
		if (c.mName == candiName)
		{
			return;
		}
	}

	Candidate candi;
	candi.mName = name;
	mCandidates.push_back(candi);
	std::sort(mCandidates.begin(), mCandidates.end());
}

void CandidatesData::AddCandidatesTo(const char* parent, const StringVector& candidates)
{
	std::string parentName = parent;
	ToLowerCase(parentName);

	auto it = mCandidates.begin(), itEnd = mCandidates.end();
	for (; it != itEnd; it++)
	{
		if (it->mName == parentName)
		{
			it->AddCandidate(candidates);
			return;
		}
	}

	Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Cannot find the parent candidate(%s).", parent).c_str());
}

StringVector CandidatesData::GetCandidates(const char* input, int& outDepth)
{
	std::string strInput = input;
	ToLowerCase(strInput);
	// find parent
	std::string parent;
	size_t pos = strInput.find(' ');
	if (pos != std::string::npos)
		parent.assign(strInput.begin(), strInput.begin() + pos);

	// build list
	StringVector ret;
	CANDIDATES* candidates = 0;
	if (parent.empty())
	{
		outDepth = 0;
		candidates = &mCandidates;
	}
	else
	{
		// search 1 depth
		outDepth = 1;
		auto f = std::find(mCandidates.begin(), mCandidates.end(), parent);
		if (f != mCandidates.end())
		{
			candidates = &f->mChildren;
		}
		// second word
		pos = strInput.find_first_not_of(' ', pos);
		if (pos == std::string::npos)
		{
			strInput.clear();
		}
		else
		{
			strInput.assign(strInput.begin() + pos, strInput.end());
		}
	}

	if (candidates)
	{
		for (const auto& c : *candidates)
		{
			if (strInput.empty() // second words can be empty
				|| (c.mName.find(strInput) != std::string::npos && c.mName != strInput))
			{
				ret.push_back(c.mName);
			}
		}
	}

	return ret;
}


void Candidate::AddCandidate(const StringVector& candidates)
{
	for (auto newCandi : candidates)
	{
		ToLowerCase(newCandi);
		auto it = mChildren.begin();
		// check already registered?
		bool alreadyRegisterd = false;
		for (const auto& c : mChildren)
		{
			if (c.mName == newCandi)
			{
				alreadyRegisterd = true;
				break;
			}
		}

		if (!alreadyRegisterd)
		{
			Candidate addingCandi;
			addingCandi.mName = newCandi;
			mChildren.push_back(addingCandi);
		}
	}

	std::sort(mChildren.begin(), mChildren.end());
}