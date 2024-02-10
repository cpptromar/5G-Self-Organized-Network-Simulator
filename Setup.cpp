#include <iostream>
#include <fstream>
#include <sstream>

#include "Simulator.h"
#include "EnvironmentInitialization.h"
#include "Setup.h"
#include "FileIO.h"
#include "GUIDataContainer.h"
#include "DataRateTable.h"
#include "ErrorTracer.h"


bool Setup::GUIentryPoint()
{
	ErrorTracer::ErrorTracerInit();
	//Global string programPath set to the first string: argv[0]. 
	//This string is the directory the executable lies in.

	FileIO::setProgramFP(GUIDataContainer::argv[0]);

	if (!FileIO::verifyDRTBLExistence())
		ErrorTracer::error("\nSetup::verifyDRTBLExistence() failed in Setup::GUIentryPoint()");

	if (!DataRateTable::loadDRTBL())
		return ErrorTracer::error("\nDataRateTable::loadDRTBL() failed in Setup::GUIentryPoint()");

	if (!Simulator::SimulatorInit())
		return ErrorTracer::error("\nSimulator::SimulatorInit() failed in Setup::GUIentryPoint()");

	//after environment is generated, the program saves it to a file
	if (!FileIO::writeInitalSimulationState(""))
		return ErrorTracer::error("\nSetup::saveToFile() failed in Setup::GUIentryPoint()");

	if (!Simulator::runSimulation()) 
		return ErrorTracer::error("\nSimulator::runSimulation() failed in Setup::GUIentryPoint()");

	return true;
}

bool Setup::DEBUGentryPoint(int argc, char** argv)
{
	FileIO::setFPSlash('\\');

	if (!createTestGUIData(argc, argv))
		return false;

	Setup::GUIentryPoint();
	return ErrorTracer::programExit();
}

bool Setup::createTestGUIData(int argc, char** argv)
{
	std::string input;
	
	//Outputs provided string argument.
	std::cout << "Enter save name" << std::endl;

	//The user gets a certain number of tries to input correct values., char* str
	int tries = Simulator::AP_userTriesPerInput;

	//Loop repeats while there are tries remaining...
	//and std::cin.fail() returns true (indicating input failure).
	do
	{
		std::cin >> input;	//gets user input
		std::cin.clear();					//clears input stream
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (std::cin.fail() && tries == 0)
			return false;
	} while (std::cin.fail() && (--tries) > 0);


	GUIDataContainer::argc = argc;
	GUIDataContainer::argv = argv;

	GUIDataContainer::count = 7;

	GUIDataContainer::bsLen = 15;
	GUIDataContainer::antNum = 3;
	GUIDataContainer::transNum = 1000;
	GUIDataContainer::uePerAnt = 15;
	GUIDataContainer::simLen = 1000;
	GUIDataContainer::simNum = 1;
	GUIDataContainer::simStartNum = 0;
	GUIDataContainer::simName = input;
	GUIDataContainer::bufSizeInSeconds = 10;
	GUIDataContainer::congestionState = 90;
	GUIDataContainer::alertState = 80;


	GUIDataContainer::status.push_back(1);
	GUIDataContainer::status.push_back(0);
	GUIDataContainer::status.push_back(0);
	GUIDataContainer::status.push_back(3);
	GUIDataContainer::status.push_back(0);
	GUIDataContainer::status.push_back(0);
	GUIDataContainer::status.push_back(2);

	GUIDataContainer::neighbors[0].push_back(std::pair<int, int>(1, 1));
	GUIDataContainer::neighbors[0].push_back(std::pair<int, int>(5, 3));
	GUIDataContainer::neighbors[0].push_back(std::pair<int, int>(6, 2));

	GUIDataContainer::neighbors[1].push_back(std::pair<int, int>(0, 4));
	GUIDataContainer::neighbors[1].push_back(std::pair<int, int>(2, 2));
	GUIDataContainer::neighbors[1].push_back(std::pair<int, int>(6, 3));

	GUIDataContainer::neighbors[2].push_back(std::pair<int, int>(1, 5));
	GUIDataContainer::neighbors[2].push_back(std::pair<int, int>(3, 3));
	GUIDataContainer::neighbors[2].push_back(std::pair<int, int>(6, 4));

	GUIDataContainer::neighbors[3].push_back(std::pair<int, int>(2, 6));
	GUIDataContainer::neighbors[3].push_back(std::pair<int, int>(4, 4));
	GUIDataContainer::neighbors[3].push_back(std::pair<int, int>(6, 5));

	GUIDataContainer::neighbors[4].push_back(std::pair<int, int>(3, 1));
	GUIDataContainer::neighbors[4].push_back(std::pair<int, int>(5, 5));
	GUIDataContainer::neighbors[4].push_back(std::pair<int, int>(6, 6));

	GUIDataContainer::neighbors[5].push_back(std::pair<int, int>(0, 6));
	GUIDataContainer::neighbors[5].push_back(std::pair<int, int>(4, 2));
	GUIDataContainer::neighbors[5].push_back(std::pair<int, int>(6, 1));

	GUIDataContainer::neighbors[6].push_back(std::pair<int, int>(0, 5));
	GUIDataContainer::neighbors[6].push_back(std::pair<int, int>(1, 6));
	GUIDataContainer::neighbors[6].push_back(std::pair<int, int>(2, 1));
	GUIDataContainer::neighbors[6].push_back(std::pair<int, int>(3, 2));
	GUIDataContainer::neighbors[6].push_back(std::pair<int, int>(4, 3));
	GUIDataContainer::neighbors[6].push_back(std::pair<int, int>(5, 4));

	return true;
}

