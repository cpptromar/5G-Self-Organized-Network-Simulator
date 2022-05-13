#pragma once
#include "Setup.h"
#include <vector>

typedef struct
{
	float lowBound, highBound;
	std::vector<float> spectralEfficiencies;
} DRTBLLine;

class Setup;
class DataRateTable
{
private:
	static float SNRLowBound, SNRHighBound;
	static std::vector<DRTBLLine> table;

	friend bool Setup::GUIentryPoint();
	static bool loadDRTBL();

public:
	static uint32_t getDataRate(const float& SNR, const size_t& ch);
};
