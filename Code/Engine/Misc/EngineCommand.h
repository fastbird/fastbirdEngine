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
	int UI_Debug;
	int r_noObjectConstants;
	int r_noParticleDraw;
	int e_profile;
	int r_particleProfile;
	int r_HDR;
	int r_GodRay;
	float r_GodRayWeight;
	float r_GodRayDecay;
	float r_GodRayDensity;
	float r_GodRayExposure;
	int r_Glow;
	int r_ReportDeviceObjectLeak;
	int MoveEditParticle;

private:
	std::vector<CVar*> mCVars;
	std::vector<ConsoleCommand*> mCommands;
};
}