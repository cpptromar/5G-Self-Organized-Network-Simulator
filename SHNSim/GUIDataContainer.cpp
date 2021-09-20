#include "GUIDataContainer.h"

int GUIDataContainer::argc; 
char** GUIDataContainer::argv;

int GUIDataContainer::count, GUIDataContainer::selectedTile, GUIDataContainer::highlightedSide;
double GUIDataContainer::sideLength, GUIDataContainer::screenWidth, GUIDataContainer::screenHeight, GUIDataContainer::mouseX, GUIDataContainer::mouseY;
double GUIDataContainer::defaultWindowWidth, GUIDataContainer::defaultWindowHeight;
double GUIDataContainer::windowWidth, GUIDataContainer::windowHeight;
double GUIDataContainer::drAreaHeight, GUIDataContainer::drAreaWidth;

std::vector<std::pair<int, int>> GUIDataContainer::neighbors[1000];
std::vector<std::vector<double>> GUIDataContainer::coords;
std::vector<int> GUIDataContainer::status;	// 0 = healthy, 1 = congested, 2 = alt congested, 3 = down
std::vector<int> GUIDataContainer::path;
std::vector<int> GUIDataContainer::startTime;
std::vector<int> GUIDataContainer::riseTime;
std::vector<int> GUIDataContainer::endState;	

uint32_t GUIDataContainer::AttractivenessArray[8] = {1, 1, 1, 1, 1, 1, 1, 1}; 
uint32_t GUIDataContainer::PopulationDensityArray[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	
// stage 2 parameters
int GUIDataContainer::bsLen = 10;							//base station length in METERS?? Dr. Omar and Dr. Kets want it like this
int GUIDataContainer::antNum = 3;							//in the process of changing this to secNum... 2021-02-26
int GUIDataContainer::transNum = 100;	
int GUIDataContainer::uePerAnt = 15;
int GUIDataContainer::simLen = 1000;
int GUIDataContainer::GUIDataContainer::simNum = 1;
int GUIDataContainer::simStartNum = 0;
std::string GUIDataContainer::simName = "TEST";
int GUIDataContainer::bufSizeInSeconds = 10;
int GUIDataContainer::congestionState = 90;
int GUIDataContainer::alertState = 80;
uint32_t GUIDataContainer::algorithmVer = 1;				//default to new version (2021 version)
float GUIDataContainer::RSRPThreshold = -85.0f;				//default to -85 dBm
uint32_t GUIDataContainer::mobilityBufSizeInMinutes = 5;	//default 5 minutes


// analysis window variables
int spinnerTimeVal = 0;

//variables to help in drawing and manipulating the scatter plot
int GUIDataContainer::currentScatterPlotTime = 0;
double GUIDataContainer::scaleFactor = 0;
float GUIDataContainer::drawingCenterX = 0;
float GUIDataContainer::drawingCenterY = 0;
cairo_t* GUIDataContainer::cairoElement = nullptr;