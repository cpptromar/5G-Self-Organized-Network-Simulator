#pragma once
#include <vector>
#include <iostream>
#include <string>

// define structure that represents the list of nodes (hexagons) in the system
// as well as other parameters needed to run the simulation
struct GUIDataContainer
{
	static int argc; 
	static char** argv;

	static int count, selectedTile, highlightedSide, dotCount;
	static double sideLength, screenWidth, screenHeight, mouseX, mouseY;
	static double windowWidth, windowHeight;
	static double drAreaWidth, drAreaHeight;
 	static std::vector<std::pair<int, int>> neighbors[1000];
 	static std::vector<std::vector<double>> coords;
	static std::vector<int> path;
	static std::vector<int> status;	// 0 = healthy, 1 = congested, 2 = alt congested, 3 = down
	static std::vector<int> startTime;
	static std::vector<int> riseTime;
	static std::vector<int> endState;	

 	
	// stage 2 parameters
	static int bsLen;
	static int antNum;
	static int transNum;
	static int uePerAnt;
	static int simLen;
	static int simNum;
	static int simStartNum;
	static std::string simName;
	static int bufSizeInSeconds;
	static int congestionState;
	static int alertState;


	// analysis window variables
	static int spinnerTimeVal;
	
	//variables to help in drawing the scatter plot
	static int currentScatterPlotTime;

};
