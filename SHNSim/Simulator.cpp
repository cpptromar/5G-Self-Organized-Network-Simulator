#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>

#include "Simulator.h"
#include "Transceivers.h"
#include "FileIO.h"
#include "UserEquipment.h"
#include "BaseStation.h"
#include "NetworkLogBuffer.h"
#include "Antenna.h"
#include "EnvironmentController.h"
#include "EnvironmentInitialization.h"
#include "ErrorTracer.h"

const float Simulator::PI = 3.14159265F;
const uint32_t Simulator::AP_userTriesPerInput = 5;
const uint32_t Simulator::AP_smallestSideLength = 5;
const uint32_t Simulator::AP_largestSideLength = 20;
const size_t Simulator::AP_fewestAntennae = 3;
const size_t Simulator::AP_mostAntennae = 9;
const size_t Simulator::AP_fewestTransceivers = 1;
const size_t Simulator::AP_mostTransceivers = 500;
const float Simulator::AP_tranceiverLineLimiter = .86F;
const uint32_t Simulator::AP_maxDefaultUEperAntenna = 30;
const uint32_t Simulator::AP_minAvgDataRate = 1;
const uint32_t Simulator::AP_maxAvgDataRate = 20;
const float Simulator::AP_antennaradius = 0.25F;
const uint32_t Simulator::AP_defaultMaxBSDR = 100;
const float Simulator::AP_SNRUpperBound = 12.0f;
const float Simulator::AP_SNRLowerBound = -12.0f;
const float Simulator::AP_SimulationBandwidth = 20;
const float Simulator::AP_ProbChannelChanges = 0.25f;
const float Simulator::AP_ProbAllUsersDRFluctuate = 0.2f;
const int Simulator::AP_DemandFluctuation = 3;
const uint32_t Simulator::AP_MinBSRegionRadius = 3;
const uint32_t Simulator::AP_MaxBSRegionRadius = 500;
const float Simulator::AP_MinUserDistFromBS = 1.2f;
const float Simulator::AP_N0 = 3.9810717e-15f;
const float Simulator::AP_StateCushion = 0.05f;
const float Simulator::AP_MaxCongestionState = 2.00;
const uint32_t Simulator::AP_DefaultECStartTime = 0;
const uint32_t Simulator::AP_DefaultECRiseTime = 1;
const float Simulator::AP_PathLossAlpha = 3.0f;
const uint32_t Simulator::AP_IRPMaxRemovalFailures = 5;

std::mt19937 Simulator::rng_engine;
std::uniform_real_distribution<float> Simulator::rng_real_distribution;
std::uniform_int_distribution<uint32_t> Simulator::rng_int_distribution;

std::vector<BaseStation> Simulator::BaseStationList;
std::vector<UserEquipment> Simulator::UserEquipmentList;
IRPManager Simulator::NetworkManager;

size_t Simulator::numberOfChannels = 0;
float Simulator::BSRegionScalingFactor = 15;
uint32_t Simulator::envClock = 0;
size_t Simulator::numberOfAntennae = 3;
size_t Simulator::numberOfTransceivers = 100;
uint32_t Simulator::healthyBSNumUsersPerAnt = 15;
float Simulator::distanceBetweenTransceivers = 0;
uint32_t Simulator::BSmaxDataRate = 0;
uint32_t Simulator::simulationLength = 100;
uint32_t Simulator::numberOfSimulations = 1;
uint32_t Simulator::simulationStartNum = 0;
size_t Simulator::currentChannel = 0;
uint32_t Simulator::IRPBufSizeInSeconds = 10;
float Simulator::alertState = 0.8f;
float Simulator::defaultNormalState = 0.5f;
float Simulator::defaultCongestionState = 0.9f;
float Simulator::defaultFailureState = 0.0f;


void Simulator::setNumOfChannels(const size_t& nChannels)
{
	Simulator::numberOfChannels = nChannels;
}

void Simulator::incrementCurrentChannel()
{
	Simulator::currentChannel = (Simulator::currentChannel + 1) % Simulator::numberOfChannels;
}

bool Simulator::SimulatorInit()
{
	Simulator::initializeRAND();

	if (!EnvironmentInitialization::generateNewENV())
		return ErrorTracer::error("\nEnvironmentInitialization::generateNewENV() failed in Simulator::SimulatorInit()");

	return true;
}

void Simulator::initializeRAND()
{
	Simulator::rng_engine = std::mt19937((unsigned)time(nullptr));
	Simulator::rng_real_distribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
	Simulator::rng_int_distribution = std::uniform_int_distribution<uint32_t>();
}

BaseStation& Simulator::getBS_m(const size_t& BSID)
{
	return Simulator::BaseStationList[BSID];
}

UserEquipment& Simulator::getUE_m(const size_t& UEID)
{
	return Simulator::UserEquipmentList[UEID];
}

std::vector<BaseStation>& Simulator::getBSList_m()
{
	return Simulator::BaseStationList;
}

std::vector<UserEquipment>& Simulator::getUEList_m()
{
	return Simulator::UserEquipmentList;
}

IRPManager& Simulator::getIRPManager_m()
{
	return Simulator::NetworkManager;
}

void Simulator::addBS(const BaseStation& BS)
{
	Simulator::BaseStationList.push_back(BS);
}

void Simulator::addUE(const UserEquipment& UE)
{
 	Simulator::UserEquipmentList.push_back(UE);
}

void Simulator::setBSRegionScalingFactor(const float& bsScale)
{
	Simulator::BSRegionScalingFactor = bsScale;
}

void Simulator::setEnvClock(const uint32_t& eClk)
{
	Simulator::envClock = eClk;
}

void Simulator::setNumberOfAntennae(const size_t& numOfAntennae)
{
	Simulator::numberOfAntennae = numOfAntennae;
}

void Simulator::setHealthyBSNumUsersPerAnt(const uint32_t& healthyBSNumUsersPerAnt)
{
	Simulator::healthyBSNumUsersPerAnt = healthyBSNumUsersPerAnt;
}

void Simulator::setNumberOfTransceivers(const size_t& numOfTransceivers)
{
	Simulator::numberOfTransceivers = numOfTransceivers;
}

void Simulator::setDistanceBetweenTransceivers(const float& distBetweenTransceivers)
{
	Simulator::distanceBetweenTransceivers = distBetweenTransceivers;
}

void Simulator::setBSMaxDR(const uint32_t& BSMDR)
{
	Simulator::BSmaxDataRate = BSMDR;
}


void Simulator::setSimulationLength(const uint32_t& simLen)
{
	Simulator::simulationLength = simLen;
}

void Simulator::setNumberOfSimulations(const uint32_t& numSim)
{
	Simulator::numberOfSimulations = numSim;
}

void Simulator::setSimulationStartNum(const uint32_t& simStNum)
{
	Simulator::simulationStartNum = simStNum;
}

void Simulator::setCurrentChannel(const size_t& channel)
{
	Simulator::currentChannel = channel;
}

void Simulator::setIRPBufSizeInSeconds(const uint32_t& bufSize)
{
	Simulator::IRPBufSizeInSeconds = bufSize;
}

void Simulator::setDefaultNormalState(const float& normlvl)
{
	Simulator::defaultNormalState = normlvl;
}

void Simulator::setDefaultCongestionState(const float& conglvl)
{
	Simulator::defaultCongestionState = conglvl;
}

void Simulator::setDefaultFailureState(const float& faillvl)
{
	Simulator::defaultFailureState = faillvl;
}

void Simulator::resetCoreObjects()
{
	Simulator::BaseStationList.clear();
	Simulator::UserEquipmentList.clear();
}

void Simulator::setAlertState(const float& alertlvl)
{
	Simulator::alertState = alertlvl;
}

const size_t& Simulator::getNumOfChannels()
{
	return Simulator::numberOfChannels;
}

size_t Simulator::getNumOfBSs()
{
	return Simulator::BaseStationList.size();
}

size_t Simulator::getNumOfUsers()
{
	return Simulator::UserEquipmentList.size();
}

//FLAG
const BaseStation& Simulator::getBS(const size_t& BSID)
{
	return Simulator::BaseStationList[BSID];
}

//FLAG
const UserEquipment& Simulator::getUE(const size_t& UEID)
{
	return Simulator::UserEquipmentList[UEID];
}

const std::vector<BaseStation>& Simulator::getBSList()
{
	return Simulator::BaseStationList;
}

const std::vector<UserEquipment>& Simulator::getUEList()
{
	return Simulator::UserEquipmentList;
}

const IRPManager& Simulator::getIRPManager()
{
	return Simulator::NetworkManager;
}

const float& Simulator::getBSRegionScalingFactor()
{
	return Simulator::BSRegionScalingFactor;
}

const uint32_t& Simulator::getEnvClock()
{
	return Simulator::envClock;
}

const size_t& Simulator::getNumberOfAntennae()
{
	return Simulator::numberOfAntennae;
}

const uint32_t& Simulator::getHealthyBSNumUsersPerAnt()
{
	return Simulator::healthyBSNumUsersPerAnt;
}

const size_t& Simulator::getNumberOfTransceivers()
{
	return Simulator::numberOfTransceivers;
}

const float& Simulator::getDistanceBetweenTransceivers()
{
	return Simulator::distanceBetweenTransceivers;
}

const uint32_t& Simulator::getBSMaxDR()
{
	return Simulator::BSmaxDataRate;
}

const uint32_t& Simulator::getSimulationLength()
{
	return Simulator::simulationLength;
}

const uint32_t& Simulator::getNumberOfSimulations()
{
	return Simulator::numberOfSimulations;
}

const uint32_t& Simulator::getSimulationStartNum()
{
	return Simulator::simulationStartNum;
}

const size_t& Simulator::getCurrentChannel()
{
	return Simulator::currentChannel;
}

const uint32_t& Simulator::getIRPBufSizeInSeconds()
{
	return Simulator::IRPBufSizeInSeconds;
}

const float& Simulator::getDefaultNormalState()
{
	return Simulator::defaultNormalState;
}

const float& Simulator::getDefaultCongestionState()
{
	return Simulator::defaultCongestionState;
}

const float& Simulator::getDefaultFailureState()
{
	return Simulator::defaultFailureState;
}

const float& Simulator::getAlertState()
{
	return Simulator::alertState;
}

std::mt19937& Simulator::getRandNumEngine()
{
	return Simulator::rng_engine;
}

const float Simulator::randF()
{
	return Simulator::rng_real_distribution(Simulator::rng_engine);
}

const uint32_t Simulator::rand()
{
	return Simulator::rng_int_distribution(Simulator::rng_engine);
}

void Simulator::sendUETransmission(const size_t& ueID, const uint32_t& dataAmt, const float& pwrTrans)
{
	if (ueID < Simulator::getNumOfUsers())
		Simulator::UserEquipmentList[ueID].receive(dataAmt, pwrTrans, Simulator::AP_PathLossAlpha);
	else
		ErrorTracer::error("USERID invalid (" + std::to_string(ueID) + ") Transmission Failed");
}

bool Simulator::transferUE(const size_t& bsOrigin, const size_t& UE, const size_t& bsRecieve, const size_t& antReceive)
{
	auto UER = Simulator::getBS(bsOrigin).getUEDB().look_up(UE);
	if (UER && Simulator::getBS_m(bsRecieve).addUE(*UER, antReceive) && Simulator::getBS_m(bsOrigin).removeUE(UE))
		return true;
	else
		return false;
}

float Simulator::generateSNR(float distanceSquared)
{
	const auto distanceToLowerBoundSquared = pow(3 * Simulator::BSRegionScalingFactor, 2);
	const auto range = (Simulator::AP_SNRUpperBound - Simulator::AP_SNRLowerBound);
	return range / (((range - 1) / distanceToLowerBoundSquared) * distanceSquared + 1) + (Simulator::AP_SNRLowerBound);
	
}

//FLAG upgrade c++
bool Simulator::environmentTick()
{
	EnvironmentController::ECUpdate();

	//asks each user equipment to update
	for (size_t uEquip = 0; uEquip < Simulator::getNumOfUsers(); uEquip++) {

		// returns false if the update function in the user equipment
		// returns false, which means it has not successfully updated
		if (!Simulator::UserEquipmentList[uEquip].Update())
			return ErrorTracer::error("\nSimulator::UserEquipmentList[uEquip].Update() failed in Simulator::environmentTick()");

	}

	//asks each basestation to update.
	for (size_t bs = 0; bs < Simulator::getNumOfBSs(); bs++) {

		// returns false if the update function in the BaseStation
		// returns false, which means it has not successfully updated
		if (!Simulator::BaseStationList[bs].Update())
			return ErrorTracer::error("\nSimulator::BaseStationList[bs].Update() failed in Simulator::environmentTick()");

	}

	Simulator::NetworkManager.IRPManagerUpdate();

	return true;
}

bool Simulator::runSimulation()
{
	//This for loop iterates through the simulations.
	for (auto sim = Simulator::simulationStartNum; sim < Simulator::numberOfSimulations + Simulator::simulationStartNum; sim++)
	{
		//tells user which simulation is running
		std::cout << "\nSimulation #" << sim;

		//each time the simulation is run the environment is reinitialized 
		//..to the starting state from the original save file
		if (!FileIO::readSaveFileIntoSim())
			return ErrorTracer::error("\nFileIO::readSaveFileToENV() failed in Simulator::runSimulation()");

		//clock starts at 0
		Simulator::envClock = 0;

		//resets count variables controlling the output
		FileIO::resetLog();

		const auto numSize = static_cast<size_t>(log10(Simulator::simulationLength) + 1);		//number of digits in the simulation length

		std::cout << "\nStarting Time: " << std::string(numSize, ' ') << envClock;	//outputs the starting time and current time
		std::cout << "\nCurrent  Time: " << std::string(numSize, ' ') << envClock;	//the string with the spaces aligns the numbers 
																					//and provides padding for when it counts up to the final time
		//the following four variables all relate to the clock being displayed to user
		auto curPwrOfTen = uint32_t{ 1 };
		auto curNumSize = size_t{ 0 };
		auto formatPadding = std::string(numSize - curNumSize, ' ');
		const auto backspaceContainer = std::string(numSize, '\b');
		
		//simulation takes place in environment tick, it stops
		while (Simulator::simulationLength > Simulator::envClock)
		{
			if (!Simulator::environmentTick())
				return ErrorTracer::error("\nSimulator::environmentTick() failed in Simulator::runSimulation()");

			//adds to log
			if (!FileIO::appendLog(sim))
				return ErrorTracer::error("\nFileIO::appendLog failed in Simulator::runSimulation()");

			//clock is incremented
			Simulator::envClock++;

			//the following is a conscise way of updating the clock shown to user
			if (Simulator::envClock % curPwrOfTen == 0) { curPwrOfTen *= 10; curNumSize++; formatPadding = std::string(numSize - curNumSize, ' '); }
			std::cout << backspaceContainer << formatPadding << Simulator::envClock;
		}

		//saves the simulation end state
		const auto addendum = '.' + std::to_string(sim) + ".save";

		if (!FileIO::writeInitalSimulationState(addendum))
			return ErrorTracer::error("\nFileIO::writeSaveFileFromENV failed in Simulator::runSimulation()");
	}
	std::cout << '\n';
	return true;
}
