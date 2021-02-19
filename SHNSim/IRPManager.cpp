#include "IRPManager.h"
#include "Simulator.h"
#include "Transceivers.h"
#include "Antenna.h"
#include "BaseStation.h"
#include "GUIDataContainer.h"
#include "UserEquipment.h"
#include "UELogData.h"

#include <iterator>
#include <utility>
#include <vector>
#include <iostream>
#include <cmath>
#include "FileIO.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>

void IRPManager::InitializeIRPManager()
{
	this->Buffer.setWindowSize(Simulator::getIRPBufSizeInSeconds());
	this->networkStatuses = std::vector<IRP_BSInfo>(Simulator::getNumOfBSs());
	this->percentDecrease = 0.1f;
	auto bs = size_t{ 0 };
	for (auto& bss : this->networkStatuses)
		bss = { bs++, IRP_BSStatus::normal, 0.0f, 0.0f };
}

const NetworkLogBuffer& IRPManager::getBuffer() const
{
	return this->Buffer;
}

//FLAG buffer error
void IRPManager::dataAnalysis()
{
	auto file_obj = std::ofstream{ FileIO::getProgramFP() + FileIO::getSimName() + ".IRP.csv", std::ios::app };
	file_obj << Simulator::getEnvClock() << ',';

	for (auto& bss : this->networkStatuses)
	{
		bss.bsStatus = IRP_BSStatus::normal;
		bss.bsStateDemand = 0.0f;
		bss.bsStateSent = 0.0f;
	}

	if (this->Buffer.size() < 1)
		return;

	auto totalUEDemand = std::vector<uint32_t>(this->networkStatuses.size(), 0);
	auto totalBitsSent = std::vector<uint32_t>(this->networkStatuses.size(), 0);

	for (const auto& tickData : this->Buffer)
	{
		for (const auto& ueLD : tickData)
		{
			const auto& currBS = ueLD.BS_ID;
			totalUEDemand[currBS] += ueLD.DEMAND_DR;
			totalBitsSent[currBS] += ueLD.REAL_DR;
		}
	}

	auto index = size_t{ 0 };
	for (auto& bss : this->networkStatuses)
	{
		bss.bsStateDemand = static_cast<float>(totalUEDemand[index]) / static_cast<float>(this->Buffer.size() * Simulator::getBSMaxDR());

		file_obj << bss.bsStateDemand << ',';
		bss.bsStateSent = static_cast<float>(totalBitsSent[index]) / static_cast<float>(this->Buffer.size() * Simulator::getBSMaxDR());

		if (bss.bsStateSent <= Simulator::AP_StateCushion)
			bss.bsStatus = IRP_BSStatus::failure;
		else if (bss.bsStateDemand > Simulator::getDefaultCongestionState())
			bss.bsStatus = IRP_BSStatus::congestion;
		else
			bss.bsStatus = IRP_BSStatus::normal;

		index++;
	}
	file_obj << '\n';
	file_obj.close();
}

void IRPManager::IRPManagerUpdate()
{
	IRPManager::IRPDataCollection();
	if (Simulator::getEnvClock() % Simulator::getIRPBufSizeInSeconds() == 0)
	{
		IRPManager::dataAnalysis();

		// Self-Healing functions
		IRPManager::checkStatus();
		IRPManager::offloadUser();
		IRPManager::mobileuser(); //Simulates users moving around from one base station to another
	}
}

void IRPManager::IRPDataCollection()
{
	auto td = TickData();
	td.reserve(Simulator::getNumOfUsers());
	for (const auto& bs : Simulator::getBSList())
	{
		for (const auto& ant : bs.getAntennaVec())
		{
			for (const auto& ueTrPair : ant.getConnectionInfo().getUserTransPairings())
			{
				const auto& tr = ant.getConnectionInfo().getTransceivers()[ueTrPair.second];
				const auto& usr = Simulator::getUE(ueTrPair.first);
				const auto uer = bs.getUEDB().look_up(ueTrPair.first);
				if (!uer)
				{
					ErrorTracer::error("IRPManager could not look up User Equipment but expected it to be in UEDB in IRPManager::IRPDataCollection()");
				}
				else
				{
					const auto logData = UELogData(
						Simulator::getEnvClock(),
						bs.getBSID(),
						bs.getLoc().x,
						bs.getLoc().y,
						ant.getAntID(),
						ant.getAngle(),
						ueTrPair.second,
						tr.getLoc().x,
						tr.getLoc().y,
						tr.getTheta(),
						ueTrPair.first,
						usr.getMobilityID(),
						usr.getLoc().x,
						usr.getLoc().y,
						usr.getMaxDr(),
						usr.getDemand(),
						usr.getRecDR(),
						(*uer).powerSent,
						usr.getRecPwr()
					);
					td.push_back(logData);
				}
			}
		}
	}
	this->Buffer.push_back(td);
	return;
}

void IRPManager::PRINTDEBUG()
{
	auto index = size_t{ 0 };
	for (const auto& bsStat : IRPManager::networkStatuses)
	{
		std::cout << "\nBS#: " << index << ':';
		if (bsStat.bsStatus == IRP_BSStatus::normal)
			std::cout << "normal";
		else if (bsStat.bsStatus == IRP_BSStatus::congestion)
			std::cout << "congestion";
		else if (bsStat.bsStatus == IRP_BSStatus::failure)
			std::cout << "failure";
		index++;
	}
}

// FLAG array access
// Self-Healing functions
void IRPManager::checkStatus()
{
	this->helperBSs.clear();
	this->disabledBSs.clear();
	// for each eNodeB
	for (const auto& bss : IRPManager::networkStatuses)
	{
		// if the eNodeB is below the alarm threshold, add it as a helper BS
		// otherwise add it to the disabled list
		if (bss.bsStatus == IRP_BSStatus::normal && bss.bsStateDemand <= Simulator::getAlertState())
			this->helperBSs.push_back(bss.bsID);

		// if the current eNodeB is congested, add it to the back of the disabled list
		if (bss.bsStatus == IRP_BSStatus::congestion)
			this->disabledBSs.push_back(bss.bsID);

		// if the eNodeB is failing, add it to the front of the disabled list
		if (bss.bsStatus == IRP_BSStatus::failure)
			this->disabledBSs.insert(disabledBSs.begin(), bss.bsID);
	}
}

void IRPManager::offloadUser()
{
	// calculate max distance to search for
	float maxSearchDist = 4.5f * Simulator::getBSRegionScalingFactor();

	// for each eNodeB that needs help
	for (const auto& unhealthyBS : disabledBSs)
	{
		auto amountToRemove = float{ 0.0f };
		if (this->networkStatuses.at(unhealthyBS).bsStatus == IRP_BSStatus::failure)
			//if bs is in failure then all demand must be offloaded
			amountToRemove = float{ this->networkStatuses.at(unhealthyBS).bsStateDemand * percentDecrease };
		else
			//bs is in congestion and will attempt to offload demand unti BS is at normal level
			//however, the IRPManager reevaluates this every time it's called, so it will stop classifying the BS as in need of further adjustment once
			//the BS is under the alert level
			amountToRemove = float{ (this->networkStatuses.at(unhealthyBS).bsStateDemand - Simulator::getDefaultNormalState()) * percentDecrease };

		// get the list of users stored within a disabled eNodeB
		const auto& disabledBsRecords = Simulator::getBS(unhealthyBS).getUEDB();

		//Create temporary vector to hold information about UE demand
		auto userDemands = std::vector<std::pair<size_t, uint32_t>>();
		userDemands.reserve(disabledBsRecords.size());
		for (const auto& ue : disabledBsRecords.readDB())
			userDemands.push_back(std::make_pair((*ue).userID, (*ue).demand));

		// sort UEs in increasing order of thesir demand (or desired metric)
		// handover UE with highest demand first to quicker relieve the BS
		std::sort(userDemands.begin(), userDemands.end(), [](const std::pair<size_t, uint32_t>& lhs, const std::pair<size_t, uint32_t>& rhs) {
			return lhs.second < rhs.second;
			});

		//should prevent while loop from running indefinitely
		auto prevAmountToRemove = amountToRemove;
		auto removalAttemptFailed = uint32_t{ 0 };
		while (amountToRemove > 0 && removalAttemptFailed < Simulator::AP_IRPMaxRemovalFailures)
		{
			// get the location and id of the first user within the User Equipment DataBase (UEDB) of the unhealthy eNodeB
			if (disabledBsRecords.size() < 1)
			{
				removalAttemptFailed++;
				continue;
			}
			const auto usrID = userDemands.back().first;			//usr to remove from eNodeB
			const auto userDemand = userDemands.back().second;		//usr demand
			const auto offloadUserLoc = (*(disabledBsRecords.look_up(usrID))).loc;					//usr location
			userDemands.pop_back();

			// variables for determining the closest eNodeB to the UE to be removed
			auto minDistBetweenUEandBS = float{ 100000 };				// set it to arbitrarily large number
			auto distBetweenUEandBS = float{};							// dist holder
			auto closestBS_ID = size_t{ Simulator::getNumOfBSs() };		// destination to remove user to, initialized to an invalid BSID

			// iterate through the list of helper eNodeBs and compare their distance to the user to be removed
			// to determine the closest BS to offload to
			for (const auto& hbs : this->helperBSs)
			{
				// Location of current eNodeB
				const auto& helperBSLoc = Simulator::getBS(hbs).getLoc();

				// Determine the distance between the UE to be removed and the current helper eNodeB			
				distBetweenUEandBS = sqrt(((offloadUserLoc.y - helperBSLoc.y) * (offloadUserLoc.y - helperBSLoc.y)) + ((offloadUserLoc.x - helperBSLoc.x) * (offloadUserLoc.x - helperBSLoc.x)));

				// Search for the closest eNodeB within 3x the side length of the eNodeB
				if (distBetweenUEandBS <= maxSearchDist && distBetweenUEandBS < minDistBetweenUEandBS)
				{
					closestBS_ID = hbs; // store closest BS ID to offload user too
					minDistBetweenUEandBS = distBetweenUEandBS;
				}

			}

			// Add the user to the to the closest BS and remove it from the original BS where it is at
			if (closestBS_ID < Simulator::getNumOfBSs() && Simulator::transferUE(unhealthyBS, usrID, closestBS_ID, 0))
			{
				amountToRemove -= static_cast<float>(userDemand) / Simulator::getBSMaxDR();
			}

			if (amountToRemove == prevAmountToRemove)
			{
				removalAttemptFailed++;
				ErrorTracer::error("IRP manager: Error transferring UE to new eNodeB");
			}
		}
	}
}

//Simulation of users moving around on their own (stationary, walking, driving)
void IRPManager::mobileuser()
{
	//Clear the base station vector (used to store all the base stations no matter if they are failing, healthy, congested)
	this->combinedBSs.clear();

	// for each base station
	for (const auto& bss : IRPManager::networkStatuses)
	{
		this->combinedBSs.push_back(bss.bsID);	//add to a vector called combinedBSs
	}

	// for every base station in the list
	for (const auto& BaseStations : combinedBSs)
	{
		// get the list of users stored within the base station
		const auto& combinedUEDB = Simulator::getBS(BaseStations).getUEDB();

		//Create temporary vector called userMobilities to hold information about UE mobility
		auto userMobilities = std::vector<std::pair<size_t, size_t>>(); //pair of <user id, mobility id>
		userMobilities.reserve(combinedUEDB.size());
		
		//Add users to userMobilities
		for (const auto& ue : combinedUEDB.readDB())
		{
			userMobilities.push_back(std::make_pair((*ue).userID, (*ue).mobilityID)); //Add user to list
			ErrorTracer::error("User [ID, MobilityID] in list: [" + std::to_string((*ue).userID) + ", " + std::to_string((*ue).mobilityID) + "]\n");
		}
		
		//should prevent while loop from running indefinitely
		auto prevAmount = userMobilities.size();
		auto removalAttemptFailed = uint32_t{ 0 };

		//Keep looping until the list is empty
		while (userMobilities.size() > 0 && removalAttemptFailed < Simulator::AP_IRPMaxRemovalFailures)
		{
			const auto usrID = userMobilities.back().first;											//usr ID
			const auto userMobility = userMobilities.back().second;									//usr mobility ID
			const auto movingUserLoc = (*(combinedUEDB.look_up(usrID))).loc;						//current usr location
			
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
			
			ErrorTracer::error("User mobility ID in loop: " + std::to_string(userMobility) + "\n");

			if (mobility_distscale > 1.0f) //If the user is not stationary, move the user
			{
				// variables for determining the closest eNodeB to the UE to be removed
				auto minDistBetweenUEandBS = float{ 100000 };				// set it to arbitrarily large number
				auto distBetweenUEandBS = float{};							// dist holder
				auto closestBS_ID = size_t{ Simulator::getNumOfBSs() };		// destination to remove user to, initialized to an invalid BSID

				// iterate through the list of helper eNodeBs and compare their distance to the user to be removed
				// to determine the closest BS to offload to
				for (const auto& newBS : this->combinedBSs)
				{
					// Location of current eNodeB
					const auto& newBSLoc = Simulator::getBS(newBS).getLoc();

					// Determine the distance between the UE to be removed and the current helper eNodeB			
					distBetweenUEandBS = sqrt(((movingUserLoc.y - newBSLoc.y) * (movingUserLoc.y - newBSLoc.y)) + ((movingUserLoc.x - newBSLoc.x) * (movingUserLoc.x - newBSLoc.x)));

					// Search for the closest eNodeB within 3x the side length of the eNodeB
					if (distBetweenUEandBS <= maxSearchDist && distBetweenUEandBS < minDistBetweenUEandBS)
					{
						closestBS_ID = newBS; // store closest BS ID to offload user too
						minDistBetweenUEandBS = distBetweenUEandBS;
					}
				}

				//-----------------------------------------Update user location-----------------------------------------
				//generate random point
				const auto& radiusLimit = [](const auto& a) {return ((a < Simulator::AP_MinUserDistFromBS) ? Simulator::AP_MinUserDistFromBS : a); };
				const auto radius = float{ radiusLimit(Simulator::randF() * Simulator::getBSRegionScalingFactor()) };
				const auto phase = float{ 2.0f * (Simulator::randF() - 0.5f) * Simulator::PI };

				//location stuff
				const auto loc = Coord<float>{ static_cast<float>(radius * cos(phase)), static_cast<float>(radius * sin(phase)) };
				const auto newLoc = Coord<float>{ loc.x + Simulator::getBS(closestBS_ID).getLoc().x, loc.y + Simulator::getBS(closestBS_ID).getLoc().y };

				ErrorTracer::error("\nBaseStation ID: " + std::to_string(BaseStations) + "\n");
				ErrorTracer::error("Location: [" + std::to_string(loc.x) + ", " + std::to_string(loc.y) + "] \n");
				ErrorTracer::error("New Location: [" + std::to_string(newLoc.x) + ", " + std::to_string(newLoc.y) + "] \n");

				//Move user
				if (closestBS_ID < Simulator::getNumOfBSs() && Simulator::moveUE(BaseStations, usrID, newLoc))
				{
					ErrorTracer::error("A user has been moved! \n");
					prevAmount -= 1;
				}

				if (prevAmount == userMobilities.size())
				{
					removalAttemptFailed++;
					ErrorTracer::error("IRP manager: Error changing (x,y) location of UE \n");
				}
			}			
			else  //Else, don't move them, but still update the counter
			{
				ErrorTracer::error("A user has stood still! \n");
				prevAmount -= 1;
			}
			//remove user from list
			userMobilities.pop_back();
		}
	}
}