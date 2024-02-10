#pragma once
#include <cstdint>
#include <cstdlib>

/*	The Transmission struct functions as a simple "packet" container.
 *
 *	sender: the bsID of the sending BS.
 *	destination: the userID of the receiving UE.
 *	ant: the antID of the sending Antenna
 *	tr: the transceiver in the Antenna that is linked to the UE.
 *	data: the size of the packet, unit doesn't matter.
 */
struct Transmission
{
public:
	//data members
	size_t sender;
	size_t destination;
	size_t ant, tr;
	uint32_t data;
	//constructors & destructors
	Transmission() = default;
	Transmission(const size_t& sendr, const size_t& destin, const size_t& an, const size_t& t, const uint32_t& data);
	~Transmission() = default;
};

