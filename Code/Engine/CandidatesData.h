#pragma once

namespace fastbird
{

struct Candidate
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

class CandidatesData
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