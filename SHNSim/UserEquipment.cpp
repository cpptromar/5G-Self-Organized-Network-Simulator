#include "UserEquipment.h"
#include "Simulator.h"

UserEquipment::UserEquipment(const Coord<float>& loc, const size_t& ID, const size_t& m, std::vector<uint32_t>& possMaxDrs, const uint32_t& initDemand)
{
	this->userID = ID;
	this->mobilityID = m;
	this->loc = Coord<float>{ loc };
	this->dataDemanded = initDemand;
	this->dataReceived = 0;
	this->powerReceived = 0;
	this->possibleMaxDrs = std::move(possMaxDrs);
}

//gets USERID
const size_t& UserEquipment::getUserID() const
{
	return this->userID;
}

//gets Mobility ID
const size_t& UserEquipment::getMobilityID() const
{
	return this->mobilityID;
}

//gets Location
const Coord<float>& UserEquipment::getLoc() const
{
	return this->loc;
}

const uint32_t& UserEquipment::getDemand() const
{
	return this->dataDemanded;
}

//FLAG - MEANS POSSIBLE ARRAY BOUND EXCEED --needs proper failure containment
const uint32_t& UserEquipment::getMaxDr(size_t channel) const
{
	return this->possibleMaxDrs[channel % possibleMaxDrs.size()];
}

//FLAG - MEANS POSSIBLE ARRAY BOUND EXCEED --needs proper failure containment
const uint32_t& UserEquipment::getMaxDr() const
{
	return this->possibleMaxDrs[Simulator::getCurrentChannel()];
}

const float& UserEquipment::getRecPwr() const
{
	return this->powerReceived;
}

const uint32_t& UserEquipment::getRecDR() const
{
	return this->dataReceived;
}

void UserEquipment::setDemand(const uint32_t& demand)
{
	this->dataDemanded = demand;
}

//Set user (x,y) coordinates (used for user mobility)
void UserEquipment::setLoc(const Coord<float>& newLoc)
{
	this->loc = newLoc;
}

bool UserEquipment::incrementDemand()
{
	if (this->dataDemanded < this->getMaxDr())
	{
		this->dataDemanded++;
		return true;
	}
	else
		return false;
}

bool UserEquipment::decrementDemand()
{
	if (this->dataDemanded > 0)
	{
		this->dataDemanded--;
		return true;
	}
	else
		return false;
}

void UserEquipment::receive(const uint32_t& numBitsRec, const float& powerTransmitted, const float& pathLossAlpha)
{
	this->dataReceived = numBitsRec;
	this->powerReceived = calculateReceivedPower(powerTransmitted, pathLossAlpha);
}

bool UserEquipment::Update()
{
	this->dataReceived = 0;
	this->powerReceived = 0;
	return true;
}


