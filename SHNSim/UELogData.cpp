#include "UELogData.h"

UELogData::UELogData(uint32_t time, size_t bs_id, float bs_loc_x, float bs_loc_y, size_t ant_id, float ant_sec, size_t trx_id, float trx_x, float trx_y, 
	float trx_ang, size_t ue_id, size_t ue_mid, float ue_loc_x, float ue_loc_y, uint32_t max_dr, uint32_t demand_dr, uint32_t real_dr, float rec_snr, float trans_snr)
{
	this->TIME = time;
	this->BS_LOC_X = bs_loc_x;
	this->BS_LOC_Y = bs_loc_y;
	this->BS_ID = bs_id;
	this->ANT_ID = ant_id;
	this->ANT_SEC = ant_sec;
	this->TRX_ID = trx_id;
	this->TRX_X = trx_x;
	this->TRX_Y = trx_y;
	this->TRX_ANG = trx_ang;
	this->UE_ID = ue_id;
	this->UE_MID = ue_mid;
	this->UE_LOC_X = ue_loc_x;
	this->UE_LOC_Y = ue_loc_y;
	this->MAX_DR = max_dr;
	this->DEMAND_DR = demand_dr;
	this->REAL_DR = real_dr;
	this->REC_SNR = rec_snr;
	this->TRANS_SNR = trans_snr;
}
