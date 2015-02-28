#pragma once
#include <CommonLib/SmartPtr.h>
namespace fastbird
{
struct CVar;
struct ConsoleCommand;
class EngineCommand : public ReferenceCounter
{
public:
	EngineCommand();
	~EngineCommand();

	float WheelSens;
	float MouseSens;
	int MoveEditParticle;
	int UI_Debug;
	int e_profile;
	int e_NoMeshLoad;
	int r_UI;
	int r_noObjectConstants;
	int r_noParticleDraw;
	int r_particleProfile;
	int r_HDR;
	float r_HDRMiddleGray;
	int r_HDRCpuLuminance;
	int r_HDRFilmic;
	float r_BloomGaussianWeight;
	float r_BloomPower;
	float r_StarPower;
	int r_GodRay;
	float r_GodRayWeight;
	float r_GodRayDecay;
	float r_GodRayDensity;
	float r_GodRayExposure;
	int r_Glow;
	int r_ReportDeviceObjectLeak;
	int r_Shadow;
	int r_UseShaderCache;
	int r_GenerateShaderCache;
	int r_numRenderTargets;

private:
	std::vector<CVar*> mCVars;
	std::vector<ConsoleCommand*> mCommands;
};
}