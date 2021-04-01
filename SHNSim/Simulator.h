#pragma once

#include <string>
#include <random>
#include <chrono>
#include <cstdint>

#include "Coord.h"
#include "BaseStation.h"
#include "UserEquipment.h"
#include "UELogData.h"
#include "IRPManager.h"
#include "ErrorTracer.h"
#include "DataRateTable.h"
#include "Setup.h"

class Setup;
class Simulator : public ErrorTracer
{
private:
	///////////////////////////////////////////////////////////////////////////////////
	/*                         CRITICAL STATE PARAMETERS                             */
	///////////////////////////////////////////////////////////////////////////////////

	static std::mt19937 rng_engine;
	static std::uniform_real_distribution<float> rng_real_distribution;
	static std::uniform_int_distribution<uint32_t> rng_int_distribution;

	static size_t		numberOfChannels;
	static float		BSRegionScalingFactor;
	static uint32_t		envClock;
	static size_t		numberOfAntennae;
	static uint32_t		healthyBSNumUsersPerAnt;
	static size_t		numberOfTransceivers;
	static float		distanceBetweenTransceivers;
	static uint32_t		BSmaxDataRate;
	static uint32_t		simulationLength;
	static uint32_t		numberOfSimulations;
	static uint32_t		simulationStartNum;
	static size_t		currentChannel;
	static uint32_t		IRPBufSizeInSeconds;
	static float		alertState;
	static float		defaultNormalState;
	static float		defaultCongestionState; 
	static float		defaultFailureState;
	static uint32_t		mobilityBufSizeInMinutes;
	static float		RSRPThreshold;

	///////////////////////////////////////////////////////////////////////////////////
	/*                            CORE FUNCTIONALITY                                 */
	///////////////////////////////////////////////////////////////////////////////////
	static bool	SimulatorInit();
	static void	initializeRAND();
	static bool	environmentTick();
	static bool	runSimulation();
	friend bool	Setup::GUIentryPoint();

	static std::vector<BaseStation>		BaseStationList;
	static std::vector<UserEquipment>	UserEquipmentList;
	static IRPManager					NetworkManager;

protected:
	///////////////////////////////////////////////////////////////////////////////////
	/*                                Setters                                        */
	///////////////////////////////////////////////////////////////////////////////////
	static BaseStation&					getBS_m(const size_t& BSID);
	static UserEquipment&				getUE_m(const size_t& UEID);
	static std::vector<BaseStation>&	getBSList_m();
	static std::vector<UserEquipment>&	getUEList_m();
	static IRPManager&					getIRPManager_m();

	static void  addBS(const BaseStation& BS);
	static void  addUE(const UserEquipment& UE);

	static void setNumOfChannels(const size_t& nChannels);
	static void incrementCurrentChannel();
	static void setBSRegionScalingFactor(const float& bsScale);
	static void setEnvClock(const uint32_t& eClk);
	static void setNumberOfAntennae(const size_t& numOfAntennae);
	static void setHealthyBSNumUsersPerAnt(const uint32_t& healthyBSNumUsersPerAnt);
	static void setNumberOfTransceivers(const size_t& numOfTransceivers);
	static void setDistanceBetweenTransceivers(const float& distanceBetweenTransceivers);
	static void setBSMaxDR(const uint32_t& BSmaxDataRate);
	static void setSimulationLength(const uint32_t& simulationLength);
	static void setNumberOfSimulations(const uint32_t& numberOfSimulations);
	static void setSimulationStartNum(const uint32_t& simulationStartNum);
	static void setCurrentChannel(const size_t& channel);
	static void setIRPBufSizeInSeconds(const uint32_t& bufSize);
	static void setAlertState(const float& alertlvl);
	static void setDefaultNormalState(const float& normlvl);
	static void setDefaultCongestionState(const float& conglvl);
	static void setDefaultFailureState(const float& faillvl);
	static void resetCoreObjects();
	static void setmobilityBufSizeInMinutes(const uint32_t& mobBufSize);

public:
	///////////////////////////////////////////////////////////////////////////////////
	/*                             ARBITRARY PARAMETERS                              */
	///////////////////////////////////////////////////////////////////////////////////
	//defines constants: AP stands for "arbitrary parameter"
	//APs are primarily used to ensure valid input in the environment creation process
	//theyre all pretty much arbitrary, but chosen with care to ensure valid environment creation

	static const float		PI;
	static const uint32_t	AP_userTriesPerInput;
	static const uint32_t	AP_smallestSideLength;
	static const uint32_t	AP_largestSideLength;
	static const size_t		AP_fewestAntennae;
	static const size_t		AP_mostAntennae;
	static const size_t		AP_fewestTransceivers;
	static const size_t		AP_mostTransceivers;
	static const float		AP_tranceiverLineLimiter;
	static const uint32_t	AP_maxDefaultUEperAntenna;
	static const uint32_t	AP_minAvgDataRate;
	static const uint32_t	AP_maxAvgDataRate;
	static const float		AP_antennaradius;
	static const uint32_t	AP_defaultMaxBSDR;
	static const float		AP_SNRUpperBound;
	static const float		AP_SNRLowerBound;
	static const float		AP_SimulationBandwidth;
	static const float		AP_ProbChannelChanges;
	static const float		AP_ProbAllUsersDRFluctuate;
	static const float		AP_ProbBitsDropped;
	static const int		AP_DemandFluctuation;
	static const uint32_t	AP_MinBSRegionRadius;
	static const uint32_t	AP_MaxBSRegionRadius;
	static const float		AP_MinUserDistFromBS;
	static const float		AP_DemandFluctuationBound;
	static const float		AP_N0;
	static const float		AP_StateCushion;
	static const float		AP_MaxCongestionState;
	static const uint32_t	AP_DefaultECStartTime;
	static const uint32_t	AP_DefaultECRiseTime;
	static const float		AP_PathLossAlpha;
	static const uint32_t	AP_IRPMaxRemovalFailures;

	///////////////////////////////////////////////////////////////////////////////////
	/*                                Getters                                        */
	///////////////////////////////////////////////////////////////////////////////////

	static size_t								getNumOfBSs();
	static size_t								getNumOfUsers();
	static const BaseStation&					getBS(const size_t& BSID);
	static const UserEquipment&					getUE(const size_t& UEID);
	static const std::vector<BaseStation>&		getBSList();
	static const std::vector<UserEquipment>&	getUEList();
	static const IRPManager&					getIRPManager();

	static const size_t&	getNumOfChannels();
	static const float&		getBSRegionScalingFactor();
	static const uint32_t&	getEnvClock();
	static const size_t&	getNumberOfAntennae();
	static const uint32_t&	getHealthyBSNumUsersPerAnt();
	static const size_t&	getNumberOfTransceivers();
	static const float&		getDistanceBetweenTransceivers();
	static const uint32_t&	getBSMaxDR();
	static const uint32_t&	getSimulationLength();
	static const uint32_t&	getNumberOfSimulations();
	static const uint32_t&	getSimulationStartNum();
	static const size_t&	getCurrentChannel();
	static const uint32_t&	getIRPBufSizeInSeconds();
	static const float&		getAlertState();
	static const float&		getDefaultNormalState();
	static const float&		getDefaultCongestionState();
	static const float&		getDefaultFailureState();
	static std::mt19937&	getRandNumEngine();
	static const uint32_t&	getmobilityBufSizeInMinutes();

	static const float		randF();
	static const uint32_t	rand();

	static void				sendUETransmission(const size_t&, const uint32_t&, const float&);
	static bool				transferUE(const size_t&, const size_t&, const size_t&, const size_t&);
	static bool				moveUE(const size_t&, const size_t&, const Coord<float>&);
	static float			generateSNR(float distanceSquared);
};
