#include "ErrorTracer.h"
#include <iostream>
#include <fstream>


bool ErrorTracer::errorOccurred;
std::string ErrorTracer::errorMessage;

void ErrorTracer::ErrorTracerInit()
{
	ErrorTracer::errorOccurred = false;
	ErrorTracer::errorMessage = std::string{};
}

bool ErrorTracer::error(const std::string& messageAddition)
{
	if (!errorOccurred)
		errorOccurred = true;
	ErrorTracer::errorMessage += messageAddition;
	return false;
}

bool ErrorTracer::programExit()
{
	if (ErrorTracer::errorOccurred)
	{
		std::cout << '\n' << ErrorTracer::errorMessage << std::endl << std::flush;
		std::cout << "Press ENTER to continue...";
		std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');

		auto errorlog = std::ofstream{ "ERROR.txt", std::ios::trunc };
		if (errorlog.is_open())
		{
			errorlog << '\n' << ErrorTracer::errorMessage;
			errorlog.close();
		}

		return false;
	}
	else 
		return true;
}
