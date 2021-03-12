#include "UERecord.h"

UERecord::UERecord(const size_t& uid, const size_t& mid, const Coord<float>& loc, const size_t& at, const size_t& ct, const float& cSNR, const uint32_t& d, const uint32_t& bts, const float& ps,
				   const uint32_t& rsrp, const uint32_t& rsrq, const float& ddr)
{
	this->userID = uid;
	this->mobilityID = mid;
	this->loc = Coord<float>(loc);
	this->antenna = at;
	this->currentTransceiver = ct;
	this->currentSNR = cSNR;
	this->demand = d;
	this->bitsSent = bts;
	this->powerSent = ps;
	this->rsrp = rsrp;
	this->rsrq = rsrq;
	this->ddr = ddr;
}