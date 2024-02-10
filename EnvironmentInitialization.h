#pragma once
class EnvironmentInitialization : private Simulator
{
private:
	static bool setDefaultUsers();
	static bool setBSMaxDataRate();
	static std::vector<Coord<float>> setBSCoords(const std::vector<std::pair<int, int>>* neighbors, int numBSs);
	static void initializeNumTransceivers();
public:
	static bool generateNewENV();
	static int setRetainability(BSstatus);
};