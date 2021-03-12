#pragma once
#include <cstdint>
#include <vector>
#include "Coord.h"
#include <utility>

struct UERecord
{
	size_t userID, mobilityID, antenna, currentTransceiver;
	uint32_t demand, bitsSent;
	Coord<float> loc;
	float currentSNR, powerSent;
	
	//New KPIs
	uint32_t rsrp, rsrq;
	float ddr;

	UERecord() = delete;
	UERecord(const size_t& uid, const size_t& mid, const Coord<float>& loc, const size_t& at, const size_t& ct, const float& cSNR, const uint32_t& d, const uint32_t& bts,
		     const float& ps, const uint32_t& rsrp, const uint32_t& rsrq, const float& ddr);
	UERecord(const UERecord&) = default;
	UERecord(UERecord&&) noexcept = default;
	UERecord& operator=(const UERecord&) = default;
};
