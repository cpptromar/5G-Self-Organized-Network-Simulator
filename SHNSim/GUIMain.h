#include <iostream>
#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <string>
#include <sstream>
#include <gdk/gdkkeysyms.h>
#include <vector>
#include <utility>

#include "Setup.h"
#include "GUIDataContainer.h"
#include "ErrorTracer.h"

using namespace std;

class GUIMain //class to hold functions that can be used in the simulator code so that it can interact with GUI code
{
public:
	static void doProgressBar(double frac, bool fin); //updates progressbar
};

// String to hold the proper sized CSS file to import
//string cssPath = "SHNSim.css"; //I converted this to struct so that there wasn't more than one declaration of 
								// the same global variable (this header file shows up more than once) LOTS OF ERRORS! -SJ
struct
{
	string cssPath = "SHNSim.css";
} StyleSheets;

struct
{
	GtkWidget* progressBar; //must be in struct rather than static variable in GTKMain class above
							//because we had issues with "undefined reference to progressBar"
} MiscWidgets;


// define structure to hold windows used for project
struct
{
	GtkWidget* DrawingWindow;
	GtkWidget* SimParamWindow;
	GtkWidget* PostMenuScreen;
	GtkWidget* DiagnosticsWindow;
	GtkWidget* DebugWindow;
	GtkWidget* ProgressWindow;
	GtkWidget* AnalysisWindow;

	
} WINDOWS;

struct
{
	GtkWidget* endStatusLabel, * startTimeLabel, * riseTimeLabel, * endStateLabel;	// Labels
	GtkWidget* startTimeEntry, * riseTimeEntry, * endStateEntry;	// Text Boxes
	GtkWidget* endStatusDisplayLbl;									// Display label
} realTime;

// define structure to hold window dimensions
struct
{
	int WIDTH;
	int HEIGHT;
} SCREEN;

// define a struct to hold references to entry boxes (used to pass
// entry from the text boxes throughout the entire program)
struct
{
	GtkWidget* baseStationSide, * antennaNumber, * transceiverNum, * transceiverDist, * maxDataRate, * userEquipPerAntenna;
	GtkWidget* simulationLength, * simulationNumber, * simulationStart, * simulationSaveName, * bufferSize;
	GtkWidget* alertEntry, * congestionEntry;
} entryBoxes;

// window setup function prototypes
void setUpDrawingWindow();
void setUpSimParamWindow();
void setUpDiagnosticsWindow();
void setUpDebugWindow();
void setUpPostMenuScreen();
void setUpSimProgressWindow();
void setUpAnalysisWindow();

// navigation function prototypes - used to change windows
void goToSimParams();
void goToDebug();
void goToDrawingStage();
void goToAnalysisStage();
void goToPostStage();
void closeWindows();
void runSim();
void exitProg();

// functions used in drawing window
static void getScreenHeight();
static void drawHex(cairo_t*);
static void drawDot(cairo_t*);
static void button_clicked(GtkWidget * widget, gpointer data);
static void button_clicked_exp(GtkWidget* widget, gpointer data);
static gboolean mouse_moved(GtkWidget * widget, GdkEvent * event, gpointer user_data);
static gboolean mouse_clicked(GtkWidget * widget, GdkEventButton * event, gpointer user_data);
static gboolean mouse_clicked_test(GtkWidget* widget, GdkEventButton* event, gpointer user_data); //-SJ
static gboolean on_draw_event(GtkWidget * widget, cairo_t * cr, gpointer user_data);
static gboolean on_draw_event_test(GtkWidget* widget, cairo_t* cr, gpointer user_data);//-SJ

static float distance(double x1, double y1, double x2, double y2);
static int side(double x1, double y1, double x2, double y2);
static bool deletionValid(int tile);
static void getNeighbors();

// Utility function(s)
void getDimensions();
GtkWidget* UserMessage(GtkWindow* window, string message);
void displayInformation(GtkWidget* widget, GtkWindow* window);
bool debug();

// function to add parameters to param struct; used to pass params to run the simulation
bool addParams();

// function used to update vector with information for environment controller
void updateBsParams();
