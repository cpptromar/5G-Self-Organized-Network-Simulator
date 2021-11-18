#include <iostream>
#include <fstream>
#include <cstdint>
#include <utility>
#include <cmath>

#include "BSFailureParams.h"
#include "EnvironmentController.h"
#include "UEDataBase.h"
#include "DataRateTable.h"
#include "BaseStation.h"
#include "Antenna.h"
#include "FileIO.h"
#include "ErrorTracer.h"
#include <string>
#include <sstream>


std::vector<BSFailureParams> EnvironmentController::BSRegionControlInfo = std::vector<BSFailureParams>{};
float EnvironmentController::averageUEStateContribution = float{ 0.0f };

bool EnvironmentController::ECInitialization(std::vector<BSFailureParams>& ctrlInfo)
{
	EnvironmentController::BSRegionControlInfo = std::move(ctrlInfo);

	//if (!EnvironmentController::checkBSFPValidity())
	//	return ErrorTracer::error("\ncheckBSFPValidity() failed in EnvironmentController::ECInitialization()");

	EnvironmentController::updateCurrentStates();

	EnvironmentController::calculateUEContrib();

	return true;
}

bool EnvironmentController::checkBSFPValidity()
{
	for (auto& bsInfo : BSRegionControlInfo)
	{
		if(bsInfo.currentState > Simulator::AP_MaxCongestionState || bsInfo.endState > Simulator::AP_MaxCongestionState)
			return ErrorTracer::error("\nBSFP(" + std::to_string(bsInfo.bsID) + ") INVALID DUE TO INCORRECT RISE/START TIMES in EnvironmentController::checkBSFPValidity()");

		if (bsInfo.startTime < 0 || bsInfo.riseTime < 1)
			return ErrorTracer::error("\nBSFP(" + std::to_string(bsInfo.bsID) + ") INVALID DUE TO INCORRECT RISE/START TIMES in EnvironmentController::checkBSFPValidity()");

		if (bsInfo.endStatus == BSstatus::normal)
		{
			bsInfo.currentStatus = BSstatus::normal;
			if (bsInfo.endState < 0 || bsInfo.endState >= Simulator::getDefaultCongestionState() - Simulator::AP_StateCushion)
				return ErrorTracer::error("\nBSFP(" + std::to_string(bsInfo.bsID) + ") INVALID DUE TO INCORRECT endState in EnvironmentController::checkBSFPValidity()");
		}
		else if (bsInfo.endStatus == BSstatus::congestionDemand || bsInfo.endStatus == BSstatus::congestionUsers)
		{
			if (bsInfo.endState <= Simulator::getDefaultCongestionState() || bsInfo.endState >= Simulator::AP_MaxCongestionState)
				return ErrorTracer::error("\nBSFP(" + std::to_string(bsInfo.bsID) + ") INVALID DUE TO INCORRECT endState in EnvironmentController::checkBSFPValidity()");
		}
		else if (bsInfo.endStatus == BSstatus::failure)
		{
			bsInfo.endState = Simulator::getDefaultFailureState();
		}
		else
			return ErrorTracer::error("\nBSFP(" + std::to_string(bsInfo.bsID) + ") INVALID: UNKOWN endStatus (SHOULD NEVER HAPPEN) in EnvironmentController::checkBSFPValidity()");
		
		//std::cout << "\n" << bsInfo.currentState << ' '<< bsInfo.endState << ' '<< bsInfo.startTime << ' '<< bsInfo.riseTime;
	}
	return true;
}

void EnvironmentController::calculateUEContrib()
{
	EnvironmentController::averageUEStateContribution = float{ 1.0f };
	for (const auto& ue : Simulator::getUEList())
		EnvironmentController::averageUEStateContribution += ue.getDemand();

	EnvironmentController::averageUEStateContribution /= Simulator::getNumOfUsers();
	EnvironmentController::averageUEStateContribution /= Simulator::getBSMaxDR();
}

void EnvironmentController::updateCurrentStates()
{
	auto file_obj = std::ofstream{ FileIO::getProgramFP() + FileIO::getSimName() + ".EC.csv", std::ios::app };
	file_obj << Simulator::getEnvClock() << ',';
	for (auto& bsInfo : BSRegionControlInfo)
	{
		if (bsInfo.currentStatus == BSstatus::failure)
		{
			bsInfo.currentState = 0.0f; //(float)BaseStationList[bsInfo.bsID].getDataRate() / (float)Simulator::getBSMaxDR();
		}
		else
		{
			auto totalDemand = float{ 0.0f };
			for (const auto& ue : bsInfo.UEsInRegion)
				totalDemand += Simulator::getUE(ue).getDemand();
			bsInfo.currentState = totalDemand / Simulator::getBSMaxDR();
		}

		file_obj << bsInfo.currentState << ',';

	}
	file_obj << '\n';
	file_obj.close();
}

void EnvironmentController::channelFluctuation()
{
	if (Simulator::randF() <= Simulator::AP_ProbChannelChanges)
	{
		Simulator::incrementCurrentChannel();
		for(auto& ue : Simulator::getUEList_m())
		{
			const auto& maxDR = ue.getMaxDr();
			if (ue.getDemand() > maxDR)
				ue.setDemand(maxDR);
		}
	}
}

void EnvironmentController::DRFluctuation()
{
	if (Simulator::randF() <= Simulator::AP_ProbAllUsersDRFluctuate)
	{
		for (auto& ue : Simulator::getUEList_m())
		{
			const auto DRFluctuation = static_cast<int>((Simulator::rand() % (2 * Simulator::AP_DemandFluctuation + 1)) - Simulator::AP_DemandFluctuation );
			const auto&  maxDR = ue.getMaxDr();
			const auto&  demand = ue.getDemand();
			const auto newDemand = int{ static_cast<int>(demand) + DRFluctuation };
			if (newDemand >= 0 && maxDR >= static_cast<uint32_t>(newDemand))
				ue.setDemand(static_cast<uint32_t>(newDemand));
		}
	}
}

void EnvironmentController::UpdateUserLoc()
{
	//Simulation of users moving around on their own (stationary, walking, driving)

	// for every base station in the list
	for (const auto& BaseStation : Simulator::getBSList())
	{
		// get the user database stored within the base station
		const auto& UEDB = BaseStation.getUEDB();

		//Create temporary vector called userMobilities to hold information about UE mobility
		auto userMobilities = std::vector<std::pair<size_t, size_t>>(); //pair of <user id, mobility id>
		userMobilities.reserve(UEDB.size());

		//Add users to userMobilities
		for (const auto& ue : UEDB.readDB())
		{
			userMobilities.push_back(std::make_pair((*ue).userID, (*ue).mobilityID)); //Add user to list
		}

		//should prevent while loop from running indefinitely
		auto prevAmount = userMobilities.size();
		auto removalAttemptFailed = uint32_t{ 0 };
		//Keep looping until the list is empty
		while (userMobilities.size() > 0 && removalAttemptFailed < Simulator::AP_IRPMaxRemovalFailures)
		{
			const auto usrID = userMobilities.back().first;											//usr ID
			const auto userMobility = userMobilities.back().second;									//usr mobility ID
			const auto movingUserLoc = (*(UEDB.look_up(usrID))).loc;								//current usr location

			float mobility_distscale = 1.0f; //Default is 1 base station distance (later in the code, we don't move the user even though this value is 1)

			switch (userMobility)
			{
			case 0: //stationary
				mobility_distscale = 1.0f; //At most 1 base station distance
				break;
			case 1: //walking
				mobility_distscale = 2.0f; //At most 2 base station distances
				break;
			case 2: //driving
				mobility_distscale = 5.0f; //At most 5 base station distances
				break;
			}

			// calculate max distance the user can travel
			float maxSearchDist = mobility_distscale * Simulator::getBSRegionScalingFactor();
			if (mobility_distscale > 1.0f) //If the user is not stationary, move the user
			{
				// variables for determining which base station to move UE 
				std::vector<int> BSmobileList;
				BSmobileList.clear();

				int totalAttractiveness = 0;
				std::vector<int> AttractivenessList;
				AttractivenessList.clear();

				auto distBetweenUEandBS = float{};							// dist holder
				auto newBS_ID = size_t{ Simulator::getNumOfBSs() };		// destination to remove user to, initialized to an invalid BSID

				// iterate through the list of helper eNodeBs and compare their distance to the user to be removed
				// to determine the closest BS to offload to
				for (const auto& newBS : Simulator::getBSList())
				{
					// Location of current eNodeB
					const auto& newBSLoc = newBS.getLoc();

					// Determine the distance between the UE to be removed and base station			
					distBetweenUEandBS = sqrt(((movingUserLoc.y - newBSLoc.y) * (movingUserLoc.y - newBSLoc.y)) + ((movingUserLoc.x - newBSLoc.x) * (movingUserLoc.x - newBSLoc.x)));
					
					// For each base station in range, put their ID into a vector list
					if (distBetweenUEandBS <= maxSearchDist)
					{
						newBS_ID = newBS.getBSID(); // store BS ID to BsmobileList vector
						BSmobileList.push_back(newBS_ID);

						totalAttractiveness = totalAttractiveness + newBS.getBaseStationAttractiveness();
						AttractivenessList.push_back(newBS.getBaseStationAttractiveness());
					}
				}
				//------------------------------Choose a random base station within range------------------------------

				int BSrandNum = Simulator::rand() % totalAttractiveness;
				for (int i = 0; i < BSmobileList.size(); i++)
				{
					if (BSrandNum < AttractivenessList[i])
					{
						newBS_ID = BSmobileList[i];
						i = BSmobileList.size();
					}
					BSrandNum = BSrandNum - AttractivenessList[i];
				}
				
			
				//-----------------------------------------Update user location-----------------------------------------
				//generate random point
				const auto& radiusLimit = [](const auto& a) {return ((a < Simulator::AP_MinUserDistFromBS) ? Simulator::AP_MinUserDistFromBS : a); };
				const auto radius = float{ radiusLimit(Simulator::randF() * Simulator::getBSRegionScalingFactor()) };
				const auto phase = float{ 2.0f * (Simulator::randF() - 0.5f) * Simulator::PI };

				//location stuff
				const auto loc = Coord<float>{ static_cast<float>(radius * cos(phase)), static_cast<float>(radius * sin(phase)) };
				const auto newLoc = Coord<float>{ loc.x + Simulator::getBS(newBS_ID).getLoc().x, loc.y + Simulator::getBS(newBS_ID).getLoc().y };
				
				
				
				//Move user
				if (newBS_ID < Simulator::getNumOfBSs() 
					&& Simulator::moveUE(BaseStation.getBSID(), usrID, newLoc) )
				{
					
					prevAmount -= 1;
				}

				if (prevAmount == userMobilities.size())
				{
					removalAttemptFailed++;
					ErrorTracer::error("EnvironmentController: Error changing (x,y) location of UE \n");
				}
			}
			else  //Else, don't move them, but still update the counter
			{
				prevAmount -= 1;
			}
			//remove user from list
			userMobilities.pop_back();
		}
	}
}

void EnvironmentController::rampingState(BSFailureParams& bsfp, const int& timeRemaining)
{
	float diff = bsfp.endState - bsfp.currentState;

	if (timeRemaining > 0)
		diff /= timeRemaining;

	modifyState(bsfp, diff);
}

void EnvironmentController::restoreState(BSFailureParams& bsfp)
{
	const float diff = bsfp.endState - bsfp.currentState;
	modifyState(bsfp, diff);
}

void EnvironmentController::modifyState(BSFailureParams& bsfp, const float& diff)
{
	float remainingDiff = diff;

	auto positiveSign = bool{ remainingDiff >= 0.0f };
	const auto numUsers = uint32_t{ static_cast<uint32_t>(std::abs(remainingDiff) / EnvironmentController::averageUEStateContribution) };
	/*
	if (positiveSign)
		EnvironmentController::addUsers(bsfp, numUsers, remainingDiff);
	else
		EnvironmentController::removeUsers(bsfp, numUsers, remainingDiff);
	*/

	positiveSign = (remainingDiff >= 0.0f);
	const auto amtData = uint32_t{ static_cast<uint32_t>(std::abs(remainingDiff) * Simulator::getBSMaxDR()) };
	/*
	if (positiveSign)
		EnvironmentController::incrementDemands(bsfp, amtData, remainingDiff);
	else
		EnvironmentController::decrementDemands(bsfp, amtData, remainingDiff);
*/
}

void EnvironmentController::incrementDemands(BSFailureParams& bsfp, const uint32_t& amtData, float& diff)
{
	for (auto dataAdded = uint32_t{ 0 }; dataAdded < amtData; dataAdded++)
	{
		if (bsfp.UEsInRegion.size() == 0)
			return;

		const auto& user = bsfp.UEsInRegion[static_cast<size_t>(Simulator::rand() % bsfp.UEsInRegion.size())];
		if (Simulator::getUE_m(user).incrementDemand())
		{
			diff -= 1.0f / Simulator::getBSMaxDR();
		}
	}
}

void EnvironmentController::decrementDemands(BSFailureParams & bsfp, const uint32_t& amtData, float& diff)
{
	for (auto dataAdded = uint32_t{ 0 }; dataAdded < amtData; dataAdded++)
	{
		if (bsfp.UEsInRegion.size() == 0)
			return;

		const auto& user = bsfp.UEsInRegion[static_cast<size_t>(Simulator::rand() % bsfp.UEsInRegion.size())];
		if (Simulator::getUE_m(user).decrementDemand())
			diff += 1.0f / Simulator::getBSMaxDR();
	}

}

void EnvironmentController::addUsers(BSFailureParams& bsfp, const uint32_t& numUsers, float& diff)
{
	srand(time(NULL)); //generate random number seed

	for (auto users = uint32_t{ 0 }; users < numUsers; users++)
	{
		auto bsID = size_t{ bsfp.bsID };
		auto antID = size_t{ static_cast<size_t>(Simulator::rand() % Simulator::getNumberOfAntennae()) };
		const auto& bs = Simulator::getBS(bsID);

		if (bs.getAntenna(antID).getConnectionInfo().isFull())
		{
			auto stillTransAvail = bool{ false };
			for(size_t ant = 0; ant < Simulator::getNumberOfAntennae(); ant++)
				if (!bs.getAntenna(ant).getConnectionInfo().isFull())
				{
					stillTransAvail = true;
					antID = ant;
					break;
				}

			if (!stillTransAvail)
				return;
		}

		const auto& radiusLimit = [](const auto& a) {return ((a < Simulator::AP_MinUserDistFromBS) ? Simulator::AP_MinUserDistFromBS : a); };
		const auto rRadius = float{ radiusLimit(Simulator::randF() * Simulator::getBSRegionScalingFactor()) };
		const auto rPhase = float{ 2.0f * (Simulator::randF() - 0.5f) * Simulator::PI / Simulator::getNumberOfAntennae() + bs.getAntenna(antID).getAngle() * Simulator::PI / 180.0f };

		//gets the randomly selected point
		const auto loc = Coord<float>{ static_cast<float>(rRadius * cos(rPhase)), static_cast<float>(rRadius * sin(rPhase)) };

		const auto distanceSquared = float{ static_cast<float>(pow(loc.x, static_cast<int>(2)) + pow(loc.y, static_cast<int>(2))) };
		const auto SNR = float{ Simulator::generateSNR(distanceSquared) };
		auto dataRate = uint32_t{ DataRateTable::getDataRate(SNR, Simulator::getCurrentChannel()) };

		//next user to be added will have the current # of users. E.G. if there are 0 UEs then the first ID = 0.
		const auto currUserID = size_t{ Simulator::getNumOfUsers() };

		//Generate a random mobility ID for the current user [0 = Stationary, 1 = Walking, 2 = Driving (car)]
		const auto currMobilityID = (rand() % 3);

		//tranceiver set to the UE
		const auto currentTranceiver = Simulator::getBS_m(bsID).getAntenna(antID).getConnectionInfo_m().addUser(currUserID);
		if (!currentTranceiver.first)
			continue;

		auto currentDemand = uint32_t{ 0 };
		if (bsfp.endStatus == BSstatus::congestionDemand)
			currentDemand = dataRate;
		else
			currentDemand = Simulator::rand() % dataRate;

		const auto newRecord = UERecord{ currUserID, currMobilityID, Coord<float>{ loc.x + bs.getLoc().x, loc.y + bs.getLoc().y }, antID, currentTranceiver.second, SNR, currentDemand, 0, 0, 0, 0, 0, 0};
		Simulator::getBS_m(bsID).addUERecord(newRecord);

		const auto& numChan = Simulator::getNumOfChannels();
		auto ch = size_t{ 0 };
		auto possMaxDrsForUE = std::vector<uint32_t>( numChan, 0 );
		for (auto& dr : possMaxDrsForUE)
		{
			dr = DataRateTable::getDataRate(SNR, ch);
			if (dr == 0)
				ErrorTracer::error("\nINVALID DATARATE IN possMaxDrsForUE, POSSIBLE DRTBL ISSUE in EnvironmentController::addUsers(BSFailureParams& bsfp, const int& numUsers, float& diff)");
		}

		const auto userLoc = Coord<float>{ loc.x + bs.getLoc().x, loc.y + bs.getLoc().y };
		auto newUser = UserEquipment{ userLoc, currUserID, currMobilityID, possMaxDrsForUE, currentDemand };
		Simulator::addUE(newUser);

		bsfp.UEsInRegion.push_back(currUserID);
		diff -= static_cast<float>(currentDemand) / static_cast<float>(Simulator::getBSMaxDR());
	}
}

void EnvironmentController::removeUsers(BSFailureParams& bsfp, const uint32_t& numUsers, float& diff)
{
	for (auto users = uint32_t{ 0 }; users < numUsers; users++)
	{
		if (bsfp.UEsInRegion.size() > 0)
		{
			const auto userID = bsfp.UEsInRegion[static_cast<size_t>(Simulator::rand() % bsfp.UEsInRegion.size())];

			auto regionSuccess = bool{ false };
			for (auto it = bsfp.UEsInRegion.begin(); it != bsfp.UEsInRegion.end(); it++)
				if ((*it) == userID)
				{
					bsfp.UEsInRegion.erase(it);
					regionSuccess = true;
					break;
				}

			auto bsSuccess = bool{ true };
			if (!Simulator::getBS_m(bsfp.bsID).removeUE(userID))
				bsSuccess = false;

			if (regionSuccess && bsSuccess)
				diff += static_cast<float>(Simulator::getUE(userID).getDemand()) / static_cast<float>(Simulator::getBSMaxDR());
		}
		else
			return;
	}
}

void EnvironmentController::ECUpdate()
{
	EnvironmentController::channelFluctuation();
	EnvironmentController::DRFluctuation();

	if ((Simulator::getEnvClock() != 0))
		if ((Simulator::getEnvClock() % (Simulator::getmobilityBufSizeInMinutes() * 60)) == 0) //Based on the how often the user wants (in minutes), move user equipment location around.
			EnvironmentController::UpdateUserLoc();
	
	
	updateCurrentStates();

	for (auto& bsfp : BSRegionControlInfo)
	{
		if ((bsfp.startTime + bsfp.riseTime <= Simulator::getEnvClock()) && (bsfp.currentStatus == bsfp.endStatus))
		{
			if (
				(bsfp.currentState > bsfp.endState + Simulator::AP_StateCushion
				|| bsfp.currentState < bsfp.endState - Simulator::AP_StateCushion)
				&& bsfp.endStatus != BSstatus::failure
				)
					restoreState(bsfp);
			else
				continue;
		}
		else if (bsfp.startTime <= Simulator::getEnvClock())
		{
			if (bsfp.endStatus == BSstatus::failure)
			{
				Simulator::getBS_m(bsfp.bsID).setFailedTrue();
				bsfp.currentStatus = BSstatus::failure;
				bsfp.currentState = 0.0f;
			}
			else
			{
				rampingState(bsfp, bsfp.startTime + bsfp.riseTime - Simulator::getEnvClock());
				if (bsfp.startTime + bsfp.riseTime <= Simulator::getEnvClock())
				{
					bsfp.currentStatus = bsfp.endStatus;

					//if this is hit it means the EC could not induce the end state
					//therefore the end state is set to whatever was acheivable, 
					//minus a little bit of cushion to allow for fluctuation
					if (bsfp.currentState > bsfp.endState + Simulator::AP_StateCushion
						|| bsfp.currentState < bsfp.endState - Simulator::AP_StateCushion)
						bsfp.endState = bsfp.currentState - Simulator::AP_StateCushion;
				}
			}
		}

	}
}

//FLAG
const BSstatus& EnvironmentController::getCurrentBSStatus( const size_t& bs)
{
	return BSRegionControlInfo[bs].currentStatus;
}

//FLAG
const BSstatus& EnvironmentController::getEndBSStatus(const size_t& bs)
{
	return BSRegionControlInfo[bs].endStatus;
}