#include "BaseStation.h"
#include "UserEquipment.h"
#include "Antenna.h"
#include "Simulator.h"
#include "EnvironmentController.h"
#include "EnvironmentInitialization.h"
#include <cmath>
#include <cstdint>
#include <memory>


const float BaseStation::calculateTransmittedPower(const float& simulationBandwidth, const float& SNR_db) const
{
	//converts to watts
	auto snr_ndb = powf(10.0f, 0.1f * SNR_db);

	//calculates transmitted pwr
	return snr_ndb * Simulator::AP_N0 * Simulator::AP_SimulationBandwidth; //
	
}

BaseStation::BaseStation(const size_t& i, const Coord<float>& loc, const bool BS_Status, uint32_t BaseStationAttractiveness, uint32_t BaseStationPopulationDensity)
{
	this->bsID = i;
	this->loc = loc;
	this->dataRate = 0;
	this->BSAntennae = std::vector<Antenna>(Simulator::getNumberOfAntennae());
	this->userRecords = UEDataBase();
	this->BS_Status = BS_Status;
	this->BaseStationAttractiveness = BaseStationAttractiveness;
	this->BaseStationPopulationDensity = BaseStationPopulationDensity;
	initTransceivers();
}

BaseStation::BaseStation(BaseStation&& rv) noexcept
{
	this->bsID = std::exchange(rv.bsID, 0);
	this->loc = std::move(rv.loc);
	this->dataRate = std::exchange(rv.dataRate, 0);
	this->BS_Status = std::exchange(rv.BS_Status, 0);
	this->BSAntennae = std::move(rv.BSAntennae);
	this->userRecords = std::move(rv.userRecords);
	this->outgoingTransmissions = std::move(rv.outgoingTransmissions);
	this->BaseStationAttractiveness = std::move(rv.BaseStationAttractiveness);
	this->BaseStationPopulationDensity = std::move(rv.BaseStationPopulationDensity);

}

void BaseStation::initTransceivers()
{
	const auto bsLoc = this->loc;
	const auto degreesToRadians = Simulator::PI / 180.0f;
	const auto degreeSeparation = 360.0f / Simulator::getNumberOfAntennae();

	auto antID = size_t{ 0 };
	for (auto& ant : this->BSAntennae)
	{
		const auto angle = degreeSeparation * antID;
		const auto antennaExactY = Simulator::AP_antennaradius * sin(degreesToRadians * angle);
		const auto antennaExactX = Simulator::AP_antennaradius * cos(degreesToRadians * angle);
		const auto antCoord = Coord<float>{ static_cast<float>(antennaExactX + bsLoc.x), static_cast<float>(antennaExactY + bsLoc.y) };
		ant = Antenna{ antID++, angle, antCoord };
		//antID++;

		const auto vectSlope = Coord<float>{ static_cast<float>(sin(degreesToRadians * angle)), static_cast<float>(-1.0f * cos(degreesToRadians * angle)) };
		auto trLoc = Coord<float>{};

		auto dist = uint32_t{ 1 };
		auto distCount = uint32_t{ 0 };
		auto dir = int{ 1 };
		auto trID = size_t{ 0 };
		auto trangle = 45.00f;
		for (auto& tr : ant.getConnectionInfo_m().getTransceivers_m())
		{
			trLoc.x = vectSlope.x * dir * dist * Simulator::getDistanceBetweenTransceivers() + antCoord.x;
			trLoc.y = vectSlope.y * dir * dist * Simulator::getDistanceBetweenTransceivers() + antCoord.y;
			if (this->getBSID() == 4)																										// if excessive uptilt, random tr angle between 60 and 90
				trangle = ((float)((Simulator::rand() % 3000) + 6000))/100;
			else if (this->getBSID() == 5)																									// if excessive downtilt, random tr angle between 0 and 30
				trangle = ((float)(Simulator::rand() % 3000))/100;
			else																												// if non affecting case, normal tr angle between 30 and 60
				trangle = ((float)((Simulator::rand() % 3000) + 3000))/100;
			tr = Transceiver{ trID++, trLoc, trangle };

			dir *= -1;
			distCount = (distCount + 1) % 2;
			dist = (distCount == 0) ? dist + 1 : dist;
		}
	}
}

const UEDataBase& BaseStation::getUEDB() const
{
	return this->userRecords;
}

void BaseStation::addUERecord(const UERecord& uer)
{
	this->userRecords.push_back(uer);
}

bool BaseStation::removeUE(const size_t& userID)
{
	const auto antenna = this->userRecords.removeUser(userID);
	if (antenna >= 0 && antenna < this->BSAntennae.size())
		return this->BSAntennae[antenna].getConnectionInfo_m().removeUser(userID);
	else
		return false;

}

bool BaseStation::addUE(const UERecord& uer, const size_t& antID)
{
	auto newUser = UERecord(uer);
	newUser.antenna = antID;
	if (newUser.antenna < this->BSAntennae.size())
	{
		const auto currTrans = this->BSAntennae[newUser.antenna].getConnectionInfo_m().addUser(newUser.userID);
		if (currTrans.first)
		{
			newUser.currentTransceiver = currTrans.second;

			const auto distanceSquared = float{ static_cast<float>(pow(uer.loc.x - this->loc.x, static_cast<int>(2)) + pow(uer.loc.y - this->loc.y, static_cast<int>(2))) };
			const auto SNR = float{ Simulator::generateSNR(distanceSquared) };
			newUser.currentSNR = SNR;

			this->userRecords.push_back(newUser);
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool BaseStation::moveUE(const size_t& ue_id, const Coord<float>& newloc)
{
	auto UER = this->userRecords.look_up_m(ue_id);
	if (UER)
	{
		if ((*UER).ret == 1)
		{
			(*UER).loc = newloc;
			return true;
		}
		else
		{
			//(*UER).ret = EnvironmentInitialization::setRetainability();
			(*UER).ret = 1;		// currently hardcoded, flips retainability back to 1 for connection reestablished

			return false;
		}

	}
	else
	{
		ErrorTracer::error("BaseStation: Could not find UE Record when attempting to move user.");
		return false;
	}
	return false;
}

void BaseStation::setFailedTrue()
{
	this->BS_Status = 1; //failure
	//BS_Status needs to be changed to an int and follow the enum values
}

void BaseStation::setFailedFalse()
{
	this->BS_Status = 0; //normal
}

void BaseStation::setBaseStationAttractiveness(uint32_t newBaseStationAttractiveness)
{
	this->BaseStationAttractiveness = newBaseStationAttractiveness;
}

void BaseStation::setBaseStationPopulationDensity(uint32_t newBaseStationPopulationDensity)
{
	this->BaseStationPopulationDensity = newBaseStationPopulationDensity;
}

const size_t& BaseStation::getBSID() const
{
	return this->bsID;
}

const Coord<float>& BaseStation::getLoc() const
{
	return this->loc;
}

const bool& BaseStation::getStatus() const
{
	return this->BS_Status;
}

const uint32_t& BaseStation::getDataRate() const
{
	return this->dataRate;
}

const uint32_t& BaseStation::getBaseStationAttractiveness() const
{
	return this->BaseStationAttractiveness;
}

const uint32_t& BaseStation::getBaseStationPopulationDensity() const
{
	return this->BaseStationPopulationDensity;
}

//FLAG --needs proper failure containment
Antenna& BaseStation::getAntenna(const size_t& ant)
{
	if (ant <= this->BSAntennae.size())
		return this->BSAntennae[ant];
	else
		return this->BSAntennae[0];
}

//FLAG --needs proper failure containment
const Antenna& BaseStation::getAntenna(const size_t& ant) const
{
	if (ant < 0 || ant >= this->BSAntennae.size())
		return this->BSAntennae[0];
	else
		return this->BSAntennae[ant];
}

const std::vector<Antenna>& BaseStation::getAntennaVec() const
{
	return this->BSAntennae;
}

bool BaseStation::Update()
{
	this->dataRate = 0;
	const auto bsLoc = this->loc;
	if (this->BS_Status) //base station status is not normal
	{
		
		for (auto& uer : this->userRecords.readWriteDB())
		{
			(*uer).currentSNR = 0;
			(*uer).bitsSent = 0;
			(*uer).powerSent = 0;
			(*uer).rsrp = 0;
			(*uer).rsrq = 0;
			(*uer).rssi = 0;
			(*uer).ddr = 0;
		}
	}
	else
	{
		//unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
		//std::default_random_engine e(seed);
		this->userRecords.shuffle();

		// iterates through each all the UE in a BaseStation
		// (BaseStations are iterated through in the main class)
		for (auto& uer : this->userRecords.readWriteDB())
		{
			(*uer).currentSNR = 0;
			(*uer).bitsSent  = 0;
			(*uer).powerSent = 0;
			
			//New KPIs (hard-coded for now)
			(*uer).rsrp = 0;
			(*uer).rsrq = 0;
			(*uer).rssi = 0;
			(*uer).ddr  = 0;
			(*uer).ddr = 1;
			

			(*uer).demand = Simulator::getUE((*uer).userID).getDemand();
			this->outgoingTransmissions.push_back(Transmission{ this->bsID, (*uer).userID, (*uer).antenna, (*uer).currentTransceiver, (*uer).demand });
		}

		while(!outgoingTransmissions.empty() && this->dataRate + (*outgoingTransmissions.begin()).data < Simulator::getBSMaxDR()) // add the packet to the outgoing buffer of appropriate antenna
		{
			const auto& transmission = *outgoingTransmissions.begin();

			const auto& userID = transmission.destination;
			auto UER = this->userRecords.look_up_m(userID);
			if (UER)
			{
				(*UER).dist = sqrt(pow((bsLoc.x - (*UER).loc.x), 2) + pow((bsLoc.y - (*UER).loc.y), 2));
				// SNR Re-Calculation
				const auto distanceSquared = float{ static_cast<float>(pow((*UER).loc.x - this->loc.x, static_cast<int>(2)) + pow((*UER).loc.y - this->loc.y, static_cast<int>(2))) }; // used to calculate SNR
				const auto SNR = float{ Simulator::generateSNR(distanceSquared) };
				(*UER).currentSNR = SNR;
				
				//NEW ADDITION *** AVERAGE THROUGHPUT CALCULATE LATER
				(*UER).avth = (float)(rand() % 100) /100;
				// Data Rate 
				this->dataRate += transmission.data;
				(*UER).bitsSent = transmission.data;

				// Power Calculation
				const auto& powerTransmitted = this->calculateTransmittedPower(Simulator::AP_SimulationBandwidth, (*UER).currentSNR);	//AP_SimulationBandwidth = 20
				(*UER).powerSent = powerTransmitted;
				
				//trx angle calculation
				float trangle;
				if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::excessiveUptilt)																										// if excessive uptilt, random tr angle between 60 and 90
					trangle = ((float)((Simulator::rand() % 3000) + 6000)) / 100;
				else if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::excessiveDowntilt)																									// if excessive downtilt, random tr angle between 0 and 30
					trangle = ((float)(Simulator::rand() % 3000)) / 100;
				else																												// if non affecting case, normal tr angle between 30 and 60
					trangle = ((float)((Simulator::rand() % 3000) + 3000)) / 100;
				(*UER).trxangle = trangle;
				// KPIs
				//The RS part = Reference Signal BUT WE DON'T ACTUALLY HAVE A REFERENCE SIGNAL. Instead, each signal is represented by an update that occurs every tick.
				//RSRP = Reference Signal Received Power
				//RSSI = Reference Signal Strength Index
				//DDR = Data Drop Rate (Bits lost) We just calculate this simply by doing bits sent - bits recieved
				//N = Number of resource blocks (100 for 20 MHz bandwidth)
				float N = 100;
				
				(*UER).rsrp = 10 * log10(powerTransmitted / Simulator::AP_PathLossAlpha) + 30;	//convert to dBm P(dBm) = 10 * log10(P(W) / 1W) + 30 , AP_PathLossAlpha hardcoded to 1.2f
				if (this->bsID == 2)
				{
					(*UER).rsrp = 10 * log10(powerTransmitted / 0.1) + 30;
				}
				//convert rsrp to recieved strength signal index (RSSI)
				(*UER).rssi = (*UER).rsrp + 10 * log10(12 * N);

				//calculate reference signal recieved quality (RSRQ)
				(*UER).rsrq = 10*log10(N) + (*UER).rsrp - (*UER).rssi;

				//Bits Dropped / Data Drop Rate
				//bits received = bits sent - rand value (max value 10)
				uint32_t bitsDropped = 0;

				if (Simulator::randF() <= Simulator::AP_ProbBitsDropped) //Depending on the probability set in Simulator, drop a certain amount of bits
				{
						//Maximum of 13 bits dropped
					if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::normal)
					{
						bitsDropped = (Simulator::rand() % 13);
					}
					else if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::congestionDemand)
					{
						bitsDropped = (Simulator::rand() % 30);
					}
					else if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::congestionUsers)
					{
						bitsDropped = (Simulator::rand() % 13);
					}
					else if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::failure)
					{
						bitsDropped = (Simulator::rand() % 100);
					}
					else
					{
						bitsDropped = (Simulator::rand() % 13);
					}
				}
				if (Simulator::randF() <= .8f)
				{
					if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::excessiveUptilt)
					{
						//for excessiveUptilt, data rates will naturally deplete as users convene closer to the BS
						//calculated based off of basestation side length, hardcoded to its default value = 10
						bitsDropped = (*UER).bitsSent - ((*UER).bitsSent * (*UER).dist / 100);

					}
					else if (EnvironmentController::getCurrentBSStatus(this->bsID) == BSstatus::excessiveDowntilt)
					{
						//for excessiveDowntilt, data rates will naturally deplete as users expand outward
						//calculated based off of basestation side length, hardcoded to its default value = 10
						bitsDropped = (*UER).bitsSent * (*UER).dist / 100;

					}
				}


				//Must convert to integer because it can be negative sometimes
				int bitsReceived = (*UER).bitsSent - bitsDropped;
				
				//If there are negative bits being sent, then fix it
				if (bitsReceived < 0)
					bitsReceived = 0;
				//Percentage of data dropped (depends on the conditions in equation above)

				if (bitsReceived == 0)
					(*UER).ddr = 0;
				else
					(*UER).ddr = ((float(bitsDropped) / float((*UER).bitsSent)) * 100);
	
				//Send bits received because we perform the calculation of data drop and whatever here
				Simulator::sendUETransmission(userID, uint32_t(bitsReceived), powerTransmitted);

				if ((*UER).dist < 5) // for now calculates DIST95, can be changed later to become more dynamic
				{
					(*UER).dist95 = 0;
				}
				else
				{
					(*UER).dist95 = 1;
				}
				//******************NEW KPIS BASED ON METHODOLOGY******************//

				/*RETAINABILITY
					Ratio of Dropped Connections to total connections
					Will work by randomizing a connection state based on good/bad retainability
					If bad, then will make the user appear invisible to the network, simulating a dropped connection
					Once connection is reestablished, the user will reappear
					Moved to EnvironmentInitialization
				*/
				


			}

			this->outgoingTransmissions.erase(outgoingTransmissions.begin());
		
		}
	}	

	return true;
}
