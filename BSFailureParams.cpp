#include "BSFailureParams.h"

BSFailureParams::BSFailureParams()
{
	this->bsID = size_t{ 0 };
	this->currentState = float{ 0.0f };
	this->endState = float{ 0.0f };
	this->currentStatus = BSstatus::normal;
	this->endStatus = BSstatus::normal;
	this->startTime = uint32_t{ 0 };
	this->riseTime = uint32_t{ 0 };
	this->UEsInRegion = std::vector<size_t>();
}

BSFailureParams::BSFailureParams(const size_t& bsid, const float& currstate, const float& endstate, const BSstatus& currstatus, const BSstatus& endstatus, const uint32_t& starttime, const uint32_t& risetime, const std::vector<size_t> users)
{
	this->bsID = bsid;
	this->currentState = currstate;
	this->endState = endstate;
	this->currentStatus = currstatus;
	this->endStatus = endstatus;
	this->startTime = starttime;
	this->riseTime = risetime;
	this->UEsInRegion = std::vector<size_t>{ users };
}
  
BSFailureParams::BSFailureParams(const BSFailureParams& BSFP)
{
	this->bsID = BSFP.bsID;
	this->currentState = BSFP.currentState;
	this->endState = BSFP.endState;
	this->currentStatus = BSFP.currentStatus;
	this->endStatus = BSFP.endStatus;
	this->startTime = BSFP.startTime;
	this->riseTime = BSFP.riseTime;
	this->UEsInRegion = std::vector<size_t>{ BSFP.UEsInRegion };
}