#include "GUIDataContainer.h"

int GUIDataContainer::argc; 
char** GUIDataContainer::argv;

int GUIDataContainer::count, GUIDataContainer::selectedTile, GUIDataContainer::highlightedSide;
double GUIDataContainer::sideLength, GUIDataContainer::screenWidth, GUIDataContainer::screenHeight, GUIDataContainer::mouseX, GUIDataContainer::mouseY;
std::vector<std::pair<int, int>> GUIDataContainer::neighbors[1000];
std::vector<std::vector<double>> GUIDataContainer::coords;
std::vector<int> GUIDataContainer::status;	// 0 = healthy, 1 = congested, 2 = alt congested, 3 = down
std::vector<int> GUIDataContainer::path;
std::vector<int> GUIDataContainer::startTime;
std::vector<int> GUIDataContainer::riseTime;
std::vector<int> GUIDataContainer::endState;	
	
// stage 2 parameters
int GUIDataContainer::bsLen = 5;
int GUIDataContainer::antNum = 3;
int GUIDataContainer::transNum = 100;
int GUIDataContainer::uePerAnt = 15;
int GUIDataContainer::simLen = 1000;
int GUIDataContainer::GUIDataContainer::simNum = 1;
int GUIDataContainer::simStartNum = 0;
std::string GUIDataContainer::simName = "TEST";
int GUIDataContainer::bufSizeInSeconds = 10;
int GUIDataContainer::congestionState = 90;
int GUIDataContainer::alertState = 80;
