#include <Engine/StdAfx.h>
#include <Engine/Misc/CandidatesData.h>

namespace fastbird
{

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
	for each(const Candidate& c in mCandidates)
	{
		if (c.mName==candiName)
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
	for (; it!= itEnd; it++)
	{
		if (it->mName == parentName)
		{
			it->AddCandidate(candidates);
			return;
		}
	}

	Log("Cannot find the parent candidate(%s).", parent);
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
		for each(const Candidate& c in *candidates)
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
	for each(std::string newCandi in candidates)
	{
		ToLowerCase(newCandi);
		auto it = mChildren.begin();
		// check already registered?
		bool alreadyRegisterd = false;
		for each(const Candidate& c in mChildren)
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

}