#pragma once
#include <cstdint>
#include <vector>
#include "Coord.h"
#include <utility>

struct UERecord
{
	size_t userID, antenna, currentTransceiver;
	uint32_t demand, bitsSent;
	Coord<float> loc;
	float currentSNR, powerSent;

	UERecord() = delete;
	UERecord(const size_t& uid, const Coord<float>& loc, const size_t& at, const size_t& ct, const float& cSNR, const uint32_t& d, const uint32_t& bts, const float& ps);
	UERecord(const UERecord&) = default;
	UERecord(UERecord&&) noexcept = default;
	UERecord& operator=(const UERecord&) = default;
};
