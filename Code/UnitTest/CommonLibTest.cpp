#include <UnitTest/StdAfx.h>

#include <CommonLib/Math/fbMath.h>
#include <CommonLib/threads.h>
#include <CommonLib/Profiler.h>

using namespace fastbird;
typedef unsigned long long uint64;
typedef unsigned char uint8;

class PerlinTask: public Task
{
    BYTE* Buffer;
    int Index;
    int X;
    int Y;
    int W;
    int H;
    int S;

    FORCEINLINE float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    FORCEINLINE float lerp(float t, float a, float b) { return a + t * (b - a); }
    FORCEINLINE float grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
        float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
              v = h<4 ? y : h==12||h==14 ? x : z;
        return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
    }

    static int Permutations[];

    FORCEINLINE float Noise3D(float x, float y, float z)
    {
        int floorX = (int)floor(x), floorY = (int)floor(y), floorZ = (int)floor(z);
        int X = floorX & 255,                  // FIND UNIT CUBE THAT
             Y = floorY & 255,                  // CONTAINS POINT.
             Z = floorZ & 255;
        x -= floorX;                                // FIND RELATIVE X,Y,Z
        y -= floorY;                                // OF POINT IN CUBE.
        z -= floorZ;
        float  u = fade(x),                                // COMPUTE FADE CURVES
               v = fade(y),                                // FOR EACH OF X,Y,Z.
               w = fade(z);
        int A = Permutations[X  ]+Y, AA = Permutations[A]+Z, AB = Permutations[A+1]+Z,      // HASH COORDINATES OF
             B = Permutations[X+1]+Y, BA = Permutations[B]+Z, BB = Permutations[B+1]+Z;      // THE 8 CUBE CORNERS,

        return  lerp(w, lerp(v, lerp(u, grad(Permutations[AA  ], x  , y  , z   ),  // AND ADD
                                        grad(Permutations[BA  ], x-1, y  , z   )), // BLENDED
                                lerp(u, grad(Permutations[AB  ], x  , y-1, z   ),  // RESULTS
                                        grad(Permutations[BB  ], x-1, y-1, z   ))),// FROM  8
                        lerp(v, lerp(u, grad(Permutations[AA+1], x  , y  , z-1 ),  // CORNERS
                                        grad(Permutations[BA+1], x-1, y  , z-1 )), // OF CUBE
                                lerp(u, grad(Permutations[AB+1], x  , y-1, z-1 ),
                                        grad(Permutations[BB+1], x-1, y-1, z-1 ))));
    }

public:
    PerlinTask(BYTE* _Buffer, int _X, int _Y, int _W, int _H, int _S, volatile long* ExecCounter)
    : Task(false, !ExecCounter, ExecCounter),
      Buffer(_Buffer),
      X(_X),
      Y(_Y),
      W(_W),
      H(_H),
      S(_S)
    {
        if(ExecCounter)
        {
            InterlockedIncrement(ExecCounter);
        }
    }

    volatile long* GetSyncCounter()
    {
        return &mSyncCounter;
    }

    void Execute(TaskScheduler* Parent)
    {
        BYTE* Ptr = Buffer;
        for(int y=Y; y<Y+H; y++)
        {
            for(int x=X; x<X+W; x++)
            {
                float Result = 0.0f;
                float Scale = 0.001f;
                float Weight = 1.0f;

                for(int o=0; o<16; o++)
                {
                    Result += Noise3D(x * Scale, y * Scale, 1.0f) * Weight;
                    Scale *= 2.0f;
                    Weight *= 0.5f;
                }

                *Ptr++ = (BYTE)(ClampRet(Result * 0.5f + 0.5f, 0.0f, 1.0f) * 255.0f);
            }

            Ptr += (S - W);
        }
    }
};

int PerlinTask::Permutations[] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

TEST(CommonLib, DISABLED_Thread)
{
	TaskScheduler taskScheduler;
	// 
    // Perlin noise test
    //
    // This test computes a 2048x2048 perlin noise with 16 octaves, and compares the results of the
    // single-threaded and multi-threaded versions to ensure they gives the exact same results.
    //
    uint64 Perlin_SingleThread = 0;
    uint64 Perlin_MultiThreads = 0;

    printf("\nRunning perlin noise tests...\n");

    // Single-threaded version
    uint8* ImageST = (uint8*)malloc(2048*2048);
    memset(ImageST, 0, 2048*2048);
	
	float time = 0;
	{
		Profiler profile("Perlin_SingleThread", &time);
		PerlinTask* PerlinTaskST = new PerlinTask(ImageST, 0, 0, 2048, 2048, 2048, NULL);   
		
		PerlinTaskST->Execute(NULL);
		delete PerlinTaskST;
	}
    printf("  Single-thread: %.2fs\n", time);

    // Multi-threaded version
    uint8* ImageMT = (uint8*)malloc(2048*2048);
    memset(ImageMT, 0, 2048*2048);
	{
		time = 0;
		Profiler profile("Perlin_MultiThread", &time);
		PerlinTask* Tasks[32*32] = { 0 };
		for(int y=0; y<32; y++)
		{
			for(int x=0; x<32; x++)
			{
				Tasks[y*32+x] = new PerlinTask(
					ImageMT + y * 64 * 2048 + x * 64, x * 64, y * 64, 64, 64, 2048, 
					Tasks[0] ? Tasks[0]->GetSyncCounter() : NULL);
			}
		}

		for(int i=0; i<32*32; i++)
		{
			taskScheduler.AddTask(Tasks[i]);
		}
		Tasks[0]->Sync();
	}
    printf("  Multi-threads: %.2fs\n", time);

	EXPECT_TRUE(!memcmp(ImageST, ImageMT, 2048*2048));
    free(ImageST);
    free(ImageMT);
}

TEST(CommonLib, Vec3)
{
	using namespace fastbird;
	Vec3 x(1, 0, 0);
	Vec3 y(0, 1, 0);
	EXPECT_TRUE(x.Cross(y) == Vec3::UNIT_Z);
}

TEST(CommonLib, Projection)
{
	using namespace fastbird;
	float theta = 3.141592f/2.f;
	float d = 1.0f/tan(theta/2.f);
	float width = 1920;
	float height= 1080;
	float a = width/height;
	float DofA = d/a;
	float f = 1000.f;
	float n = 10.0f;
	float A = f/(f-n);
	float B = n*f/(n-f);

	Mat44 proj(
		DofA,	0,	0,	0,
		0,		A,	0,	B,
		0,		0,	d,	0,
		0,		1,	0,	0
		);

	Mat44 projYZSwap(
		DofA,	0,	0,	0,		
		0,		0,	d,	0,
		0,		A,	0,	B,
		0,		1,	0,	0
		);

	Vec4 testPosition(0, 5, 10, 1);
	Vec4 result = proj * testPosition;
	Vec4 resultYZSwap = projYZSwap * testPosition;

	EXPECT_TRUE(result.x==resultYZSwap.x && result.y == resultYZSwap.z &&
		result.z == resultYZSwap.y);
}