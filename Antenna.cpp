#include "Antenna.h"
#include <utility>
#include <cstddef>
// Here is test
Antenna::Antenna(const size_t& antid, const float& angle, const Coord<float>& loc)
{
	this->antID = antid;
	this->loc = loc;
	this->antAngle = angle;
	this->connectionInfo = TransceiverList();
}

Antenna::Antenna(const Antenna& ant)
{
	this->antID = ant.antID;
	this->loc = Coord<float>(ant.loc);
	this->antAngle = ant.antAngle;
	this->connectionInfo = TransceiverList(ant.connectionInfo);
}

Antenna::Antenna(Antenna&& ant) noexcept
{
	this->antID = std::exchange(ant.antID, 0);
	this->loc = std::move(ant.loc);
	this->antAngle = std::exchange(ant.antAngle, 0.0f);
	this->connectionInfo = std::move(ant.connectionInfo);
}

Antenna& Antenna::operator=(Antenna&& ant) noexcept
{
	if (this != &ant)
	{
		this->antID = std::exchange(ant.antID, 0);
		this->loc = std::move(ant.loc);
		this->antAngle = std::exchange(ant.antAngle, 0.0f);
		this->connectionInfo = std::move(ant.connectionInfo);
	}
	return *this;
}

const float& Antenna::getAngle() const
{
	return this->antAngle;
}

const Coord<float>& Antenna::getLoc() const
{
	return this->loc;
}

const size_t& Antenna::getAntID() const
{
	return this->antID;
}

const TransceiverList& Antenna::getConnectionInfo() const
{
	return this->connectionInfo;
}

TransceiverList& Antenna::getConnectionInfo_m()
{
	return this->connectionInfo;
}
