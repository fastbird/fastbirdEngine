#include <Engine/StdAfx.h>
#include <Engine/CloudSimulation.h>

// reference : https://software.intel.com/en-us/articles/dynamic-volumetric-cloud-rendering-for-games-on-multi-core-platforms
// Y. Dobashi, K. Kaneda, H. Yamashita, T. Okita, and T. Nishita. "A Simple, Efficient Method for Realistic Animation of Clouds".SIGGRAPH 2000, pp. 19-28
namespace fastbird
{
	//-----------------------------------------------------------------------
	CloudSimulation::CloudSimulation()
	{
		memset(this, 0, sizeof(CloudSimulation));
	}

	//-----------------------------------------------------------------------
	CloudSimulation::~CloudSimulation()
	{
		Cleanup();
	}
	//-----------------------------------------------------------------------
	bool CloudSimulation::Setup(unsigned length, unsigned width, unsigned height)
	{
		mLength = length;
		mWidth = width;
		mHeight = height;

		int size = length * width * height;
		mLastPhaseIndex = 0;
		mNextPhaseIndex = 1;
		mElapsedSteps = 0;

		bool success = NewByteSpace(size, &mHumSpace[0])
			&& NewByteSpace(size, &mHumSpace[1])
			&& NewByteSpace(size, &mCldSpace[0])
			&& NewByteSpace(size, &mCldSpace[1])
			&& NewByteSpace(size, &mActSpace[0])
			&& NewByteSpace(size, &mActSpace[1])
			&& NewByteSpace(size, &mActFactorSpace)
			&& NewFloatSpace(size, &mDensitySpace[0])
			&& NewFloatSpace(size, &mDensitySpace[1])

			&& NewByteSpace(size, &mShapeSpace)
			&& NewFloatSpace(size, &mHumProbSpace)
			&& NewFloatSpace(size, &mExtProbSpace)
			&& NewFloatSpace(size, &mActProbSpace)
			&& NewFloatSpace(size, &mMidDensitySpace);

		if (success)
		{
			for (int i = 0; i < size; ++i)
			{
				mCldSpace[0][i] = 1;
				mCldSpace[1][i] = 1;
			}
			mTotalNumCells = size;
			InitProbSpace();
			ShapeVolume();
			CelluarAutomate(mNextPhaseIndex);
			CalculateDensity(mNextPhaseIndex);
			return true;
		}
		Cleanup();
		return false;
	}
	//-----------------------------------------------------------------------
	void CloudSimulation::Cleanup()
	{
		FB_ARRDELETE(mShapeSpace);
		FB_ARRDELETE(mHumProbSpace);
		FB_ARRDELETE(mExtProbSpace);
		FB_ARRDELETE(mActProbSpace);
		FB_ARRDELETE(mHumSpace[0]);
		FB_ARRDELETE(mHumSpace[1]);
		FB_ARRDELETE(mCldSpace[0]);
		FB_ARRDELETE(mCldSpace[1]);
		FB_ARRDELETE(mActSpace[0]);
		FB_ARRDELETE(mActSpace[1]);
		FB_ARRDELETE(mActFactorSpace);
		FB_ARRDELETE(mDensitySpace[0]);
		FB_ARRDELETE(mDensitySpace[1]);
		FB_ARRDELETE(mMidDensitySpace);
	}
	//-----------------------------------------------------------------------
	// build the ellipsoid volume of cloud
	void CloudSimulation::ShapeVolume()
	{
		double cenX = mLength / 2.0;
		double cenY = mWidth / 2.0;
		double cenZ = mHeight / 2.0;
		double distance;

		mNumCellsInVolume = 0;
		float fProbSeed;
		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					distance = SQUARE(i - cenX) / SQUARE(cenX)
						+ SQUARE(j - cenY) / SQUARE(cenY)
						+ SQUARE(k - cenZ) / SQUARE(cenZ);
					if (distance < 1.0)
					{
						fProbSeed = (float)exp(-distance);
						InitCellInVolume(i, j, k, fProbSeed);
					}
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	void CloudSimulation::InitProbSpace()
	{
		int fHigh = mHeight - 1;
		int lm = mLength / 2;
		int wm = mWidth / 2;
		int hm = mHeight / 2;

		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					mExtProbSpace[CLOUD_INDEX(i, j, k)] = (float)exp(-(fHigh - k));
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	void CloudSimulation::GrowCloud(unsigned phaseIndex)
	{
		int index;

		UpdateActFactorSpace(!phaseIndex);

		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					if (!IsCellInVolume(i, j, k, &index)) 
						continue;
					mActSpace[phaseIndex][index] = (!mActSpace[!phaseIndex][index]) && (mHumSpace[!phaseIndex][index]) && (mActFactorSpace[index]);
					mHumSpace[phaseIndex][index] = mHumSpace[!phaseIndex][index] && (!mActSpace[!phaseIndex][index]);
					mCldSpace[phaseIndex][index] = mCldSpace[!phaseIndex][index] || mActSpace[!phaseIndex][index];
					/*Log("ActSpace = %u", mActSpace[phaseIndex][index]);
					Log("HumSpace = %u", mActSpace[phaseIndex][index]);
					Log("CldSpace = %u", mActSpace[phaseIndex][index]);*/
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	void CloudSimulation::ExtinctCloud(unsigned phaseIndex)
	{
		int index;

		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					if (!IsCellInVolume(i, j, k, &index)) 
						continue;
					mCldSpace[phaseIndex][index] = mCldSpace[phaseIndex][index] && (Random(0.f, 1.f) > mExtProbSpace[index]);
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	void CloudSimulation::SupplyVapor(unsigned phaseIndex)
	{
		int index;

		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					if (!IsCellInVolume(i, j, k, &index)) 
						continue;
					mHumSpace[phaseIndex][index] = mHumSpace[phaseIndex][index] || (Random(0.f, 1.f) < mHumProbSpace[index]);
					mActSpace[phaseIndex][index] = mHumSpace[phaseIndex][index] && (mActSpace[phaseIndex][index] || (Random(0.f, 1.f) < mActProbSpace[index]));

				}
			}
		}
	}

	//-----------------------------------------------------------------------
	// calculate the current density space by interpolating the density spaces of the last phase and the next phase
	void  CloudSimulation::InterpolateDensitySpace(float alpha)
	{
		if (alpha <= 0.0) // alpha is the interpolation factor
		{
			mCurrentDensitySpace = mDensitySpace[mLastPhaseIndex];
		}
		else if (alpha >= 1.0)
		{
			// exchange the indexes of the last phase and the next phase;
			mLastPhaseIndex = mNextPhaseIndex;
			mNextPhaseIndex = !mLastPhaseIndex;

			// point current DensitySpace to the DensitySpace of last phase
			mCurrentDensitySpace = mDensitySpace[mLastPhaseIndex];

			// Create the next DensitySpace of the next phase
			CelluarAutomate(mNextPhaseIndex);
			CalculateDensity(mNextPhaseIndex);
		}
		else
		{
			int index;

			for (int i = 0; i < mLength; i++)
			{
				for (int j = 0; j < mWidth; j++)
				{
					for (int k = 0; k < mHeight; k++)
					{
						if (!IsCellInVolume(i, j, k, &index)) 
							continue;
						mMidDensitySpace[index] = (float)((1.0 - alpha) * mDensitySpace[mLastPhaseIndex][index]
							+ alpha * mDensitySpace[mNextPhaseIndex][index]);
					}
				}
			}

			mCurrentDensitySpace = mMidDensitySpace;
		}
	}
	//-----------------------------------------------------------------------
	float CloudSimulation::GetCellDensity(int i, int j, int k)
	{
		int index = CLOUD_INDEX(i, j, k);
		if ((index < 0) || (index >= (int)mTotalNumCells)) return 0;
		return mCurrentDensitySpace[index];
	}
	//-----------------------------------------------------------------------
	float CloudSimulation::GetPointDensity(const Vec3& point)
	{
		int i = (int)point.x;
		int j = (int)point.y;
		int k = (int)point.z;
		float r = point.x - i;
		float s = point.y - j;
		float uPhaseIndex = point.z - k;

		// get the densities of 8 points around the point.
		float d0 = GetCellDensity(i, j, k);
		float d1 = GetCellDensity(i, j, k + 1);
		float d2 = GetCellDensity(i, j + 1, k);
		float d3 = GetCellDensity(i, j + 1, k + 1);
		float d4 = GetCellDensity(i + 1, j, k);
		float d5 = GetCellDensity(i + 1, j, k + 1);
		float d6 = GetCellDensity(i + 1, j + 1, k);
		float d7 = GetCellDensity(i + 1, j + 1, k + 1);

		// interpolate densities
		float z01 = (d1 - d0)*uPhaseIndex + d0;
		float z23 = (d3 - d2)*uPhaseIndex + d2;
		float z45 = (d5 - d4)*uPhaseIndex + d4;
		float z67 = (d7 - d6)*uPhaseIndex + d6;
		float x0145 = (z45 - z01)*r + z01;
		float x2367 = (z67 - z23)*r + z23;
		float result = ((x2367 - x0145)*s + x0145);
		return result;
	}

	//-----------------------------------------------------------------------
	bool CloudSimulation::IsCellInVolume(int i, int j, int k, int* pIndex/* = 0*/)
	{
		if (!IsCellInSpace(i, j, k))
			return false;
		else
		{
			int index = CLOUD_INDEX(i, j, k);
			if (mShapeSpace[index])
			{
				if (pIndex)
					*pIndex = index;
				return true;
			}
			else
				return false;
		}
	}
	//-----------------------------------------------------------------------
	bool CloudSimulation::IsCellInSpace(int i, int j, int k)
	{
		if ((i < 0) || (i >= mLength)
			|| (j < 0) || (j >= mWidth)
			|| (k < 0) || (k >= mHeight))
			return false;
		else
			return true;
	}
	//-----------------------------------------------------------------------
	// set the probablities of every cell.
	void CloudSimulation::InitCellInVolume(int i, int j, int k, float probSeed)
	{
		if (!IsCellInSpace(i, j, k)) 
			return;

		if (mNumCellsInVolume >= 2500)
		{
			Log("Generating too many clouds vertices! Ignoring some.");
			return;
		}
			
		int index = CLOUD_INDEX(i, j, k);
		if (mShapeSpace[index] == false)
		{
			mShapeSpace[index] = true;
			mNumCellsInVolume++;
		}
		float fCurExtProb = (float)0.2 * (1 - probSeed);
		float fCurHumProb = (float)0.05 * probSeed;
		float fCurActProb = (float)0.001 * probSeed;

		mExtProbSpace[index] *= fCurExtProb;
		mHumProbSpace[index] = mHumProbSpace[index] * (1 - fCurHumProb) + fCurHumProb;
		mActProbSpace[index] = mActProbSpace[index] * (1 - fCurActProb) + fCurActProb;
	}
	//-----------------------------------------------------------------------
	bool CloudSimulation::IsPointInSpace(const Vec3& point)
	{
		if ((point.x < 0) || (point.x >= mLength)
			|| (point.y < 0) || (point.y >= mWidth)
			|| (point.z < 0) || (point.z >= mHeight))
			return false;
		else
			return true;
	}

	//-----------------------------------------------------------------------
	void CloudSimulation::SetByteCell(BYTE* pByteSpace, int i, int j, int k, BYTE value)
	{
		if (!IsCellInSpace(i, j, k)) return;
		pByteSpace[CLOUD_INDEX(i, j, k)] = value;
	}
	//-----------------------------------------------------------------------
	BYTE CloudSimulation::GetByteCell(BYTE* pByteSpace, int i, int j, int k)
	{
		if (!IsCellInSpace(i, j, k)) return 0;
		return pByteSpace[CLOUD_INDEX(i, j, k)];
	}
	//-----------------------------------------------------------------------
	bool CloudSimulation::NewByteSpace(int size, BYTE** ppByteSpace)
	{
		*ppByteSpace = FB_ARRNEW(BYTE, size);
		if (*ppByteSpace)
		{
			memset(*ppByteSpace, 0, size*sizeof(BYTE));
			return true;
		}
		else
			return false;
	}
	//-----------------------------------------------------------------------
	bool CloudSimulation::NewFloatSpace(int size, float** ppFloatSpace)
	{
		*ppFloatSpace = FB_ARRNEW(float, size);
		if (*ppFloatSpace)
		{
			memset(*ppFloatSpace, 0, size*sizeof(float));
			return true;
		}
		else
			return false;
	}
	//-----------------------------------------------------------------------
	void CloudSimulation::UpdateActFactorSpace(unsigned phaseIndex)
	{
		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					if (!IsCellInVolume(i, j, k)) continue;
					BYTE FAct = GetByteCell(mActSpace[phaseIndex], i + 1, j, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j + 1, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j, k + 1)
						|| GetByteCell(mActSpace[phaseIndex], i - 1, j, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j - 1, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j, k - 1)
						|| GetByteCell(mActSpace[phaseIndex], i - 2, j, k)
						|| GetByteCell(mActSpace[phaseIndex], i + 2, j, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j - 2, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j + 2, k)
						|| GetByteCell(mActSpace[phaseIndex], i, j, k - 2);

					SetByteCell(mActFactorSpace, i, j, k, FAct);
					//Log("ActFactor = %u", FAct);
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	void CloudSimulation::CelluarAutomate(unsigned uPhaseIndex)
	{
		// update the simulation space.
		ExtinctCloud(!uPhaseIndex);

		GrowCloud(uPhaseIndex);

		if (mElapsedSteps == 0)
		{
			SupplyVapor(uPhaseIndex);
			mElapsedSteps = CLOUD_SUPPLY_INTERVAL;
		}
		else
			mElapsedSteps--;
	}
	//-----------------------------------------------------------------------
	// Calculate every cell's density in the simluation space.
	// The density distribution of clouds in the real world is continuous from 0 to 1. 
	// The distribution obtained from the simulation,however, has only two values, that is, 0 or 1. 
	// Therefore, the function calculates continuous distribution by smoothing the binary distribution, or two-valued distribution of CldSpace
	void CloudSimulation::CalculateDensity(unsigned uPhaseIndex)
	{
		int	index;
		for (int i = 0; i < mLength; i++)
		{
			for (int j = 0; j < mWidth; j++)
			{
				for (int k = 0; k < mHeight; k++)
				{
					if (!IsCellInVolume(i, j, k, &index)) 
						continue;

					mDensitySpace[uPhaseIndex][index] = 0;

					// accumulate the binary cloud values of the cells surrouding the current cell(i,j,k) and itself.
					for (int p = i - 1; p <= i + 1; p++)
					for (int q = j - 1; q <= j + 1; q++)
					for (int r = k - 1; r <= k + 1; r++)
						mDensitySpace[uPhaseIndex][index] += GetByteCell(mCldSpace[uPhaseIndex], p, q, r);

					mDensitySpace[uPhaseIndex][index] /= 27.0f; // 27 is the number of all iterations of above nesting loop.
					//Log("Density = %f", mDensitySpace[uPhaseIndex][index]);
				}
			}
		}
	}


}