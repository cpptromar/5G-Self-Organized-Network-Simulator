#include "UERecord.h"

UERecord::UERecord(const size_t& uid, const size_t& mid, const Coord<float>& loc, const size_t& at, const size_t& ct, const float& cSNR, const uint32_t& d, const uint32_t& bts, const float& ps,
				   const float& rsrp, const float& rssi, const float& rsrq, const float& ddr, const float& avth, const float& ret, const float& dist, const int& dist95, const float& trxangle)
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
	this->rssi = rssi;
	this->rsrq = rsrq;
	this->ddr = ddr;
	this->avth = avth;
	this->ret = ret;
	this->dist = dist;
	this->dist95 = dist95;
	this->trxangle = trxangle;
}
const float& UERecord::getTRXANGLE() {
	return this->trxangle;
}
const float& UERecord::getRSRP() {
	return this->rsrp;
}

const float& UERecord::getDDR() {
	return this->ddr;
}

const float& UERecord::getRSRQ() {
	return this->rsrq;
}

const float& UERecord::getAVTH() {
	return this->avth;
}

const float& UERecord::getRET() {
	return this->ret;
}

const float& UERecord::getDIST() {
	return this->dist;
}

const int& UERecord::getDIST95() {
	return this->dist95;
}


