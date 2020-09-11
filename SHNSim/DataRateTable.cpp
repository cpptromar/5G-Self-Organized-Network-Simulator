#include <iostream>
#include <fstream>

#include "DataRateTable.h"
#include "ErrorTracer.h"
#include "FileIO.h"
#include "Simulator.h"


float DataRateTable::SNRLowBound = float{ 0.0f };
float DataRateTable::SNRHighBound = float{ 0.0f };
std::vector<DRTBLLine> DataRateTable::table = std::vector<DRTBLLine>{};

bool DataRateTable::loadDRTBL()
{
	//opens file
	auto DRTBL = std::ifstream{ FileIO::getDRTBLFP(), std::ios::in };
	
	if (!DRTBL.is_open())
		return ErrorTracer::error("\nCOULD NOT OPEN DRTBL in DataRateTable::loadDRTBL()");

	//***************************************************************************************************************

	//counts total lines in file & resets file ptr
	//***************************************************************************************************************
	auto lineNum = uint32_t{ 0 };
	auto buf = char{ 0 };
	auto containsCarriageReturn = bool{false};
	while (!DRTBL.eof())
	{
		do
		{
			DRTBL.read(&buf, 1);
			if (buf == 'r')
				containsCarriageReturn = true;
		} while (buf != '\n' && !DRTBL.eof());
		if (!DRTBL.eof()) lineNum++;
	}
	DRTBL.clear();
	DRTBL.seekg(0, std::ios_base::beg);
	//***************************************************************************************************************

	//sets DRTBL to start reading from "value lines"
	//***************************************************************************************************************
	//to hold lines from file while processing
	auto lineContainer = std::string{};

	//this skips the first 2 lines
	auto lineCount = uint32_t{ 0 };
	for (lineCount = 0; lineCount < 3; lineCount++)
		std::getline(DRTBL, lineContainer);
	//***************************************************************************************************************

	//starts processing value lines and adding them to table. 
	//***************************************************************************************************************
	auto SNRHighestBound = float{ 0.0f };
	auto SNRLowestBound = float{ 0.0f };
	auto SNRLowerBound = float{ 0.0f };
	auto SNRUpperBound = float{ 0.0f };
	auto previousSNRUpperBound = float{ 0.0f };

	auto firstIteration = bool{ true };
	auto initialized = bool{ false };
	auto numChan = Simulator::getNumOfChannels();
	do
	{
		DRTBL >> SNRLowerBound >> buf >> SNRUpperBound >> buf;

		//Ok so when I was working with this I noticed that it seemed like sometimes the
		//the text values in the csv files weren't always converted into the same float values
		//this essentially prevents that from happening.
		//The fear is that if an SNR value happened to be near one of the boundaries it might not end up falling between
		//any of the upper-lower bound pairs if one of them was corrupted. This would result in program failure
		//I have the suspicion that it's not necessary but it's a fairly harmless preventative measure.
		//***************************************************************************************************************
		if (!initialized)
			initialized = true;
		else if (previousSNRUpperBound != SNRLowerBound)
			SNRLowerBound = previousSNRUpperBound;

		previousSNRUpperBound = SNRUpperBound;
		//***************************************************************************************************************
		
		if (firstIteration)
		{
			SNRLowestBound = SNRLowerBound;
			SNRHighestBound = SNRUpperBound;
			firstIteration = false;
		}
		else
		{
			if (SNRHighestBound < SNRUpperBound)
				SNRHighestBound = SNRUpperBound;

			if (SNRLowestBound > SNRLowerBound)
				SNRLowestBound = SNRLowerBound;
		}

		auto newLine  = DRTBLLine{ SNRLowerBound, SNRUpperBound, std::vector<float>(numChan, 0) };

		for (auto& se : newLine.spectralEfficiencies)
			DRTBL >> se >> buf;

		DRTBL.putback(buf);

		DataRateTable::table.push_back(newLine);

	} while (!DRTBL.eof() && lineCount++ < lineNum);

	DataRateTable::SNRHighBound = SNRHighestBound;
	DataRateTable::SNRLowBound = SNRLowestBound;

	DRTBL.close();
	//***************************************************************************************************************
	return true;
}

//FLAG --needs proper failure containment
//also maybe shift BW multiplication to initialization phase
uint32_t DataRateTable::getDataRate(const float& SNR, const size_t& ch)
{
	for (const auto& line : DataRateTable::table)
	{
		if (SNR <= line.highBound && SNR >= line.lowBound)
			return static_cast<uint32_t>(Simulator::AP_SimulationBandwidth * line.spectralEfficiencies[ch]);
	}
	return 0;
}
