#pragma once
#include "Coord.h"
#include "Transceivers.h"
#include <cstdint>

//I just want to see how GitHub saves changes.. (This is a test) - SJ 2020-09-10-1151

/*	The Antenna class functions as a container for the transceivers that connect to 
 *	the userequipment. The antennae are generated only in the construction of the BS.
 *	
 *	antID: This represents the place in the BaseStation's BSAntennae vector that the antenna is in.
 *	loc: The exact location of the antenna in the environment
 *	antAngle: This is the angle of the antenna relatvie to the BaseStation. This is always some multiple of 2PI/#Antenna per BS.
 *	connectionInfo: This is the container for the transceivers, which can be viewed as the connections to the UEs. To see more go to TransceiverList.h. 
 */
class Antenna
{
protected:
	size_t antID;
	Coord<float> loc;
	float antAngle;
	TransceiverList connectionInfo;

public:
	//constructors & destructor
	Antenna() = default;
	Antenna(const size_t& antid, const float& angle, const Coord<float>& loc);
	Antenna(const Antenna&);
	Antenna(Antenna&&) noexcept;
	Antenna& operator=(Antenna&&) noexcept;
	~Antenna() = default;

	//getters are provided, but setters are not because
	//parameters are permanently set at the construction
	//of the BaseStation object
	const float& getAngle() const;
	const Coord<float>& getLoc() const;
	const size_t& getAntID() const;

	//allows read but not write access to info contained in the transceivers
	const TransceiverList& getConnectionInfo() const;

	//allows read write access to info contained in the transceiver not allow write access.
	//only use if absolutely needed.
	TransceiverList& getConnectionInfo_m();
};