#pragma once

#define CLOUD_SUPPLY_INTERVAL 5	//the number of frames passed supply vapor 
#define CLOUD_EXTINCT_FACTOR 0.1
#define CLOUD_INDEX(i,j,k)	((i)*mHeight*mWidth+(j)*mHeight+(k))
#define SQUARE(x) ((x) * (x))
namespace fastbird
{
	class CloudSimulation
	{
	private:
		int mLength, mWidth, mHeight;
		unsigned mNumCellsInVolume; // only filled cell
		unsigned mTotalNumCells;

		BYTE* mShapeSpace;
		float* mHumProbSpace; // humidity
		float* mExtProbSpace; // extinct
		float* mActProbSpace; // activation

		unsigned mLastPhaseIndex;
		unsigned mNextPhaseIndex;

		BYTE* mHumSpace[2];
		BYTE* mCldSpace[2]; // clouds
		BYTE* mActSpace[2];
		BYTE* mActFactorSpace;

		float* mDensitySpace[2];
		float* mMidDensitySpace;
		float* mCurrentDensitySpace;
		int mElapsedSteps;

	public:
		CloudSimulation();
		virtual ~CloudSimulation();
		void Cleanup();

		bool Setup(unsigned length, unsigned width, unsigned height);

		void InterpolateDensitySpace(float alpha);
		float GetCellDensity(int i, int j, int k);
		float GetPointDensity(const Vec3& point);
		bool IsCellInVolume(int i, int j, int k, int* pIndex = 0);
		bool IsCellInSpace(int i, int j, int k);
		bool IsPointInSpace(const Vec3& point);
		unsigned GetNumCellInVolume() { return mNumCellsInVolume; }
		
		

	protected:		
		void ShapeVolume();
		void InitProbSpace();
		void GrowCloud(unsigned phaseIndex);
		void ExtinctCloud(unsigned phaseIndex);
		void SupplyVapor(unsigned phaseIndex);
		void InitCellInVolume(int i, int j, int k, float probSeed);

		void SetByteCell(BYTE* pByteSpace, int i, int j, int k, BYTE value);
		BYTE GetByteCell(BYTE* pByteSpace, int i, int j, int k);
		bool NewByteSpace(int size, BYTE** ppByteSpace);
		bool NewFloatSpace(int size, float** ppFloatSpace);
		void UpdateActFactorSpace(unsigned phaseIndex);
		void CelluarAutomate(unsigned uPhaseIndex);
		void CalculateDensity(unsigned uPhaseIndex);
	};
}