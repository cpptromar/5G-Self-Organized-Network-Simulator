#pragma once
#include <string>
#include <limits>	// used to include numeric_limits function
#include "Setup.h"
#include <limits>

class Setup;
class ErrorTracer
{
private:
	static bool errorOccurred;
	static std::string errorMessage;
	static void ErrorTracerInit();
	friend bool Setup::GUIentryPoint();
public:
	static bool error(const std::string& messageAddition);
	static bool programExit();
};
