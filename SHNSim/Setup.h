#pragma once

//Set up for environment initialization
class Setup
{
public:
	static bool GUIentryPoint();
	static bool DEBUGentryPoint(int argc, char** argv);
	static bool createTestGUIData(int argc, char** argv);
};