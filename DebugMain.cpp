#pragma once
#include "debugMain.h"
#include "Simulator.h"
#include "Setup.h"
#include <iostream>
#include "GUIDataContainer.h"

int main(int argc, char** argv)			
{		
	if (!Setup::DEBUGentryPoint(argc, argv))
		return 1;
	
	return 0;									
}		