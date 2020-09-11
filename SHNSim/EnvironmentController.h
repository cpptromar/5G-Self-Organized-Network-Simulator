#pragma once
#include "Simulator.h"
#include "BSFailureParams.h"

class EnvironmentController : private Simulator
{
private:
	static std::vector<BSFailureParams> BSRegionControlInfo;
	static float averageUEStateContribution;

	static bool checkBSFPValidity();
	static void calculateUEContrib();
	static void updateCurrentStates();
	static void channelFluctuation();
	static void DRFluctuation();
	static void rampingState(BSFailureParams&, const int&);
	static void restoreState(BSFailureParams&);
	static void modifyState(BSFailureParams&, const float&);
	static void incrementDemands(BSFailureParams&, const uint32_t&, float&);
	static void decrementDemands(BSFailureParams&, const uint32_t&, float&);
	static void addUsers(BSFailureParams&, const uint32_t&, float&);
	static void removeUsers(BSFailureParams&, const uint32_t&, float&);
public:
	static bool ECInitialization(std::vector<BSFailureParams>&);
	static void ECUpdate();
	static const BSstatus& getCurrentBSStatus(const size_t&);
	static const BSstatus& getEndBSStatus(const size_t&);
};