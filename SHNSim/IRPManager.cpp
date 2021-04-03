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

void IRPManager::InitializeIRPManager()
{
	this->Buffer.setWindowSize(Simulator::getIRPBufSizeInSeconds());
	this->networkStatuses = std::vector<IRP_BSInfo>(Simulator::getNumOfBSs());
	this->percentDecrease = 0.1f;
	this->doneHealing = false;
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

		//Choose algorithm based on user input
		switch (Simulator::getalgorithmVer()) {
		case 0:
			IRPManager::offloadUser();
			break;
		case 1:
			IRPManager::offloadUserKPIs(); //Run ours?
			break;
		default:
			ErrorTracer::error("IRPManager::IRPManagerUpdate(): Error choosing algorithm");
			break;
		}
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
						usr.getRecPwr(),
						// New KPIs
						(*uer).rsrp,
						(*uer).rssi,
						(*uer).rsrq,
						(*uer).ddr
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

//New algorithm based on BaseStation Status and new KPIs
void IRPManager::offloadUserKPIs() {
	// Heal the network first, then optimize afterwards
	float maxSearchDist = 4.5f * Simulator::getBSRegionScalingFactor();											// Calculate max distance to search for

	// Healing
	if (this->doneHealing == false) {
		// Perform healing on every unhealthy BaseStation

		float allBSAmountsToRemove = { 0.0f };																	//Create a percentage to check if the basestations are done healing

		for (const auto& unhealthyBS : disabledBSs) {
			//Offload users from failing BaseStation first, then congested BaseStations second
			float amountToRemove = { 0.0f };																	//Create a percentage float to keep offloading users until the BS is uncongested

			if (this->networkStatuses.at(unhealthyBS).bsStatus == IRP_BSStatus::failure)						//If BS is failing, offload all users
				amountToRemove = float{ this->networkStatuses.at(unhealthyBS).bsStateDemand * percentDecrease };
			else																								//BS is congested, so offload until uncongested
				amountToRemove = float{ (this->networkStatuses.at(unhealthyBS).bsStateDemand - Simulator::getDefaultNormalState()) * percentDecrease };

			//Checking if all basestations are done healing
			allBSAmountsToRemove += amountToRemove;																//Add current BaseStation's amountToRemove to the total

			//If the BaseStation needs users to be offloaded,
			
			//1. Collect user information once
			const UEDataBase& disabledBsRecords = Simulator::getBS(unhealthyBS).getUEDB();						//Get UEDataBase from the current BaseStation
			auto userDemands = std::vector<std::pair<size_t, uint32_t>>();										//Make vector pair
			userDemands.reserve(disabledBsRecords.size());														//Reserve memory for the vector
			for (const auto& ue : disabledBsRecords.readDB())													//For each UE in the DataBase found earlier,
				userDemands.push_back(std::make_pair((*ue).userID, (*ue).demand));								//Add their ID and demand to the vector pair

			//2. Sort User Equipment from lowest demand to highest. This will let us prioritize users with higher demand first
			std::sort(userDemands.begin(), userDemands.end(),													//Sort from beginning of vector pair to the end
				[](const std::pair<size_t, uint32_t>& lhs, const std::pair<size_t, uint32_t>& rhs) {			//Based on their demand
					return lhs.second < rhs.second;																//Refer to std::sort
				});

			//3. Loop until BaseStation is uncongested 
			
			//Error checking
			float prevAmountToRemove = amountToRemove;
			uint32_t removalAttemptFailed = { 0 };
			
			//Start loop
			while (amountToRemove > 0 && removalAttemptFailed < Simulator::AP_IRPMaxRemovalFailures)			//Try to offload users until max failures is reached
			{
				//Error checking
				if (disabledBsRecords.size() < 1)																//If the disabledBsRecords is empty
				{
					removalAttemptFailed++;																		//Add to failures
					continue;																					//Skip trying to offload user
				}
				
				//3a. Get information about the user with the highest demand (will be last on the vector list)
				const size_t usrID = userDemands.back().first;													//The UserID of the user
				const uint32_t userDemand = userDemands.back().second;											//The Demand of that user
				const Coord<float> offloadUserLoc = (*(disabledBsRecords.look_up(usrID))).loc;					//The Location of that user
				userDemands.pop_back();																			//Remove them from the vector list

				//3b. Calculate which BaseStation is the closest to offload the UE to
				float minDistBetweenUEandBS = { 100000 };														//Will be used later to keep track of the closest BaseStation
				float distBetweenUEandBS = {};																	//Actual distance between UE and BS
				size_t closestBS_ID = { Simulator::getNumOfBSs() };												//BaseStation ID number of the closest BaseStation

				//3b1. Compare the distance between each helper BaseStation and the current User Equipment (using iterators)
				for (const auto& hbs : this->helperBSs)															//For each helper BaseStation in the helperBS vector list (created on IRPManager.h)
				{
					const Coord<float>& helperBSLoc = Simulator::getBS(hbs).getLoc();							//Get location of the helper BS
					
					//Distance formula = sqrt((y2-y1)^2 + (x2-x1)^2)											//Calculate distance between UE and BS
					distBetweenUEandBS = sqrt(((offloadUserLoc.y - helperBSLoc.y) * (offloadUserLoc.y - helperBSLoc.y))
											+ ((offloadUserLoc.x - helperBSLoc.x) * (offloadUserLoc.x - helperBSLoc.x)));
					
					//If the helper BaseStation is within searching distance && it is closer than the previously found distance,
					if (distBetweenUEandBS <= maxSearchDist && distBetweenUEandBS < minDistBetweenUEandBS)
					{
						closestBS_ID = hbs;																		//Store the ID of that helper BS
						minDistBetweenUEandBS = distBetweenUEandBS;												//Store the distance between that helper BS and the current UE
					}
				}

				//Exiting this loop
				//3b2. Add the user to the to the closest BS and remove it from the original BS where it is at
				if (closestBS_ID < Simulator::getNumOfBSs() &&													//Check if the BSID is valid
					Simulator::transferUE(unhealthyBS, usrID, closestBS_ID, 0)) {								//Call transferUE
					amountToRemove -= static_cast<float>(userDemand) / Simulator::getBSMaxDR();					//Modify amountToRemove
				}

				if (amountToRemove == prevAmountToRemove) {	 													//If transferUE didn't work,
					removalAttemptFailed++;																		//Add 1 to amount of removal attempts
					ErrorTracer::error("IRPManager::offloadUserKPIs(): Error transferring UE to new eNodeB");	//Use an ErrorTracer to notify the simulator via the console window
				}
			} //End amountToRemove loop
		} //End BaseStation loop

		//Checking if all basestations are done healing
		if (allBSAmountsToRemove <= 0) {																		//If all BaseStations are done healing (no more amount to remove)
			this->doneHealing = true;																			//Set doneHealing to true so that optimization will occur next time
		}

	} //End Healing
	
	// Optimization
	else { //doneHealing == true, start optimizing
		//Get BaseStations
		
		//Group both helper base stations and congested base stations
		std::vector<size_t> OptimizingBSs;		// store BSs that need to be optimized (healthy and congested)

		for (const auto& bss : IRPManager::networkStatuses) {
			if (bss.bsStatus == IRP_BSStatus::normal && bss.bsStateDemand <= Simulator::getAlertState())		//If the BaseStation is healthy
				OptimizingBSs.push_back(bss.bsID);
			if (bss.bsStatus == IRP_BSStatus::congestion)														//Or if the BaseStation is congested
				OptimizingBSs.push_back(bss.bsID);

			//Don't check failing BaseStations because all users will be offloaded by the time optimization is occurring,
			//therefore, having an empty UEDB with no users to optimize
		}

		for (const size_t& CurrBS_ID : OptimizingBSs) {															//For each BaseStation (non-failing only since all the users will be offloaded already)
			
			//1. Collect user information and make a vector list (RSRPUser)
			const UEDataBase& disabledBsRecords = Simulator::getBS(CurrBS_ID).getUEDB();						//Get UEDataBase from the current BaseStation
			
			auto RSRPUser = std::vector<std::pair<size_t, float>>();											//Make vector pair
			RSRPUser.reserve(disabledBsRecords.size());															//Reserve memory for the vector
			for (const auto& uer : disabledBsRecords.readDB())													//For each UE in the DataBase found earlier,
				RSRPUser.push_back(std::make_pair((*uer).userID, (*uer).getRSRP()));							//Add their ID and RSRP to the vector pair

			//2. Sort User Equipment from worst RSRP (front) to best RSRP (back). This will let us prioritize users furthest away from the BaseStation first
			std::sort(RSRPUser.begin(), RSRPUser.end(),															//Sort from beginning of vector pair to the end
				[](const std::pair<size_t, float>& lhs, const std::pair<size_t, float>& rhs) {					//Based on their RSRP
					return lhs.second < rhs.second;																//Refer to std::sort
				});

			float RSRPThreshold = Simulator::getRSRPThreshold();												//Get RSRP Threshold from Simulator

			//3. Clean up RSRPUser vector list so that only the users outside of the threshold remain
			while (RSRPUser.back().second > RSRPThreshold && RSRPUser.size() > 0) {							//Compare best RSRP to Threshold and if list is empty
				RSRPUser.erase(RSRPUser.end());																//Remove any users from the vector list that don't need to be transferred (good RSRP value)
			}

			//4. Keep transferring users until the vector list is empty (all users outside of RSRP threshold are transferred)
			while (RSRPUser.size() > 0) {																	//Keep removing users from vector list until empty
				//4a. Gather User information
				const size_t usrID = RSRPUser.back().first;													//The User ID of this user
																											//CurrBS_ID is the BS ID of this user
				const Coord<float> offloadUserLoc = (*(disabledBsRecords.look_up(usrID))).loc;				//The Location of this user

				//4b. Calculate which BaseStation is the closest to offload the UE to
				float minDistBetweenUEandBS = { 100000 };													//Will be used later to keep track of the closest BaseStation
				float distBetweenUEandBS = {};																//Actual distance between UE and BS
				size_t closestBS_ID = { Simulator::getNumOfBSs() };											//BaseStation ID number of the closest BaseStation
				RSRPUser.pop_back();																		//Remove user from sorted RSRP vector list

				//4c. Compare the distance between each helper BaseStation and the current User Equipment (using iterators)
				for (const auto& hbs : this->helperBSs)														//For each helper BaseStation in the helperBS vector list (created on IRPManager.h)
				{
					const Coord<float>& helperBSLoc = Simulator::getBS(hbs).getLoc();						//Get location of the helper BS

					//Distance formula = sqrt((y2-y1)^2 + (x2-x1)^2)										//Calculate distance between UE and BS
					distBetweenUEandBS = sqrt(((offloadUserLoc.y - helperBSLoc.y) * (offloadUserLoc.y - helperBSLoc.y))
						+ ((offloadUserLoc.x - helperBSLoc.x) * (offloadUserLoc.x - helperBSLoc.x)));

					//If the helper BaseStation is within searching distance && it is closer than the previously found distance,
					if (distBetweenUEandBS <= maxSearchDist && distBetweenUEandBS < minDistBetweenUEandBS)
					{
						closestBS_ID = hbs;																	//Store the ID of that helper BS
						minDistBetweenUEandBS = distBetweenUEandBS;											//Store the distance between that helper BS and the current UE
					}
				}

				//4d. Add the user to the to the closest BS and remove it from the original BS where it is at
				Simulator::transferUE(CurrBS_ID, usrID, closestBS_ID, 0);									//Transfer UE
			}//end of transferring all UEs outside of RSRP threshold range
		}//Exit BaseStation Loop
	}//Exit optimization section
}