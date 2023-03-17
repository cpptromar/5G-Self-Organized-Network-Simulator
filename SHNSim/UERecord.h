#pragma once
#include <cstdint>
#include <vector>
#include "Coord.h"
#include <utility>
#include <cstddef>

struct UERecord
{
	size_t userID, mobilityID, antenna, currentTransceiver;
	uint32_t demand, bitsSent;
	Coord<float> loc;
	float currentSNR, powerSent;
	
	//New KPIs
	float rsrp, rssi, rsrq;
	float ddr;
	float avth;
	float ret;
	float dist;
	int dist95;
	float trxangle;

	UERecord() = delete;
	UERecord(const size_t& uid, const size_t& mid, const Coord<float>& loc,const size_t& at, const size_t& ct, const float& cSNR, const uint32_t& d, const uint32_t& bts,
		     const float& ps, const float& rsrp, const float& rssi, const float& rsrq, const float& ddr, const float& avth, const float& ret, const float& dist, const int& dist95, const float& trxangle);
	UERecord(const UERecord&) = default;
	UERecord(UERecord&&) noexcept = default;
	UERecord& operator=(const UERecord&) = default;

	const float& getRSRP();
	const float& getDDR();
	const float& getRSRQ();
	const float& getAVTH();
	const float& getRET();
	const float& getDIST();
	const int& getDIST95();
	const float& getTRXANGLE();


};
