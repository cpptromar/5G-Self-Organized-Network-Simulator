#pragma once
#include <cstdint>
#include <cstddef>
#include "Coord.h"
#include <vector>


class UserEquipment
{
protected:
	size_t userID;
	size_t mobilityID;
	Coord<float> loc;
	uint32_t dataDemanded;
	uint32_t dataReceived;
	float powerReceived;
	std::vector<uint32_t> possibleMaxDrs;

	static inline float calculateReceivedPower(const float& powerTransmitted, const float& pathLossAlpha) {
		return powerTransmitted / pathLossAlpha;
	}

public:
	UserEquipment(const Coord<float>& loc, const size_t& ID, const size_t& m, std::vector<uint32_t>& possMaxDrs, const uint32_t& initDemand);
	const size_t& getUserID() const;
	const size_t& getMobilityID() const;
	const Coord<float>& getLoc() const;
	const uint32_t& getDemand() const;
	const uint32_t& getMaxDr(size_t channel) const;
	const uint32_t& getMaxDr() const;
	const float& getRecPwr() const;
	const uint32_t& getRecDR() const;
	void setDemand(const uint32_t&);
	void setLoc(const Coord<float>&);
	bool incrementDemand();
	bool decrementDemand();
	void receive(const uint32_t&, const float&, const float& pathLossAlpha);
	bool Update();
};
