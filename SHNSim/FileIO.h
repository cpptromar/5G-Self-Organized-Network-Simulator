#pragma once
#include "Simulator.h"
#include "UELogData.h"
#include <cstring>

class FileIO : private Simulator
{
private:
	static char	filePathSlash;
	static bool	splitLogFiles;
	static uint32_t	logCount;
	static uint32_t	logRowCount;
	static std::string programPath;
	static std::string dataRateTableFilePath;
	static std::string logFilePath;
	static std::string simulationSaveName;
	static uint32_t lineCounter; //used for reading line-by-line the saved CSV file

	template<typename T> inline static const char* chPtrConv(T* p) { return reinterpret_cast<const char*>(p); }
	template<typename T> inline static char* chPtrConv_m(T* p) { return reinterpret_cast<char*>(p); }

	static void setSimSaveFromFP(const std::string& fp);
	static void setDRTBLFP(const std::string& drtblFP);
	static bool fixDRTBLFile();

	static void incrementLogCount();
	static void incrementLogRowCount();
	static void resetLogRowCount();

public:
	static const uint32_t AP_MaxLogLines;
	static const std::string defaultDRTBLName;
	static const std::string DRTBLSignature;


	static const std::string& getDRTBLFP();
	static const std::string& getProgramFP();
	static const std::string getSimSaveFP();
	static const std::string& getSimName();

	static void setFPSlash(const char& fpSlash);
	static void setProgramFP(const char argZero[]);
	static void setSimulationSaveName(const std::string&);

	static void resetLog();
	static bool verifyDRTBLExistence();
	static bool writeInitalSimulationState(const std::string& addendum);
	static bool readSaveFileIntoSim();

	static bool appendLog(const uint32_t& sim);
	static bool readLog_Init(const uint32_t& sim, int &numOfVars);
	static bool readLog_NextLine(const uint32_t& simNum, std::string* varNames);
	static bool readLog_NextLine(const uint32_t& sim, float* lineData);
	
};