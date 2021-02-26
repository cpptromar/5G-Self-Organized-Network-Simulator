#include "BaseStation.h"
#include "UserEquipment.h"
#include "Antenna.h"
#include "Simulator.h"
#include <cmath>
#include <cstdint>
#include <memory>

const float BaseStation::calculateTransmittedPower(const float& simulationBandwidth, const float& SNR_db) const
{
	//converts to watts
	auto snr_ndb = powf(10.0f, 0.1f * SNR_db);

	//calculates transmitted pwr
	return snr_ndb * Simulator::AP_N0 * Simulator::AP_SimulationBandwidth;
	
}

BaseStation::BaseStation(const size_t& i, const Coord<float>& loc, const bool failed)
{
	this->bsID = i;
	this->loc = loc;
	this->dataRate = 0;
	this->BSAntennae = std::vector<Antenna>(Simulator::getNumberOfAntennae());
	this->userRecords = UEDataBase();
	this->failed = failed;

	initTransceivers();
}

BaseStation::BaseStation(BaseStation&& rv) noexcept
{
	this->bsID = std::exchange(rv.bsID, 0);
	this->loc = std::move(rv.loc);
	this->dataRate = std::exchange(rv.dataRate, 0);
	this->failed = std::exchange(rv.failed, false);
	this->BSAntennae = std::move(rv.BSAntennae);
	this->userRecords = std::move(rv.userRecords);
	this->outgoingTransmissions = std::move(rv.outgoingTransmissions);
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
		for (auto& tr : ant.getConnectionInfo_m().getTransceivers_m())
		{
			trLoc.x = vectSlope.x * dir * dist * Simulator::getDistanceBetweenTransceivers() + antCoord.x;
			trLoc.y = vectSlope.y * dir * dist * Simulator::getDistanceBetweenTransceivers() + antCoord.y;
			tr = Transceiver{ trID++, trLoc, angle };

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
		(*UER).loc = newloc;
		return true;
	}
	else
	{
		ErrorTracer::error("BaseStation: Could not find UE Record when attempting to move user.");
		return false;
	}
}

void BaseStation::setFailedTrue()
{
	this->failed = true;
}

const size_t& BaseStation::getBSID() const
{
	return this->bsID;
}

const Coord<float>& BaseStation::getLoc() const
{
	return this->loc;
}

const bool& BaseStation::getFailed() const
{
	return this->failed;
}

const uint32_t& BaseStation::getDataRate() const
{
	return this->dataRate;
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

	if (this->failed)
	{
		
		for (auto& uer : this->userRecords.readWriteDB())
		{
			(*uer).bitsSent = 0;
			(*uer).powerSent = 0;
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
			(*uer).bitsSent = 0;
			(*uer).powerSent = 0;
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
				this->dataRate += transmission.data;
				(*UER).bitsSent = transmission.data;
				const auto& powerTransmitted = this->calculateTransmittedPower(Simulator::AP_SimulationBandwidth, (*UER).currentSNR);
				(*UER).powerSent = powerTransmitted;

				Simulator::sendUETransmission(userID, transmission.data, powerTransmitted);
			}

			this->outgoingTransmissions.erase(outgoingTransmissions.begin());
		
		}
	}	

	return true;
}
