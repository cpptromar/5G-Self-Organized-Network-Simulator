#pragma once
#include <vector>
#include <cstdint>

struct UELogData 
{
public:
	uint32_t TIME;
	size_t BS_ID;
	float BS_LOC_X;
	float BS_LOC_Y;
	size_t ANT_ID;
	float ANT_SEC;
	size_t TRX_ID;
	float TRX_X;
	float TRX_Y;
	float TRX_ANG;
	size_t UE_ID;
	float UE_LOC_X;
	float UE_LOC_Y;
	uint32_t MAX_DR;
	uint32_t DEMAND_DR;
	uint32_t REAL_DR;
	float REC_SNR;
	float TRANS_SNR;

	UELogData() = default;
	UELogData(const UELogData&) = default;
	UELogData& operator=(const UELogData&) = default;
	UELogData(UELogData&&) = default;
	UELogData(uint32_t, size_t, float, float, size_t, float, size_t, float, float, float, size_t, float, float, uint32_t, uint32_t, uint32_t, float,float);
	~UELogData() = default;
};

typedef std::vector<UELogData> TickData;
