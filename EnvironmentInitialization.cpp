#include <iostream>

#include "Simulator.h"
#include "EnvironmentInitialization.h"
#include "Antenna.h"
#include "GUIDataContainer.h"
#include "DataRateTable.h"
#include "EnvironmentController.h"
#include "FileIO.h"
#include "ErrorTracer.h"
#include <time.h>

//Gets the normal amount of users
	//Creates the UE records for BS in normal condition
bool EnvironmentInitialization::setDefaultUsers()
{
	for (auto& bs : Simulator::getBSList_m())
	{
		for (const auto& ant : bs.getAntennaVec())
		{
			for (auto ue = size_t{ 0 }; ue < floor((Simulator::getHealthyBSNumUsersPerAnt() * bs.getBaseStationPopulationDensity())/5); ue++) //Simulator::getHealthyBSNumUsersPerAnt()
			{
				//gets a randomly point
				const auto& radiusLimit = [](const auto& a) {return ((a < Simulator::AP_MinUserDistFromBS) ? Simulator::AP_MinUserDistFromBS : a); };
				const auto rRadius = float{ radiusLimit(Simulator::randF() * Simulator::getBSRegionScalingFactor()) };
				const auto rPhase = float{ 2.0f * (Simulator::randF() - 0.5f) * Simulator::PI / Simulator::getNumberOfAntennae() + ant.getAngle() * Simulator::PI / 180.0f };
				const auto loc = Coord<float>{ static_cast<float>(rRadius * cos(rPhase)), static_cast<float>(rRadius * sin(rPhase)) };

				const auto distanceSquared = float{static_cast<float>(pow(loc.x, static_cast<int>(2)) + pow(loc.y, static_cast<int>(2)))};
				const auto SNR = Simulator::generateSNR(distanceSquared);
				const auto dataRate = DataRateTable::getDataRate(SNR, Simulator::getCurrentChannel());

				//next user to be added will have the current # of users. E.G. if there are 0 UEs then the first ID = 0.
				const auto currUserID = Simulator::getNumOfUsers();

				//Generate a random mobility ID for the current user [0 = Stationary, 1 = Walking, 2 = Driving (car)]
				const auto currMobilityID = (Simulator::rand() % 3);
				
				const auto BaseStationID = bs.getBSID();
				const auto BaseStationStatus = (BSstatus)(GUIDataContainer::status[BaseStationID]);

				//tranceiver set to the UE
				const auto currentTranceiver = bs.getAntenna(ant.getAntID()).getConnectionInfo_m().addUser(currUserID);
				if (!currentTranceiver.first)
					continue;
				
				uint32_t currentDemand = 0;
				if (BaseStationStatus == BSstatus::normal ||
					BaseStationStatus == BSstatus::congestionUsers ||
					BaseStationStatus == BSstatus::failure)
				{
					 currentDemand = uint32_t{ (Simulator::rand() % (dataRate + 1)) };
				}
				else if (BaseStationStatus == BSstatus::congestionDemand)
				{
					currentDemand = uint32_t{ dataRate };
				}
				else
				{
					 currentDemand = uint32_t{ (Simulator::rand() % (dataRate + 1)) };
				}
				//std::cout << currentDemand << "  " << dataRate << "  \n";

				const auto newRecord = UERecord{ 
					currUserID, 
					currMobilityID,
					Coord<float>{loc.x + bs.getLoc().x, loc.y + bs.getLoc().y}, 
					ant.getAntID(), 
					currentTranceiver.second, 
					SNR, 
					currentDemand, 
					0, 
					0,
					0,
					0,
					0,
					0
				};
				bs.addUERecord(newRecord);

				const auto& numChan = Simulator::getNumOfChannels();
				auto ch = size_t{ 0 };
				std::vector<uint32_t> possMaxDrsForUE = std::vector<uint32_t>(numChan, 0);

				for (auto& dr : possMaxDrsForUE)
				{
					dr = DataRateTable::getDataRate(SNR, ch++);
					if (dr == 0)
						return ErrorTracer::error("\nINVALID DATARATE IN possMaxDrsForUE, POSSIBLE DRTBL ISSUE in EnvironmentInitialization::setDefaultUsers()");
				}

				const auto newLoc = Coord<float>{ loc.x + bs.getLoc().x, loc.y + bs.getLoc().y };
				auto newUser = UserEquipment{ newLoc, currUserID, currMobilityID, possMaxDrsForUE, currentDemand };
				Simulator::addUE(newUser);
			}
			
		}
	}

	return true;

}

//Does as name suggests
//This applies to all BSs
//FLAG CHECK DIVISION
bool EnvironmentInitialization::setBSMaxDataRate()
{
	
	//total data rates of the normal BSs
	auto bsNetDataRate = uint32_t{ 0 };
	//iterates through UEs
	for (const auto& ue: Simulator::getUEList())
		bsNetDataRate += ue.getDemand();

	//gets average data rate for a normal base station
	bsNetDataRate /= Simulator::getNumOfBSs();

	//sets the max bs data rate to be that everage plus padding to ensure normal operation for normal base stations
	Simulator::setBSMaxDR(static_cast<uint32_t>(bsNetDataRate /Simulator::getDefaultNormalState()));
	
	

	return true;
}

//FLAG ARRAY ACCESS
std::vector<Coord<float>> EnvironmentInitialization::setBSCoords(const std::vector<std::pair<int, int>>* neighbors, int numBSs)
{
	auto BaseStationLocations = std::vector<Coord<float>>(numBSs);
	std::fill(BaseStationLocations.begin(), BaseStationLocations.end(), Coord<float>(0.0f, 0.0f));

	auto bsIsInstantiated = std::vector<bool>(numBSs, false);
	bsIsInstantiated[0] = true;

	auto BSInstantionQueue = std::vector<size_t>{ 0 };

	auto currBS = size_t{};
	auto nextBS = size_t{};
	auto nextSideNum = size_t{};

	auto currBSLoc = Coord<float>{ 0, 0 };
	auto nextBSLoc = Coord<float>{ 0, 0 };

	auto distBetweenHex = float{ Simulator::getBSRegionScalingFactor() * 2.0f };
	while(!BSInstantionQueue.empty())
	{
		currBS = BSInstantionQueue.back();
		currBSLoc = BaseStationLocations[currBS];

		BSInstantionQueue.pop_back();

		for (const auto& neihbr :GUIDataContainer::neighbors[currBS])
		{
			nextBS = neihbr.first;

			if (bsIsInstantiated[nextBS])
				continue;
			else
			{
				bsIsInstantiated[nextBS] = true;
				BSInstantionQueue.push_back(nextBS);
			}

			nextSideNum = neihbr.second;

			nextBSLoc.x = currBSLoc.x + distBetweenHex * std::cos(Simulator::PI * (0.5f - ((1.0f / 3.0f) * ((float)(nextSideNum)-1))));
			nextBSLoc.y = currBSLoc.y + distBetweenHex * std::sin(Simulator::PI * (0.5f - ((1.0f / 3.0f) * ((float)(nextSideNum)-1))));
			BaseStationLocations[nextBS] = nextBSLoc;
		}
	}
	return BaseStationLocations;
}

void EnvironmentInitialization::initializeNumTransceivers()
{
	auto minVal1 = Simulator::AP_tranceiverLineLimiter / Simulator::getNumberOfTransceivers();
	auto minVal2 = 2.0f * Simulator::AP_antennaradius * tanf(Simulator::PI / Simulator::getNumberOfAntennae()) / Simulator::getNumberOfTransceivers();
	auto minDistance = (minVal1 < minVal2) ? minVal1 : minVal2;
	Simulator::setDistanceBetweenTransceivers(minDistance * 0.9f);
}

bool EnvironmentInitialization::generateNewENV()
{
	//arbitrarily starts simulation @ a random channel
	Simulator::setCurrentChannel(Simulator::rand() % Simulator::getNumOfChannels());

	Simulator::setBSRegionScalingFactor(static_cast<float>(GUIDataContainer::bsLen));
	Simulator::setNumberOfAntennae(GUIDataContainer::antNum);
	Simulator::setNumberOfTransceivers(GUIDataContainer::transNum);
	Simulator::setHealthyBSNumUsersPerAnt(GUIDataContainer::uePerAnt);
	EnvironmentInitialization::initializeNumTransceivers();

	//User Mobility / Movement parameters
	Simulator::setmobilityBufSizeInMinutes(GUIDataContainer::mobilityBufSizeInMinutes);

	//Self-Healing/Optimization parameters
	Simulator::setalgorithmVer(GUIDataContainer::algorithmVer);
	Simulator::setRSRPThreshold(GUIDataContainer::RSRPThreshold);

	std::vector<Coord<float>> BaseStationLocations = EnvironmentInitialization::setBSCoords(GUIDataContainer::neighbors, GUIDataContainer::count);

	//Initializes actual BaseStations.
	
	//uint32_t AttractivenessArray[8] = { ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1), ((Simulator::rand() % 10) + 1) }; //hard coded for now
	//uint32_t PopulationDensityArray[8] = { ((Simulator::rand() % 10) +1), (Simulator::rand() % 10) + 1, (Simulator::rand() % 10) + 1, (Simulator::rand() % 10) + 1, (Simulator::rand() % 10) + 1, (Simulator::rand() % 10) + 1, (Simulator::rand() % 10) + 1, (Simulator::rand() % 10) + 1 }; // add to gui later



	auto bsCount = size_t{ 0 };
	int bscounta = 0;
	int bscountp = 0;
	for (const auto& bsLoc : BaseStationLocations)
	{
		Simulator::addBS(BaseStation{ bsCount++, bsLoc, false, GUIDataContainer::AttractivenessArray[bscounta++], GUIDataContainer::PopulationDensityArray[bscountp++] });
	}

	Simulator::setIRPBufSizeInSeconds(GUIDataContainer::bufSizeInSeconds);
	Simulator::setAlertState((float)(GUIDataContainer::alertState)/100.0f);
	Simulator::setDefaultNormalState(0.5f);
	Simulator::setDefaultCongestionState((float)(GUIDataContainer::congestionState)/100.0f);

	//Gets the normal amount of users
	//Creates the UE records for BS in normal condition
	if (!EnvironmentInitialization::setDefaultUsers())
		return ErrorTracer::error("\nEnvironmentInitialization::setDefaultUsers() failed in EnvironmentInitialization::generateNewENV()");

	//Does as name suggests
	//This applies to all BSs
	if (!EnvironmentInitialization::setBSMaxDataRate())
		return ErrorTracer::error("\nEnvironmentInitialization::setBSMaxDataRate() failed in EnvironmentInitialization::generateNewENV()");

	auto BSFP_TEST = std::vector<BSFailureParams>();
	auto bsInd = size_t{0};
	for (const auto& bs : Simulator::getBSList())
	{
		auto users = std::vector<size_t>{};
		users.reserve(bs.getUEDB().size());
		for (const auto& ue : bs.getUEDB().readDB())
			users.push_back((*ue).userID);

		//BSFP_TEST.push_back(BSFailureParams(bsInd, 0.0f, 0.5, BSstatus::normal, BSstatus::normal, 0, 1, users));
		BSFP_TEST.push_back(BSFailureParams( bsInd, 0.0f, float{(float)(GUIDataContainer::endState[bsInd])/100.0f}, BSstatus::normal, (BSstatus)(GUIDataContainer::status[bsInd]), (uint32_t)GUIDataContainer::startTime[bsInd], (uint32_t)GUIDataContainer::riseTime[bsInd], users ));
		bsInd++;
	}
	/*
	BSFP_TEST[0].endState = 0.3f;
	BSFP_TEST[0].startTime = 10;
	BSFP_TEST[0].riseTime = 15;

	BSFP_TEST[1].endStatus = BSstatus::congestionUsers;
	BSFP_TEST[1].endState = 1.3f;
	BSFP_TEST[1].startTime = 10;
	BSFP_TEST[1].riseTime = 15;

	BSFP_TEST[2].endStatus = BSstatus::congestionDemand;
	BSFP_TEST[2].endState = 1.0f;
	BSFP_TEST[2].startTime = 50;
	BSFP_TEST[2].riseTime = 30;

	BSFP_TEST[3].endStatus = BSstatus::normal;
	BSFP_TEST[3].endState = 0.5f;
	BSFP_TEST[3].startTime = 20;
	BSFP_TEST[3].riseTime = 45;*/

	//BSFP_TEST[0].endStatus = BSstatus::congestionDemand;
	//BSFP_TEST[0].endState = 1.2f;
	//BSFP_TEST[0].startTime = 10;
	//BSFP_TEST[0].riseTime = 20;

	Simulator::setSimulationLength(GUIDataContainer::simLen);
	Simulator::setNumberOfSimulations(GUIDataContainer::simNum);
	Simulator::setSimulationStartNum(GUIDataContainer::simStartNum);
	FileIO::setSimulationSaveName(GUIDataContainer::simName);

	if (!EnvironmentController::ECInitialization(BSFP_TEST))
		return ErrorTracer::error("\nEnvironmentController::ECInitialization failed in EnvironmentInitialization::generateNewENV()");

	Simulator::getIRPManager_m().InitializeIRPManager();

	return true;
}



