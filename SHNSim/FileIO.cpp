#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>

#include "ErrorTracer.h"
#include "Simulator.h"
#include "FileIO.h"
#include "UEDataBase.h"
#include "UserEquipment.h"
#include "Antenna.h"
#include "DebugMain.h"

const uint32_t FileIO::AP_MaxLogLines = static_cast<uint32_t>(1000000);
const std::string FileIO::defaultDRTBLName = std::string{ "DRTBL.csv" };
const std::string FileIO::defaultCurrentTickCSV = std::string{ "CurrentTick.csv" };
const std::string FileIO::defaultMachineLearningInputCSV = std::string{ "MachineLearningInput.csv" };
const std::string FileIO::DRTBLSignature = std::string{ "Q29ubm9yIENhcnI=" };
std::map<std::int32_t, std::int64_t> FileIO::dict_time2pos;

std::string FileIO::programPath = std::string{};
std::string FileIO::dataRateTableFilePath = std::string{};
std::string FileIO::simulationSaveName = std::string{ "ERROR" };
bool FileIO::splitLogFiles = bool{ true };
char FileIO::filePathSlash = char{ '/' };
uint32_t FileIO::logCount = uint32_t{ 0 };
uint32_t FileIO::logRowCount = uint32_t{ 0 };

uint32_t FileIO::lineCounter = uint32_t{ 0 };



const std::string& FileIO::getProgramFP()
{
	return FileIO::programPath;
}

const std::string& FileIO::getDRTBLFP()
{
	return FileIO::dataRateTableFilePath;
}

const std::string& FileIO::getCurrentTickCSVFP()
{
	return FileIO::defaultCurrentTickCSV;
}

const std::string& FileIO::getMachineLearningInputFP()
{
	return FileIO::defaultMachineLearningInputCSV;
}

const std::string FileIO::getSimSaveFP()
{
	return std::string{ FileIO::programPath + FileIO::simulationSaveName + ".env" };
}

const std::string& FileIO::getSimName()
{
	return FileIO::simulationSaveName;
}

void FileIO::setSimSaveFromFP(const std::string& fp)
{
	auto lastFileSlash = fp.find_last_of("/\\");
	auto name = fp.substr(lastFileSlash + 1);

	if (name.substr(name.size() - 4, 4) == ".env")
		name = name.substr(0, name.size() - 4);

	FileIO::simulationSaveName = name;
}

void FileIO::setFPSlash(const char& fpSlash)
{
	FileIO::filePathSlash = fpSlash;
}

void FileIO::setProgramFP(const char prgPath[])
{
	FileIO::programPath = std::string{ prgPath };

	//char back is set to the last char in string program path
	auto back = FileIO::programPath.back();

	//The loop checks to see if the last char (char back) in program path is a foward slash.
	//If (char back) is not a foward slash then the last char in program path is erased and...
	//(char back) is set once again to the new last char in programPath.
	while (back != FileIO::filePathSlash)
	{
		FileIO::programPath.erase(FileIO::programPath.size() - 1);
		back = FileIO::programPath.back();
	}
}

void FileIO::setDRTBLFP(const std::string& drtblFP)
{
	FileIO::dataRateTableFilePath = drtblFP;
}

void FileIO::incrementLogCount()
{
	FileIO::logCount++;
}

void FileIO::incrementLogRowCount()
{
	FileIO::logRowCount++;
}

void FileIO::resetLogRowCount()
{
	FileIO::logRowCount = uint32_t{ 0 };
}

void FileIO::setSimulationSaveName(const std::string& simSvName)
{
	FileIO::simulationSaveName = simSvName;
}

/*
 *	 THERE ARE HARDCODED VALUES IN HERE. EVENTUALLY WILL BE CORRECTED/MOVED
 *	 Values include "DRTBL.csv" and its file signature which should be "Q29ubm9yIENhcnI=,"
 *	 If the DRTBL does not have that signature as the first line, the file is consider invalid.
 */
bool FileIO::verifyDRTBLExistence()
{
	const auto drtblFP = std::string{ FileIO::defaultDRTBLName };

	auto drtblTest = std::ifstream{ drtblFP, std::ios::in };

	if (!drtblTest.is_open())
	{
		drtblTest.close();
		return ErrorTracer::error("\nCOULD NOT OPEN DRTBL in FileIO::verifyDRTBLExistence()");
	}
	FileIO::setDRTBLFP(drtblFP);

	auto buf = char{ 0 };
	auto containsCarriageReturn = bool{ false };
	while (!drtblTest.eof())
	{
		drtblTest.read(&buf, 1);
		if (buf == '\r')
		{
			containsCarriageReturn = true;
			break;
		}
	}
	drtblTest.close();
	if (containsCarriageReturn)
		fixDRTBLFile();

	//to hold lines from file while processing
	auto lineContainer = std::string{};

	auto DRTBL = std::ifstream{ drtblFP, std::ios::in };
	std::getline(DRTBL, lineContainer);

	//The loop checks to see if the last char (char back) in the string is a comma.
	//If it is then the last char in the string is erased and...
	//(char back) is set once again to the new last char in programPath.
	auto& back = lineContainer.back();
	while (back == ',')
	{
		lineContainer.erase(lineContainer.size() - 1);
		back = lineContainer.back();
	}

	if (lineContainer != FileIO::DRTBLSignature)
		return ErrorTracer::error("\nDRTBL SIGNATURE INVALID in Setup::verifyDRTBLExistence()");

	//The next line contains the number of channels, so we read in the line, and convert to int.
	std::getline(DRTBL, lineContainer);

	//We trim the comma off the end of the line
	lineContainer = lineContainer.substr(0, lineContainer.find_first_of(','));
	//create a string stream so we can convert the line to an int
	auto strToInt = std::stringstream{ lineContainer };
	auto nChannels = int{ 0 };
	strToInt >> nChannels;
	Simulator::setNumOfChannels(nChannels);

	DRTBL.close();
	return true;
}


bool FileIO::fixDRTBLFile()
{
	const auto drtblFP = std::string{ FileIO::programPath + FileIO::defaultDRTBLName };

	auto drtblTest = std::ifstream{ drtblFP, std::ios::in };

	if (!drtblTest.is_open())
	{
		drtblTest.close();
		return ErrorTracer::error("\nCOULD NOT OPEN DRTBL in FileIO::fixDRTBLFile()");
	}

	auto lineNum = uint32_t{ 0 };
	auto buf = char{ 0 };
	while (!drtblTest.eof())
	{
		do
		{
			drtblTest.read(&buf, 1);
		} while (buf != '\n' && !drtblTest.eof());
		if (!drtblTest.eof()) lineNum++;
	}
	drtblTest.close();

	//to hold lines from file while processing
	auto lineContainer = std::string{};
	auto fileContainer = std::vector<std::string>();
	fileContainer.reserve(lineNum);

	auto DRTBL = std::ifstream{ drtblFP, std::ios::in };

	auto containsCarriageReturn = bool{ false };
	while (!DRTBL.eof())
	{
		std::getline(DRTBL, lineContainer);
		lineContainer.erase(std::remove(lineContainer.begin(), lineContainer.end(), '\r'), lineContainer.end());
		fileContainer.push_back(lineContainer);
	}
	DRTBL.close();


	auto newDRTBL = std::ofstream{ drtblFP, std::ios::trunc };

	for (auto& str : fileContainer)
		newDRTBL << str << '\n';

	return true;
}

void FileIO::resetLog()
{
	FileIO::logCount = int{ 0 };
	FileIO::logRowCount = int{ 0 };
}

//Writes file from program state.
bool FileIO::writeInitalSimulationState(const std::string& addendum = "")
{
	//Creates ofstream object, trunc means that previous file content is discarded.
	auto file_obj = std::ofstream{ FileIO::getSimSaveFP() + addendum, std::ios::trunc | std::ios::binary };

	//Checks to make sure the object was created successfully.
	if (!file_obj)
		return ErrorTracer::error("\nCOULD NOT OPEN FILE(" + FileIO::getSimSaveFP() + addendum + ") in FileIO::writeSaveFileFromENV()");

	//Outputs plain integer global variables to file. 
	const auto& envClock = Simulator::getEnvClock();
	file_obj.write(FileIO::chPtrConv(&envClock), sizeof(envClock));

	const auto& numAnt = Simulator::getNumberOfAntennae();
	file_obj.write(FileIO::chPtrConv(&Simulator::getNumberOfAntennae()), sizeof(numAnt));

	const auto& numTrans = Simulator::getNumberOfTransceivers();
	file_obj.write(FileIO::chPtrConv(&numTrans), sizeof(numTrans));

	const auto& transDist = Simulator::getDistanceBetweenTransceivers();
	file_obj.write(FileIO::chPtrConv(&transDist), sizeof(transDist));

	const auto& BSMaxDR = Simulator::getBSMaxDR();
	file_obj.write(FileIO::chPtrConv(&BSMaxDR), sizeof(BSMaxDR));

	const auto& numChan = Simulator::getNumOfChannels();
	file_obj.write(FileIO::chPtrConv(&numChan), sizeof(numChan));

	const auto numUsers = Simulator::getNumOfUsers();
	file_obj.write(FileIO::chPtrConv(&numUsers), sizeof(numUsers));

	//Outputs values from each user(UserEquipment) to file.
	for (const auto& ue : Simulator::getUEList())
	{
		const auto& userID = ue.getUserID();
		file_obj.write(FileIO::chPtrConv(&userID), sizeof(userID));

		const auto& mobilityID = ue.getMobilityID();
		file_obj.write(FileIO::chPtrConv(&mobilityID), sizeof(mobilityID));

		const auto& x = ue.getLoc().x;
		file_obj.write(FileIO::chPtrConv(&x), sizeof(x));

		const auto& y = ue.getLoc().y;
		file_obj.write(FileIO::chPtrConv(&y), sizeof(y));

		const auto& demand = ue.getDemand();
		file_obj.write(FileIO::chPtrConv(&demand), sizeof(demand));

		for (auto ch = size_t{ 0 }; ch < Simulator::getNumOfChannels(); ch++)
		{
			const auto& DR = ue.getMaxDr(ch);
			file_obj.write(FileIO::chPtrConv(&DR), sizeof(DR));
		}
	}

	const auto numBSs = Simulator::getNumOfBSs();
	file_obj.write(FileIO::chPtrConv(&numBSs), sizeof(numBSs));
	//Outputs values from each BaseStation to file.
	for (const auto& bs : Simulator::getBSList())
	{
		const auto& x = bs.getLoc().x;
		file_obj.write(FileIO::chPtrConv(&x), sizeof(x));

		const auto& y = bs.getLoc().y;
		file_obj.write(FileIO::chPtrConv(&y), sizeof(y));

		const auto& failed = bs.getStatus();
		file_obj.write(FileIO::chPtrConv(&failed), sizeof(failed));

		const auto& BaseStationAttractiveness = bs.getBaseStationAttractiveness();
		file_obj.write(FileIO::chPtrConv(&BaseStationAttractiveness), sizeof(BaseStationAttractiveness));

		const auto& BaseStationPopulationDensity = bs.getBaseStationPopulationDensity();
		file_obj.write(FileIO::chPtrConv(&BaseStationPopulationDensity), sizeof(BaseStationPopulationDensity));

		for (const auto& ant : bs.getAntennaVec())
		{
			const auto numOfTransceiversUsed = ant.getConnectionInfo().numberOfTransceiversUsed();
			file_obj.write(FileIO::chPtrConv(&numOfTransceiversUsed), sizeof(numOfTransceiversUsed));
			for (const auto& tr : ant.getConnectionInfo().getUserTransPairings())
			{
				const auto userID = tr.first;
				file_obj.write(FileIO::chPtrConv(&userID), sizeof(userID));
			}
		}


		const auto& db = bs.getUEDB().readDB();
		const auto dbSize = db.size();
		file_obj.write(FileIO::chPtrConv(&dbSize), sizeof(dbSize));
		for (const auto& uer : db)
		{
			const auto& userID = (*uer).userID;
			file_obj.write(FileIO::chPtrConv(&userID), sizeof(userID));

			const auto& mobilityID = (*uer).mobilityID;
			file_obj.write(FileIO::chPtrConv(&mobilityID), sizeof(mobilityID));

			const auto& x = (*uer).loc.x;
			file_obj.write(FileIO::chPtrConv(&x), sizeof(x));

			const auto& y = (*uer).loc.y;
			file_obj.write(FileIO::chPtrConv(&y), sizeof(y));

			const auto& ant = (*uer).antenna;
			file_obj.write(FileIO::chPtrConv(&ant), sizeof(ant));

			const auto& tr = (*uer).currentTransceiver;
			file_obj.write(FileIO::chPtrConv(&tr), sizeof(tr));

			const auto& snr = (*uer).currentSNR;
			file_obj.write(FileIO::chPtrConv(&snr), sizeof(snr));

			const auto& demand = (*uer).demand;
			file_obj.write(FileIO::chPtrConv(&demand), sizeof(demand));

			const auto& bts = (*uer).bitsSent;
			file_obj.write(FileIO::chPtrConv(&bts), sizeof(bts));

			const auto& ps = (*uer).powerSent;
			file_obj.write(FileIO::chPtrConv(&ps), sizeof(ps));
			
			const auto& rsrp = (*uer).rsrp;
			file_obj.write(FileIO::chPtrConv(&rsrp), sizeof(rsrp));

			const auto& rssi = (*uer).rssi;
			file_obj.write(FileIO::chPtrConv(&rssi), sizeof(rssi));

			const auto& rsrq = (*uer).rsrq;
			file_obj.write(FileIO::chPtrConv(&rsrq), sizeof(rsrq));

			const auto& ddr = (*uer).ddr;
			file_obj.write(FileIO::chPtrConv(&ddr), sizeof(ddr));

			const auto& avth = (*uer).avth;
			file_obj.write(FileIO::chPtrConv(&avth), sizeof(avth));

			const auto& ret = (*uer).ret;
			file_obj.write(FileIO::chPtrConv(&ret), sizeof(ret));

			const auto& dist = (*uer).dist;
			file_obj.write(FileIO::chPtrConv(&dist), sizeof(dist));

			const auto& dist95 = (*uer).dist95;
			file_obj.write(FileIO::chPtrConv(&dist95), sizeof(dist95));

			const auto& trxangle = (*uer).trxangle;
			file_obj.write(FileIO::chPtrConv(&trxangle), sizeof(trxangle));

			
		}
	}

	//Closes file object.
	file_obj.close();
	return true;
}

//Reads file into program state.
bool FileIO::readSaveFileIntoSim()
{
	//Creates ifstream object, trunc means that previous file content is discarded.
	auto file_obj = std::ifstream{ FileIO::getSimSaveFP(), std::ios::in | std::ios::binary };
	std::cout << FileIO::getSimSaveFP() << std::endl;
	//Checks to make sure the object was created successfully.
	if (!file_obj)
		return ErrorTracer::error("\nCOULD NOT OPEN FILE(" + FileIO::getSimSaveFP() + ") in FileIO::readSaveFileToENV()");

	Simulator::resetCoreObjects();

	//Reads variables from file.
	auto eClk = uint32_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&eClk), sizeof(eClk));
	Simulator::setEnvClock(eClk);

	auto numA = size_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&numA), sizeof(numA));
	Simulator::setNumberOfAntennae(numA);

	auto numT = size_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&numT), sizeof(numT));
	Simulator::setNumberOfTransceivers(numT);

	auto distT = float{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&distT), sizeof(distT));
	Simulator::setDistanceBetweenTransceivers(distT);

	auto bsMDR = uint32_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&bsMDR), sizeof(bsMDR));
	Simulator::setBSMaxDR(bsMDR);

	auto numChan = size_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&numChan), sizeof(numChan));
	Simulator::setNumOfChannels(numChan);

	auto numUsers = size_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&numUsers), sizeof(numUsers));
	//Reads values to each user(UserEquipment) from file.
	for (auto c = size_t{ 0 }; c < numUsers; c++)
	{
		auto userID = size_t{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&userID), sizeof(userID));
		if (userID != c)
			return ErrorTracer::error("\nPOSSIBLE FILE CORRUPTION(" + std::to_string(userID) + "!=" + std::to_string(userID) + ") in FileIO::readSaveFileToENV()");

		auto mobilityID = size_t{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&mobilityID), sizeof(mobilityID));

		auto x = float{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&x), sizeof(x));

		auto y = float{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&y), sizeof(y));

		auto demand = uint32_t{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&demand), sizeof(demand));

		auto possMaxDrs = std::vector<uint32_t>(Simulator::getNumOfChannels(), uint32_t{ 0 });
		for (auto& drInChan : possMaxDrs)
			file_obj.read(FileIO::chPtrConv_m(&drInChan), sizeof(drInChan));

		auto newUser = UserEquipment{ Coord<float>{ x,y }, userID, mobilityID, possMaxDrs, demand };
		Simulator::addUE(newUser);
	}

	auto numBSs = size_t{ 0 };
	file_obj.read(FileIO::chPtrConv_m(&numBSs), sizeof(numBSs));
	//reads values for each BaseStation from file.
	for (size_t bs = 0; bs < numBSs; bs++)
	{
		auto x = float{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&x), sizeof(x));

		auto y = float{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&y), sizeof(y));

		auto failed =  false;
		file_obj.read(FileIO::chPtrConv_m(&failed), sizeof(failed));
		
		auto BaseStationAttractiveness = uint32_t{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&BaseStationAttractiveness), sizeof(BaseStationAttractiveness));

		auto BaseStationPopulationDensity = uint32_t{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&BaseStationPopulationDensity), sizeof(BaseStationPopulationDensity));
		

		auto newBS = BaseStation{ bs, Coord<float>{ x, y }, failed, BaseStationAttractiveness, BaseStationPopulationDensity};

		for (auto i = size_t{ 0 }; i < Simulator::getNumberOfAntennae(); i++)
		{
			auto numOfTransceiversUsed = size_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&numOfTransceiversUsed), sizeof(numOfTransceiversUsed));

			for (auto t = size_t{ 0 }; t < numOfTransceiversUsed; t++)
			{
				auto userID = size_t{ 0 };
				file_obj.read(FileIO::chPtrConv_m(&userID), sizeof(userID));

				if(!newBS.getAntenna(i).getConnectionInfo_m().addUser(userID).first)
					return ErrorTracer::error("\nPOSSIBLE FILE CORRUPTION, attempted to add more users to an antenna than should be possible in FileIO::readSaveFileToENV()");
			}
		}

		auto dbSize = size_t{ 0 };
		file_obj.read(FileIO::chPtrConv_m(&dbSize), sizeof(dbSize));
		for (size_t ue = 0; ue < dbSize; ue++)
		{
			auto userID = size_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&userID), sizeof(userID));
			
			auto mobilityID = size_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&mobilityID), sizeof(mobilityID));
			
			auto x = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&x), sizeof(x));

			auto y = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&y), sizeof(y));

			auto ant = size_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&ant), sizeof(ant));

			auto tr = size_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&tr), sizeof(tr));

			auto snr = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&snr), sizeof(snr));

			auto demand = uint32_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&demand), sizeof(demand));

			auto bts = uint32_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&bts), sizeof(bts));

			auto ps = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&ps), sizeof(ps));

			auto rsrp = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&rsrp), sizeof(rsrp));

			auto rssi = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&rssi), sizeof(rssi));

			auto rsrq = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&rsrq), sizeof(rsrq));

			auto ddr = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&ddr), sizeof(ddr));

			auto avth = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&avth), sizeof(avth));

			auto ret = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&ret), sizeof(ret));

			auto dist = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&dist), sizeof(dist));

			auto dist95 = uint32_t{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&dist95), sizeof(dist95));

			auto trxangle = float{ 0 };
			file_obj.read(FileIO::chPtrConv_m(&trxangle), sizeof(trxangle));

			auto newRecord = UERecord{ userID, mobilityID, Coord<float>{x, y}, ant, tr, snr, demand, bts, ps, rsrp, rssi, rsrq, ddr, avth, ret, dist, dist95, trxangle};
			newBS.addUERecord(newRecord);
		}
		Simulator::addBS(newBS);
	}

	return true;
}

bool FileIO::appendLog(const uint32_t& simNum)
{
	std::string filePath = FileIO::programPath + FileIO::simulationSaveName;

	std::string newFilePath;
	if (FileIO::splitLogFiles)
	{
		if (FileIO::logRowCount >= FileIO::AP_MaxLogLines)
		{
			FileIO::incrementLogCount();
			FileIO::logRowCount = 0;
		}
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + "_LOG_" + std::to_string(FileIO::logCount) + ".csv";
	}
	else
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + ".csv";

	auto log = std::ofstream{ newFilePath, std::ios::app };

	if (FileIO::logRowCount == 0)
	{
		log << "Time,BS_ID,BS_STATUS,BS_LOC_X,BS_LOC_Y,ANT_ID,ANT_SEC,TRX_ID,TRX_X,TRX_Y,TRX_ANG,UE_ID,UE_MID,UE_LOC_X,UE_LOC_Y,MAX_DR (bits),DEMAND_DR (bits),REAL_DR (bits),BS_Trans_PWR (W),UE_Rec_PWR (W),RSRP (dBm),RSSI (dBm),RSRQ (dB),DDR (%), SNR, AVTH, RET, DIST, DIST95\n"; // added the name for the column holding the bs number
		FileIO::incrementLogRowCount();
	}

	if (Simulator::getIRPManager().getBuffer().size() < 1)
		return ErrorTracer::error("Network Manager buffer empty when attempting to access log.");
	
	// keep track of where the different time ticks are in the log..
	FileIO::dict_time2pos.insert(std::pair<std::uint32_t, std::uint64_t>(Simulator::getEnvClock(), log.tellp()));

	// This is a ForEach loop (C++11 added foreach loop capabilities) 
	// It does the loop for each ue in Simulator::getIRPManager().getBuffer().getLastTick() - SJ 
	for (const auto& ue_ld : Simulator::getIRPManager().getBuffer().getLastTick()) 
	{
		log << ue_ld.TIME << ','
			<< ue_ld.BS_ID << ','
			<< ue_ld.BS_STATUS << ','
			<< ue_ld.BS_LOC_X << ','
			<< ue_ld.BS_LOC_Y << ','
			<< ue_ld.ANT_ID << ','
			<< ue_ld.ANT_SEC << ','
			<< ue_ld.TRX_ID << ','
			<< ue_ld.TRX_X << ','
			<< ue_ld.TRX_Y << ','
			<< ue_ld.TRXANGLE << ','
			<< ue_ld.UE_ID << ','
			<< ue_ld.UE_MID << ','
			<< ue_ld.UE_LOC_X << ','
			<< ue_ld.UE_LOC_Y << ','
			<< ue_ld.MAX_DR << ','
			<< ue_ld.DEMAND_DR << ','
			<< ue_ld.REAL_DR << ','
			<< ue_ld.TRANS_PWR << ','
			<< ue_ld.REC_PWR << ','
			<< ue_ld.RSRP << ','
			<< ue_ld.RSSI << ','
			<< ue_ld.RSRQ << ','
			<< ue_ld.DDR << ','
			<< ue_ld.SNR << ','
			<< ue_ld.AVTH << ','
			<< ue_ld.RET << ','
			<< ue_ld.DIST << ','
			<< ue_ld.DIST95 << '\n';
		if (FileIO::splitLogFiles)
		{
			FileIO::incrementLogRowCount();
		}

	}

	log.close();

	return true;
}

bool FileIO::writeCurrentTick(const uint32_t& simNum)
{

	auto CurrentTickCSV = std::ofstream{ FileIO::getCurrentTickCSVFP(), std::ios::in };
	
	CurrentTickCSV << "Time,BS_ID,BS_STATUS,BS_LOC_X,BS_LOC_Y,ANT_ID,ANT_SEC,TRX_ID,TRX_X,TRX_Y,TRX_ANG,UE_ID,UE_MID,UE_LOC_X,UE_LOC_Y,MAX_DR (bits),DEMAND_DR (bits),REAL_DR (bits),BS_Trans_PWR (W),UE_Rec_PWR (W),RSRP (dBm),RSSI (dBm),RSRQ (dB),DDR (%), SNR, AVTH, RET, DIST, DIST95\n"; // added the name for the column holding the bs number
	
	if (Simulator::getIRPManager().getBuffer().size() < 1)
		return ErrorTracer::error("Network Manager buffer empty when attempting to access log.");

	// keep track of where the different time ticks are in the log..
	int numofUsers = 0;
	for (const auto& ue_ld : Simulator::getIRPManager().getBuffer().getLastTick())
	{
		numofUsers = numofUsers + 1;
	}
	
	// It does the loop for each ue in Simulator::getIRPManager().getBuffer().getLastTick() - SJ 
	int i = 0;
	for (const auto& ue_ld : Simulator::getIRPManager().getBuffer().getLastTick())
	{
		if (i < numofUsers)
		{
			CurrentTickCSV << ue_ld.TIME << ','
				<< ue_ld.BS_ID << ','
				<< ue_ld.BS_STATUS << ','
				<< ue_ld.BS_LOC_X << ','
				<< ue_ld.BS_LOC_Y << ','
				<< ue_ld.ANT_ID << ','
				<< ue_ld.ANT_SEC << ','
				<< ue_ld.TRX_ID << ','
				<< ue_ld.TRX_X << ','
				<< ue_ld.TRX_Y << ','
				<< ue_ld.TRXANGLE << ','
				<< ue_ld.UE_ID << ','
				<< ue_ld.UE_MID << ','
				<< ue_ld.UE_LOC_X << ','
				<< ue_ld.UE_LOC_Y << ','
				<< ue_ld.MAX_DR << ','
				<< ue_ld.DEMAND_DR << ','
				<< ue_ld.REAL_DR << ','
				<< ue_ld.TRANS_PWR << ','
				<< ue_ld.REC_PWR << ','
				<< ue_ld.RSRP << ','
				<< ue_ld.RSSI << ','
				<< ue_ld.RSRQ << ','
				<< ue_ld.DDR << ','
				<< ue_ld.SNR << ','
				<< ue_ld.AVTH << ','
				<< ue_ld.RET << ','
				<< ue_ld.DIST << ','
				<< ue_ld.DIST95 << '\n';


			i = i + 1;
		}
			
	}

	CurrentTickCSV.close();

	return true;
}

//Reads the data var names (first line) and places the number of variables in numOfVars 
 bool FileIO::readLog_Init(const uint32_t& simNum, int &numOfVars) 
{
	 FileIO::lineCounter = 0;
	//https://en.cppreference.com/w/cpp/io/basic_ifstream
	//http://www.cplusplus.com/reference/ios/ios_base/openmode/

	using namespace std;
	string filePath = FileIO::programPath + FileIO::simulationSaveName;
	
	string newFilePath;
	if (FileIO::splitLogFiles)
	{
		if (FileIO::logRowCount >= FileIO::AP_MaxLogLines)
		{
			FileIO::incrementLogCount();
			FileIO::logRowCount = 0;
		}
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + "_LOG_" + std::to_string(FileIO::logCount) + ".csv";
	}
	else
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + ".csv";

	ifstream log;
	string line;
	log.open(newFilePath, std::ios::in); //open the file for readonly (ios::in)

	if (!log.is_open())
	{
		cout << "I could not open the log!!" << endl;
		cout << "filepath = " << newFilePath << endl;
		log.close();
		return ErrorTracer::error("\nCOULD NOT OPEN the CSV file");
	}

	getline(log, line); //get the first line
	istringstream iss(line); //we use stringstream so that we can PARSE (get individual variables)the line that was read
	string var;
	int varCount = 0;

	while (std::getline(iss, var, ',')) //get individual variable "cells" and store in var 
	{
		varCount++;//count how many different variables there are to create array
	}
	numOfVars = varCount;
	int counter = 0;

	return true;
}

 //this overload version is for the variable names
 bool FileIO::readLog_NextLine(const uint32_t& simNum, std::string* varNames) //This gets the number of data var names to help with creating a data array
 {
	 FileIO::lineCounter = 0;
	 //https://en.cppreference.com/w/cpp/io/basic_ifstream
	 //http://www.cplusplus.com/reference/ios/ios_base/openmode/

	 using namespace std;
	 std::string filePath = FileIO::programPath + FileIO::simulationSaveName;

	 std::string newFilePath;
	 if (FileIO::splitLogFiles)
	 {
		 if (FileIO::logRowCount >= FileIO::AP_MaxLogLines)
		 {
			 FileIO::incrementLogCount();
			 FileIO::logRowCount = 0;
		 }
		 newFilePath = filePath + "_SIM_" + std::to_string(simNum) + "_LOG_" + std::to_string(FileIO::logCount) + ".csv";
	 }
	 else
		 newFilePath = filePath + "_SIM_" + std::to_string(simNum) + ".csv";

	 std::ifstream log;
	 std::string line;
	 log.open(newFilePath, std::ios::in); //open the file for readonly (ios::in)

	 if (!log.is_open())
	 {
		 std::cout << "I could not open the log!!" << std::endl;
		 std::cout << "filepath = " << newFilePath << std::endl;
		 log.close();
		 return ErrorTracer::error("\nCOULD NOT OPEN the CSV file");
	 }

	 getline(log, line); //get the first line
	 std::istringstream iss(line); //we use stringstream so that we can PARSE (get individual variables)the line that was read
	 std::string var;

	 int counter = 0;

	 while (getline(iss, var, ',')) //store names into array
	 {
		 varNames[counter] = var;
		 counter++;
	 }
	 FileIO::lineCounter++;
	 return true;
 }

//returns true when it is the end of the file
bool FileIO::readLog_NextLine(const uint32_t& simNum, float* lineData)
{
	using namespace std;
	//https://en.cppreference.com/w/cpp/io/basic_ifstream
	//http://www.cplusplus.com/reference/ios/ios_base/openmode/

	std::string filePath = FileIO::programPath + FileIO::simulationSaveName;
	
	std::string newFilePath;
	if (FileIO::splitLogFiles)
	{
		if (FileIO::logRowCount >= FileIO::AP_MaxLogLines)
		{
			FileIO::incrementLogCount();
			FileIO::logRowCount = 0;
		}
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + "_LOG_" + std::to_string(FileIO::logCount) + ".csv";
	}
	else
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + ".csv";

	std::ifstream log;
	std::string line;
	log.open(newFilePath, std::ios::in); //open the file for readonly (ios::in)

	if (!log.is_open())
	{
		std::cout << "I could not open the log!!" << std::endl;
		std::cout << "filepath = " << newFilePath << std::endl;
		log.close();
		return ErrorTracer::error("\nCOULD NOT OPEN the CSV file");
	}

	for (int i = 0; i <= FileIO::lineCounter; i++)
	{
		
		if (i == FileIO::lineCounter) //skip over the lines that we already did until we get to the NEXT LINE and check to see whether it's not EOF
		{
			if (getline(log, line))
			{
				std::istringstream iss(line); //we use stringstream so that we can PARSE (get individual variables)the line that was read
				int counter = 0;
				string buf;
				while (getline(iss, buf, ',')) //this inputs all the separated variables into the lineData array
				{
					lineData[counter] = stof(buf); //stof() converts string to float (STOF : String TO Float)
					counter++;
				}
			}
			else //if getline(log, line) said it's the end of file (EOF)
				return true; //it is truely the EOF
			
		}
		else
		{
			if (getline(log, line)); //skip the lines which we don't want, and if false, it is the end of file
			else
				return true; //end of file has occured!
		}
			
	}

	FileIO::lineCounter++;
	return false; //not end of file
}

// read the line starting from the given position;
// returns the next position after the line is read;
// will return NULL if eof was reached
std::uint64_t FileIO::readLog_LineAtPosition(const uint32_t& simNum, float* lineData, std::uint64_t position)
{

	using namespace std;
	//https://en.cppreference.com/w/cpp/io/basic_ifstream
	//http://www.cplusplus.com/reference/ios/ios_base/openmode/

	string filePath = FileIO::programPath + FileIO::simulationSaveName;

	string newFilePath;
	if (FileIO::splitLogFiles)
	{
		if (FileIO::logRowCount >= FileIO::AP_MaxLogLines)
		{
			FileIO::incrementLogCount();
			FileIO::logRowCount = 0;
		}
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + "_LOG_" + std::to_string(FileIO::logCount) + ".csv";
	}
	else
		newFilePath = filePath + "_SIM_" + std::to_string(simNum) + ".csv";

	ifstream log;
	string line;
	uint64_t endPosition = 0;
	log.open(newFilePath, std::ios::in); //open the file for readonly (ios::in)

	if (!log.is_open())
	{
		cout << "I could not open the log!!" << std::endl;
		cout << "filepath = " << newFilePath << std::endl;
		log.close();
	}

	log.seekg(position);		//navigate to the given position

	if (getline(log, line))		//read the line
	{
		istringstream iss(line); //we use stringstream so that we can PARSE (get individual variables)the line that was read
		int counter = 0;
		string buf;
		while (getline(iss, buf, ',')) //this inputs all the separated variables into the lineData array
		{
			lineData[counter] = stof(buf); //stof() converts string to float (STOF : String TO Float)
			counter++;
		}
	}	
	else //if getline(log, line) said it's the end of file (EOF) (i.e. it returns false)
		return NULL; //return NULL if it's the EOF

	endPosition = log.tellg();	//save the ending position

	return endPosition; //return the next position after the line is read
}