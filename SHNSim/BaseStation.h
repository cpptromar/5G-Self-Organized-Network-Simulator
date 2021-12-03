#pragma once
#include "UserEquipment.h"
#include "Transmission.h"
#include "UEDataBase.h"
#include "UERecord.h"
#include "Antenna.h"

//This enum describes the perceived status of the BS from the POV of the IRP Manager.
//This will probably be moved.
enum class BSstatus { normal = 0, congestionUsers, congestionDemand, failure };


/*	The BaseStation class function models a "cell tower". It contains a database with information about its connected users.
 *	It also has a user-specified number of Antennae, which in turn contains Transceivers that connect to the users. It also has a buffer that
 *	stores the data that is to be sent out to the UEs. This buffer allows up to twice the maximum amount of data as the BS can send out in a tick.
 *	This means that if the total demand on the BS is more than twice the maximum BS datarate, data will be dropped. Every tick the BS is guarenteed to 
 *	send out at most its maximum data rate unless it's "failed" parameter is set to true.
 *
 *	bsID: This represents the place in the Simulator's BaseStationList vector that the BS is in.
 *	loc: The exact location of the BS in the environment
 *	dataRate: This is the total data that is sent out by the BS in the last tick that occurred. It is reset to 0 before sending data out in a new tick.
 *	failed: This is by default 'false', it is only flipped to 'true' by the Environment Controller if the BS is designated to be in failure.
 *	BSAntennae: This is the container that holds the Antennae posessed by the BS, for more look at Antenna.h
 *	userRecords: This database contains valuable info about the connected UEs including their SNR, antenna, transceiver, last known datarate, location, etc. 
 *				 Check out UEDataBase.h or UERecord.h for more
 *	outgoingTransmissions: Buffer that contains all the outgoing data to be sent out to the users. Limited to 2x the max BS Datarate arbitrarily. Each Transmission contains
 *						   info regarding the recipient UE, as well as the antenna and transceiver that is "used" to send the data out, and more. See Transmission.h for more info.
 */
class BaseStation
{
private:
	size_t bsID;
	Coord<float> loc;
	uint32_t dataRate;
	bool BS_Status;
	std::vector<Antenna> BSAntennae;
	UEDataBase userRecords;
	std::vector<Transmission> outgoingTransmissions;
	uint32_t BaseStationAttractiveness;
	uint32_t BaseStationPopulationDensity;

public:
	//constructors & destructors *******************************
	BaseStation() = delete;
	BaseStation(const size_t& i, const Coord<float>& loc, const bool BS_Status, uint32_t BaseStationAttractiveness, uint32_t BaseStationPopulationDensity);

	//initTransceivers initializes the antennae and transceivers, 
	//this must be called when initializing a NEW BaseStation, which only happens in
	//BaseStation(const size_t& i, const Coord<float>& loc, const bool failed);
	void initTransceivers();

	BaseStation(const BaseStation&) = default;
	BaseStation(BaseStation&&) noexcept;
	BaseStation& operator=(const BaseStation&) = delete;
	~BaseStation() = default;
	//**********************************************************

	//Getters **************************************************
	const size_t& getBSID() const;
	const Coord<float>& getLoc() const;
	const bool& getStatus() const;
	const uint32_t& getDataRate() const;
	const uint32_t& getBaseStationAttractiveness() const;
	const uint32_t& getBaseStationPopulationDensity() const;

	//gives direct mutable access to the designated Antenna
	Antenna& getAntenna(const size_t& ant);
	//gives read access to the designated Antenna
	const Antenna& getAntenna(const size_t& ant) const;

	//allows read access to entire antenna vector for iteration purposes
	const std::vector<Antenna>& getAntennaVec() const;

	//allows read access to UE information
	const UEDataBase& getUEDB() const;
	//**********************************************************

	//This sets the BS into failure mode, only used by the EnvironmentController
	void setFailedTrue();

	void setFailedFalse();

	void setBaseStationAttractiveness(uint32_t NewBaseStationAttractiveness);

	void setBaseStationPopulationDensity(uint32_t NewBaseStationpopulationDensity);

	//adds a UERecord to the BS
	void addUERecord(const UERecord& uer);

	//completely removes the selected UE from the BS, returns false if operation fails for any reason
	//if this operation fails nothing is done
	bool removeUE(const size_t& userID);

	//attempts to add an UE to the BS; adds the UER and connects the user to a transceiver
	//if this operation fails nothing is done
	bool addUE(const UERecord& uer, const size_t& antID);

	//Allows changes to the coordinate location of the specified UE
	bool moveUE(const size_t& ue_id, const Coord<float>& newloc);

	//BS update function. This is only called by the Simulator each tick.
	bool Update();

private:
	//This is contains the formula for calculating the transmitted power based off 
	//the simulator bandwidth and SNR of the user in question. Only BSs need to use this.
	const float calculateTransmittedPower(const float& simulationBandwidth, const float& SNR) const;

};

