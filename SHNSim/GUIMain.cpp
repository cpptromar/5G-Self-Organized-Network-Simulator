#include "GUIMain.h"
#ifndef M_PI
#define M_PI  3.14159265358979323846264338327950288
#endif

using namespace std;


	GtkWidget* alertLvlTxt, * congestionLvlTxt;

int main(int argc, char** argv)
{
	GUIDataContainer::argc = argc;
	GUIDataContainer::argv = argv;

	// initialize gtk	
	gtk_init(&argc, &argv);

	// get screen dimensions
	getDimensions();


	// set up all windows
	setUpDrawingWindow();
	setUpSimParamWindow();
	setUpPostMenuScreen();
	setUpSimProgressWindow();
	setUpDiagnosticsWindow1();

	// initialize the system by making the first window visible
	//gtk_widget_show_all(WINDOWS.DrawingWindow);
	gtk_widget_show_all(WINDOWS.DiagnosticsWindow1);

	gtk_main();
	return 0;
}

void setUpDrawingWindow()
{
	GtkWidget *window, *darea, *button, *updateBsParamsBtn;

	GUIDataContainer::count = 0;
	GUIDataContainer::selectedTile = 0;
	GUIDataContainer::highlightedSide = 0;

	GUIDataContainer::screenHeight = (SCREEN.HEIGHT - 58);
	GUIDataContainer::screenWidth = (SCREEN.WIDTH - 58);
	GUIDataContainer::sideLength = GUIDataContainer::screenHeight * 0.25;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	WINDOWS.DrawingWindow = window;

	darea = gtk_drawing_area_new();
	button = gtk_button_new_with_label("Next Page");
	updateBsParamsBtn = gtk_button_new_with_label("Set Parameters");
	gtk_widget_set_name(updateBsParamsBtn, "subBtn");

	//=====================================================================================//
	
	// Selected BS params label
	realTime.endStatusLabel = gtk_label_new("End Status");
	realTime.startTimeLabel = gtk_label_new("Start Time (n >= 0)");
	realTime.riseTimeLabel = gtk_label_new("Rise Time (n >= 1)");
	realTime.endStateLabel = gtk_label_new("End State (0% < n < 200%)");
	
	// Selected BS params entry boxes
	realTime.startTimeEntry = gtk_entry_new();
	realTime.riseTimeEntry = gtk_entry_new();
	realTime.endStateEntry = gtk_entry_new();

	// Selected BS params display label
	realTime.endStatusDisplayLbl = gtk_label_new("Healthy");
	gtk_widget_set_name(realTime.endStatusDisplayLbl, "displayLabel");

	gtk_entry_set_text(GTK_ENTRY(realTime.startTimeEntry), "0");
	gtk_entry_set_text(GTK_ENTRY(realTime.riseTimeEntry), "1");
	gtk_entry_set_text(GTK_ENTRY(realTime.endStateEntry), "0");

	//=====================================================================================//

	g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), NULL);
	g_signal_connect(updateBsParamsBtn, "clicked", G_CALLBACK(updateBsParams), NULL);
	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "button-press-event", G_CALLBACK(mouse_clicked), NULL);
	g_signal_connect(window, "motion-notify-event", G_CALLBACK(mouse_moved), NULL);

	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
	gtk_widget_set_events(window, GDK_POINTER_MOTION_MASK);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), GUIDataContainer::screenWidth, GUIDataContainer::screenHeight);

	// Creating button that, when click, will display a popup with information for the user
	GtkWidget* infoBtn = gtk_button_new_with_label("Info");
	gtk_widget_set_name(infoBtn, "subBtn");
	g_signal_connect(infoBtn, "clicked", G_CALLBACK(displayInformation), GTK_WINDOW(window));

	// Declaring environment controller parameters
	GtkWidget* alertLvl, * congestionLvl;
	GtkWidget* generalLevelsTitle, * selectedBsParameters;

	alertLvl = gtk_label_new("Alert state (70% < n < 200%)");
	alertLvlTxt = gtk_entry_new();
	congestionLvl = gtk_label_new("Congestion state (70% < n < 200%)");
	congestionLvlTxt = gtk_entry_new();
	
	generalLevelsTitle = gtk_label_new("General States");
	gtk_widget_set_name(generalLevelsTitle, "title");
	selectedBsParameters = gtk_label_new("Selected BS Parameters");
	gtk_widget_set_name(selectedBsParameters, "title");
	
	entryBoxes.alertEntry = alertLvlTxt;
	entryBoxes.congestionEntry = congestionLvlTxt;
	
	gtk_entry_set_text (GTK_ENTRY (alertLvlTxt), (gchar *)(std::to_string(GUIDataContainer::alertState).c_str()));
	gtk_entry_set_text (GTK_ENTRY (congestionLvlTxt), (gchar *)(std::to_string(GUIDataContainer::congestionState).c_str()));

	// Removed "fixed" in favor of GtkBox and utilized and CSS file for coloring //
	GtkWidget* mainContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget* realTimeParamBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	// Add labels and entry boxes into param box container
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), infoBtn, 0, 0, 15);
	
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), generalLevelsTitle, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), alertLvl, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), alertLvlTxt, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), congestionLvl, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), congestionLvlTxt, 0, 0, 5);
	
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), selectedBsParameters, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.endStatusLabel, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.endStatusDisplayLbl, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.startTimeLabel, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.startTimeEntry, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.riseTimeLabel, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.riseTimeEntry, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.endStateLabel, 0, 0, 5);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), realTime.endStateEntry, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(realTimeParamBox), updateBsParamsBtn, 0, 0, 5);
	
	// Add button to the bottom of the param box container
	gtk_box_pack_end(GTK_BOX(realTimeParamBox), button, 0, 0, 5);
	
	// Pack drawing area and parameters into container
	gtk_box_pack_start(GTK_BOX(mainContainer), darea, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(mainContainer), realTimeParamBox, 0, 1, 10);
	
	// set main container to scroll
	GtkWidget *scrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledWindow), mainContainer);
	
	gtk_container_add(GTK_CONTAINER(window), scrolledWindow);
	gtk_window_set_title(GTK_WINDOW(window), "5G Simulator");
	
	// load color settings for the GUI from CSS file
	GtkCssProvider* guiProvider = gtk_css_provider_new();
	if (gtk_css_provider_load_from_path(guiProvider, StyleSheets.cssPath.c_str(), NULL))
	{
		// Window
		gtk_style_context_add_provider(gtk_widget_get_style_context(window), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// Button
		gtk_style_context_add_provider(gtk_widget_get_style_context(button), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(infoBtn), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(updateBsParamsBtn), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// Labels
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.endStatusLabel), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.startTimeLabel), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.riseTimeLabel), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.endStateLabel), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(alertLvl), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(congestionLvl), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// Entry Boxes
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.startTimeEntry), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.riseTimeEntry), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.endStateEntry), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(alertLvlTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(congestionLvlTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// Display Label
		gtk_style_context_add_provider(gtk_widget_get_style_context(realTime.endStatusDisplayLbl), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// Titles
		gtk_style_context_add_provider(gtk_widget_get_style_context(generalLevelsTitle), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(selectedBsParameters), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
}

void setUpSimParamWindow()
{
	// create stage 2 window and configure window properties
	GtkWidget* s2Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(s2Window), "Self-Healing Simulator");
	gtk_window_set_default_size(GTK_WINDOW(s2Window), SCREEN.WIDTH - 60, SCREEN.HEIGHT - 58);
	g_signal_connect(s2Window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_set_position(GTK_WINDOW(s2Window), GTK_WIN_POS_CENTER);

	// copy window into struct objcet for global use across functions
	WINDOWS.SimParamWindow = s2Window;

	// create input labels and text boxes from stage 2 of C# code
	GtkWidget* bsSide, * numAntenna, * numTransceivers, * distTransceivers, * uePerAntenna; // labels
	GtkWidget* bsSideTxt, * numAntennaTxt, * numTransceiversTxt, * distTransceiversTxt, * uePerAntennaTxt; // textbox

	// create input labels and text boxes from stage 3 of C# code
	GtkWidget* simLength, * simNum, * simStart, * simSaveName, * bufSize; // labels
	GtkWidget* simLengthTxt, * simNumTxt, * simStartTxt, * simSaveNameTxt, * bufSizeTxt; // textboxes
	
	// create back button and run simulation button
	GtkWidget* backToS1Btn, * runSimBtn;

	// instantiate input labels, textboxes, and buttons
	bsSide = gtk_label_new("BS Side Length (5 < n < 20)");
	bsSideTxt = gtk_entry_new();
	numAntenna = gtk_label_new("Number of Antenna (1 < n < 6)");
	numAntennaTxt = gtk_entry_new();
	numTransceivers = gtk_label_new("Number of Transceivers (80 < n < 200)");
	numTransceiversTxt = gtk_entry_new();
	distTransceivers = gtk_label_new("Distance between Transceivers (0.001 < n < 0.00865)");
	distTransceiversTxt = gtk_entry_new();
	uePerAntenna = gtk_label_new("Enter UEs per Antenna [normal BS] (1 < n < 40)");
	uePerAntennaTxt = gtk_entry_new();

	simLength = gtk_label_new("Length of Simulation (seconds)");
	simLengthTxt = gtk_entry_new();
	simNum = gtk_label_new("Number of Simulations");
	simNumTxt = gtk_entry_new();
	simStart = gtk_label_new("Simulation Starting Number");
	simStartTxt = gtk_entry_new();
	simSaveName = gtk_label_new("Simulation Save Name");
	simSaveNameTxt = gtk_entry_new();
	bufSize = gtk_label_new("Buffer size");
	bufSizeTxt = gtk_entry_new();

	backToS1Btn = gtk_button_new_with_label("Back");
	runSimBtn = gtk_button_new_with_label("Run Simulation");

	// connect button signals
	g_signal_connect(backToS1Btn, "clicked", G_CALLBACK(backToDrawingStage), NULL);
	g_signal_connect(runSimBtn, "clicked", G_CALLBACK(runSim), NULL);

	// create title labels
	GtkWidget* simParamTitle = gtk_label_new("Simulation Parameters");
	gtk_widget_set_name(simParamTitle, "title");
	GtkWidget* networkParamTitle = gtk_label_new("Network Parameters");
	gtk_widget_set_name(networkParamTitle, "title");
	GtkWidget* selfHealingParamTitle = gtk_label_new("Self-Healing Parameters");
	gtk_widget_set_name(selfHealingParamTitle, "title");

	// instantiate network parameter containers
	GtkWidget* networkParamContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* networkParamRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget* networkParamL = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* networkParamR = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	// instantiate simulation parameter containers
	GtkWidget* simParamContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* simParamRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget* simParamL = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* simParamR = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	// instantiate self healing parameter containers
	GtkWidget* selfHealingParamContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* selfHealingParamRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget* selfHealingParamL = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget* selfHealingParamR = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	// instantiate button container
	GtkWidget* btnContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	
	// instantiate main container
	GtkWidget* mainContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	// pack network parameters into network parameter columns
	gtk_box_pack_start(GTK_BOX(networkParamL), bsSide, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(networkParamL), bsSideTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(networkParamL), numAntenna, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(networkParamL), numAntennaTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(networkParamR), numTransceivers, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(networkParamR), numTransceiversTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(networkParamR), uePerAntenna, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(networkParamR), uePerAntennaTxt, 0, 0, 0);

	// pack simulation parameters into simulation parameter columns
	gtk_box_pack_start(GTK_BOX(simParamL), simLength, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simParamL), simLengthTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simParamL), simNum, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simParamL), simNumTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simParamR), simStart, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simParamR), simStartTxt, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(simParamR), simSaveName, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(simParamR), simSaveNameTxt, 0, 0, 0);
	
	// pack self healing parameters into self-healing container
	gtk_box_pack_start(GTK_BOX(selfHealingParamL), bufSize, 0, 0, 15);
	gtk_box_pack_start(GTK_BOX(selfHealingParamR), bufSizeTxt, 0, 0, 0);
	
	// pack buttons into button container
	gtk_box_pack_start(GTK_BOX(btnContainer), backToS1Btn, 1, 1, 10);
	gtk_box_pack_end(GTK_BOX(btnContainer), runSimBtn, 1, 1, 10);

	// pack network parameter columns into the row
	gtk_box_pack_start(GTK_BOX(networkParamRow), networkParamL, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(networkParamRow), networkParamR, 1, 1, 5);
	
	// pack simulator parameter columns into the row
	gtk_box_pack_start(GTK_BOX(simParamRow), simParamL, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(simParamRow), simParamR, 1, 1, 5);
	
	// pack self healing parameter columns into the row
	gtk_box_pack_start(GTK_BOX(selfHealingParamRow), selfHealingParamL, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(selfHealingParamRow), selfHealingParamR, 1, 1, 5);
	
	// pack components into their respetive main container
	gtk_box_pack_start(GTK_BOX(networkParamContainer), networkParamTitle, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(networkParamContainer), networkParamRow, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(simParamContainer), simParamTitle, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(simParamContainer), simParamRow, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(selfHealingParamContainer), selfHealingParamTitle, 1, 1, 5);
	gtk_box_pack_start(GTK_BOX(selfHealingParamContainer), selfHealingParamRow, 1, 1, 5);
	
	// pack main containers into final master container
	gtk_box_pack_start(GTK_BOX(mainContainer), networkParamContainer, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(mainContainer), simParamContainer, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(mainContainer), selfHealingParamContainer, 1, 1, 10);
	gtk_box_pack_end(GTK_BOX(mainContainer), btnContainer, 1, 1, 10);
	
	// set main container to scroll
	GtkWidget *scrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledWindow), mainContainer);
	
	// add main container to the window
	gtk_container_add(GTK_CONTAINER(s2Window), scrolledWindow);

	// add entry boxes (textboxes) to entry box struct
	entryBoxes.baseStationSide = bsSideTxt;
	entryBoxes.antennaNumber = numAntennaTxt;
	entryBoxes.transceiverNum = numTransceiversTxt;
	entryBoxes.transceiverDist = distTransceiversTxt;
	entryBoxes.userEquipPerAntenna = uePerAntennaTxt;
	entryBoxes.simulationLength = simLengthTxt;
	entryBoxes.simulationNumber = simNumTxt;
	entryBoxes.simulationStart = simStartTxt;
	entryBoxes.simulationSaveName = simSaveNameTxt;
	entryBoxes.bufferSize = bufSizeTxt;

	// Initializes textboxes with default values
	gtk_entry_set_text (GTK_ENTRY (bsSideTxt), (gchar *)(std::to_string(GUIDataContainer::bsLen).c_str()));
	gtk_entry_set_text (GTK_ENTRY (numAntennaTxt), (gchar *)(std::to_string(GUIDataContainer::antNum).c_str()));
	gtk_entry_set_text (GTK_ENTRY (numTransceiversTxt), (gchar *)(std::to_string(GUIDataContainer::transNum).c_str()));
	gtk_entry_set_text (GTK_ENTRY (uePerAntennaTxt), (gchar *)(std::to_string(GUIDataContainer::uePerAnt).c_str()));
	gtk_entry_set_text (GTK_ENTRY (simLengthTxt), (gchar *)(std::to_string(GUIDataContainer::simLen).c_str()));
	gtk_entry_set_text (GTK_ENTRY (simNumTxt), (gchar *)(std::to_string(GUIDataContainer::simNum).c_str()));
	gtk_entry_set_text (GTK_ENTRY (simStartTxt), (gchar *)(std::to_string(GUIDataContainer::simStartNum).c_str()));
	gtk_entry_set_text (GTK_ENTRY (simSaveNameTxt), (gchar *)(GUIDataContainer::simName.c_str()));
	gtk_entry_set_text (GTK_ENTRY (bufSizeTxt), (gchar *)(std::to_string(GUIDataContainer::bufSizeInSeconds).c_str()));

	// load color settings for the GUI from CSS file
	GtkCssProvider* guiProvider = gtk_css_provider_new();
	if (gtk_css_provider_load_from_path(guiProvider, StyleSheets.cssPath.c_str(), NULL))
	{
		// Window
		gtk_style_context_add_provider(gtk_widget_get_style_context(s2Window), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// labels
		gtk_style_context_add_provider(gtk_widget_get_style_context(bsSide), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numAntenna), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numTransceivers), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(distTransceivers), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(uePerAntenna), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simLength), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simNum), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simStart), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simSaveName), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(bufSize), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

		// buttons		
		gtk_style_context_add_provider(gtk_widget_get_style_context(backToS1Btn), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(runSimBtn), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

		// entry
		gtk_style_context_add_provider(gtk_widget_get_style_context(bsSideTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numAntennaTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(numTransceiversTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(distTransceiversTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(uePerAntennaTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simLengthTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simNumTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simStartTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simSaveNameTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(bufSizeTxt), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

		// title
		gtk_style_context_add_provider(gtk_widget_get_style_context(networkParamTitle), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(simParamTitle), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(selfHealingParamTitle), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

	}
}

void setUpPostMenuScreen()
{
	// create stage 2 window and configure window properties
	GtkWidget* PostMenuWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(PostMenuWindow), "Self-Healing Simulator");
	gtk_window_set_default_size(GTK_WINDOW(PostMenuWindow), SCREEN.WIDTH - 60, SCREEN.HEIGHT - 58);
	g_signal_connect(PostMenuWindow, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_set_position(GTK_WINDOW(PostMenuWindow), GTK_WIN_POS_CENTER);

	// copy window into struct objcet for global use across functions
	WINDOWS.PostMenuScreen = PostMenuWindow;

	// create quit button and to analysis button
	GtkWidget* quitProg, * toAnalysis, * toScatter, * toLineGraph, * toData, * exportToCsv;
	// instantiate buttons
	quitProg = gtk_button_new_with_label("Quit");
	toAnalysis = gtk_button_new_with_label("Analysis");
	toScatter = gtk_button_new_with_label("Scatter Plot");
	toLineGraph = gtk_button_new_with_label("Congestion Level Graph");
	toData = gtk_button_new_with_label("View Tabulated Data");
	exportToCsv = gtk_button_new_with_label("Export to Excel");

	// Set tooltips for buttons
	gtk_widget_set_tooltip_text(quitProg, "Exit Program");
	gtk_widget_set_tooltip_text(toAnalysis, "Exit Program"); // also edit this later
	gtk_widget_set_tooltip_text(toScatter, "Exit Program"); // also edit this later
	gtk_widget_set_tooltip_text(toLineGraph, "Exit Program"); // also edit this later
	gtk_widget_set_tooltip_text(toData, "Exit Program"); // also edit this later
	gtk_widget_set_tooltip_text(exportToCsv, "Exit Program"); // also edit this later

	// connect button signals
	g_signal_connect(quitProg, "clicked", G_CALLBACK(exitProg), NULL);
	g_signal_connect(toAnalysis, "clicked", G_CALLBACK(exitProg), NULL); // Edit this later
	g_signal_connect(toScatter, "clicked", G_CALLBACK(exitProg), NULL); // Edit this later
	g_signal_connect(toLineGraph, "clicked", G_CALLBACK(exitProg), NULL); // Edit this later
	g_signal_connect(toData, "clicked", G_CALLBACK(exitProg), NULL); // Edit this later
	g_signal_connect(exportToCsv, "clicked", G_CALLBACK(exitProg), NULL); // Edit this later

	// instantiate button container
	GtkWidget* btnContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);

	// instantiate main container
	GtkWidget* mainContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	// pack buttons into button container
	gtk_box_pack_start(GTK_BOX(btnContainer), toAnalysis, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(btnContainer), toData, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(btnContainer), toScatter, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(btnContainer), toLineGraph, 1, 1, 10);
	gtk_box_pack_start(GTK_BOX(btnContainer), exportToCsv, 1, 1, 10);
	gtk_box_pack_end(GTK_BOX(btnContainer), quitProg, 1, 1, 10);

	// pack main containers into final master container
	gtk_box_pack_end(GTK_BOX(mainContainer), btnContainer, 1, 1, 10);

	// add main container to the window
	gtk_container_add(GTK_CONTAINER(PostMenuWindow), mainContainer);

	GtkCssProvider* guiProvider = gtk_css_provider_new();
	if (gtk_css_provider_load_from_path(guiProvider, StyleSheets.cssPath.c_str(), NULL))
	{
		// Window
		gtk_style_context_add_provider(gtk_widget_get_style_context(PostMenuWindow), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

		// buttons		
		
		gtk_style_context_add_provider(gtk_widget_get_style_context(toAnalysis), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(toData), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(toScatter), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(toLineGraph), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(exportToCsv), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_add_provider(gtk_widget_get_style_context(quitProg), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
}

void setUpDiagnosticsWindow()
{
	// TODO - Will contain information about the system during runtime.
}

void setUpSimProgressWindow()
{

	GtkBuilder* builder = gtk_builder_new_from_file("GUI_ProgressWindow.glade");
	GtkWidget* window;
	GtkWidget* prog1;
	GtkWidget* buttonQuit;
	window = GTK_WIDGET(gtk_builder_get_object(builder, "windowProgress"));
	g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

	buttonQuit = GTK_WIDGET(gtk_builder_get_object(builder, "buttonQuit"));
	g_signal_connect(buttonQuit, "clicked", G_CALLBACK(gtk_main_quit), NULL); //instead of gtk_main_quit, go to a closing function (future work)

	prog1 = GTK_WIDGET(gtk_builder_get_object(builder, "prog1"));
	WINDOWS.ProgressWindow = window;
	MiscWidgets.progressBar = prog1;
}

void setUpDiagnosticsWindow1()
{
	GtkWidget* window, * darea;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	WINDOWS.DiagnosticsWindow1 = window;

	darea = gtk_drawing_area_new();

	//=====================================================================================//

	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(drawDot), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "button-press-event", G_CALLBACK(mouse_clicked_test), NULL);
	g_signal_connect(window, "motion-notify-event", G_CALLBACK(mouse_moved), NULL);

	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
	gtk_widget_set_events(window, GDK_POINTER_MOTION_MASK);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), GUIDataContainer::screenWidth, GUIDataContainer::screenHeight);
	gtk_window_set_title(GTK_WINDOW(window), "5G Simulator");

	GtkWidget* mainContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	// Pack drawing area and parameters into container
	gtk_box_pack_start(GTK_BOX(mainContainer), darea, 1, 1, 5);

	// set main container to scroll
	GtkWidget* scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledWindow), mainContainer);
	gtk_container_add(GTK_CONTAINER(window), scrolledWindow);


	// load color settings for the GUI from CSS file
	GtkCssProvider* guiProvider = gtk_css_provider_new();
	if (gtk_css_provider_load_from_path(guiProvider, StyleSheets.cssPath.c_str(), NULL))
	{
		// Window
		gtk_style_context_add_provider(gtk_widget_get_style_context(window), GTK_STYLE_PROVIDER(guiProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

	}
}


void goToSimParams()
{
	/* print out selected bs params for diagnostic purposes
	cout << "Environment Controller Parameters:" << endl;
	cout << "   Status, Start Time, Rise Time, End State" << endl;
	for(int i = 0; i < GUIDataContainer::startTime.size(); i++)
	{	
		cout << "      " << GUIDataContainer::status[i] << "       " << GUIDataContainer::startTime[i] << "           " << GUIDataContainer::riseTime[i] << "          " << GUIDataContainer::endState[i] << endl;
	}*/
	
	gtk_widget_show_all(WINDOWS.SimParamWindow);
	gtk_widget_hide_on_delete(WINDOWS.DrawingWindow);
}

// NOTE: does NOT open another window after running simulation, diagnostics page could be added if needed
void runSim()
{
	// call a function to add values from entry boxes to parameter struct
	bool valid = addParams();
	
	if(!valid)
		return;

	//gtk_widget_show_all(WINDOWS.PostMenuScreen);
	//gtk_widget_hide_on_delete(WINDOWS.SimParamWindow);
	gtk_widget_show_all(WINDOWS.ProgressWindow);

	//update UI changes (the opening and closing of the windows)
	while (gtk_events_pending()) gtk_main_iteration();


	Setup::GUIentryPoint();
	ErrorTracer::programExit();
}

void exitProg()
{
	gtk_main_quit();
}

void GUIMain::doProgressBar(double frac, bool fin) //updates the progressbar
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(MiscWidgets.progressBar), (gdouble)frac);

	//without this line, the gtk UI widgets do not refresh.
	//gtk widgets normally refresh upon returning to the main function!!
	
	//if (fin)
	//{
	//	gtk_widget_hide_on_delete(WINDOWS.ProgressWindow);
	//	gtk_widget_show_all(WINDOWS.PostMenuScreen);
	//}

	if (fin) //this is test for Steven...
	{
		gtk_widget_hide_on_delete(WINDOWS.SimParamWindow);
		gtk_widget_hide_on_delete(WINDOWS.ProgressWindow);
		gtk_widget_show_all(WINDOWS.PostMenuScreen);
	}

	while (gtk_events_pending()) gtk_main_iteration(); //update UI changes
}

void backToDrawingStage()
{
	gtk_widget_show_all(WINDOWS.DrawingWindow);
	gtk_widget_hide_on_delete(WINDOWS.SimParamWindow);
}

static void drawDot(cairo_t* cr)
{
	if (GUIDataContainer::count == 0)
	{
		/*vector<double> test;
		test.push_back(GUIDataContainer::screenWidth * 0.95 / 2.0);
		test.push_back(GUIDataContainer::screenHeight * 0.95 / 2.0);
		GUIDataContainer::coords.erase(GUIDataContainer::coords.begin(), GUIDataContainer::coords.end());
		GUIDataContainer::coords.push_back(test);

		GUIDataContainer::status.erase(GUIDataContainer::status.begin(), GUIDataContainer::status.end());
		GUIDataContainer::status.push_back((int)0);

		GUIDataContainer::endState.erase(GUIDataContainer::endState.begin(), GUIDataContainer::endState.end());
		GUIDataContainer::endState.push_back((int)50);

		GUIDataContainer::startTime.erase(GUIDataContainer::startTime.begin(), GUIDataContainer::startTime.end());
		GUIDataContainer::startTime.push_back((int)0);

		GUIDataContainer::riseTime.erase(GUIDataContainer::riseTime.begin(), GUIDataContainer::riseTime.end());
		GUIDataContainer::riseTime.push_back((int)1);

		GUIDataContainer::path.erase(GUIDataContainer::path.begin(), GUIDataContainer::path.end());
		GUIDataContainer::path.push_back(7);
		GUIDataContainer::count = 1;*/
	}
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_line_to(cr, 0, 0);
	cairo_line_to(cr, 0, GUIDataContainer::screenHeight);
	cairo_line_to(cr, GUIDataContainer::screenWidth, GUIDataContainer::screenHeight);
	cairo_line_to(cr, GUIDataContainer::screenWidth, 0);
	cairo_line_to(cr, 0, 0);
	cairo_fill(cr);

	double mousedY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::mouseY;
	double mousedX = GUIDataContainer::mouseX - GUIDataContainer::coords[GUIDataContainer::selectedTile][0];

	double mouseparam = mousedY / mousedX;
	double mouseslope = atan(mouseparam) * 180.0 / M_PI;
	double mousedist = sqrt(mousedY * mousedY + mousedX * mousedX);
	if (abs(mouseslope) >= 60)
	{
		if (mousedY < 0)
		{
			GUIDataContainer::highlightedSide = 0; // Bottom
		}
		else
		{
			GUIDataContainer::highlightedSide = 3; // Top
		}
	}
	else
	{
		if (mousedY < 0)
		{
			if (mousedX > 0)
			{
				GUIDataContainer::highlightedSide = 1;	// Bottom Right
			}
			else
			{
				GUIDataContainer::highlightedSide = 5;	// Bottom Left
			}
		}
		else
		{
			if (mousedX > 0)
			{
				GUIDataContainer::highlightedSide = 2;	// Top Right
			}
			else
			{
				GUIDataContainer::highlightedSide = 4;	// Top Left
			}
		}
	}
	cairo_set_source_rgb(cr, 0, 1, 0);
	cairo_set_line_width(cr, 2.0);
	for (int i = 0; i < GUIDataContainer::count; i++)	// Fill
	{
		if (i == GUIDataContainer::selectedTile)
		{
			switch (GUIDataContainer::status[i])
			{
			case 0:	// Healthy
				cairo_set_source_rgb(cr, 0, 100.0 / 255.0, 0);
				break;
			case 1:	// Congest 1
				cairo_set_source_rgb(cr, 150.0 / 255.0, 150.0 / 255.0, 0);
				break;
			case 2:	// Congest 2
				cairo_set_source_rgb(cr, 150.0 / 255.0, 75.0 / 255.0, 0);
				break;
			case 3:	// Fail
				cairo_set_source_rgb(cr, 150.0 / 255.0, 0, 0);
				break;
			default:
				cairo_set_source_rgb(cr, 1, 1, 1);
			}
		}
		else
		{
			switch (GUIDataContainer::status[i])
			{
			case 0:	// Healthy
				cairo_set_source_rgb(cr, 0, 200.0 / 255.0, 0);
				break;
			case 1:	// Congest 1
				cairo_set_source_rgb(cr, 200.0 / 255.0, 200.0 / 255.0, 0);
				break;
			case 2:	// Congest 2
				cairo_set_source_rgb(cr, 200.0 / 255.0, 100.0 / 255.0, 0);
				break;
			case 3:	// Fail
				cairo_set_source_rgb(cr, 200.0 / 255.0, 0, 0);
				break;
			default:
				cairo_set_source_rgb(cr, 1, 1, 1);
			}
		}
		cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));

		for (int j = 0; j <= 5; j++)
		{
			cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + j)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + j)));
		}
		cairo_fill(cr);
	}
	for (int i = 0; i < GUIDataContainer::count; i++)	// Border
	{
		cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
		for (int k = 0; k <= 5; k++)
		{
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_set_line_width(cr, 2.0);
			if (i == GUIDataContainer::selectedTile && k == GUIDataContainer::highlightedSide)
			{
				cairo_set_source_rgb(cr, 1, 0, 0);
				cairo_set_line_width(cr, 4.0);
			}
			cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
			cairo_stroke(cr);
			if (k < 5)
			{
				cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
			}
		}
	}
	cairo_line_to(cr, GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
	for (int k = 0; k <= 5; k++)
	{
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_set_line_width(cr, 2.0);
		if (k == GUIDataContainer::highlightedSide)
		{
			cairo_set_source_rgb(cr, 1, 0, 0);
			cairo_set_line_width(cr, 4.0);
		}
		cairo_line_to(cr, GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
		cairo_stroke(cr);
		if (k < 5)
		{
			cairo_line_to(cr, GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
		}
	}
	for (int i = 0; i < GUIDataContainer::count; i++)	// Numbers
	{
		cairo_set_font_size(cr, GUIDataContainer::sideLength / 2.0);

		string result, result2, result3;
		stringstream convert, convert2, convert3;
		convert << i;
		result = convert.str();
		const char* c = result.c_str();

		
		cairo_set_source_rgb(cr, 0, 0, 1);

		cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 2.5);

		cairo_show_text(cr, c);


	}

}

static void drawHex(cairo_t * cr)
{
	if (GUIDataContainer::count == 0)
	{
		vector<double> test;
		test.push_back(GUIDataContainer::screenWidth * 0.95 / 2.0);
		test.push_back(GUIDataContainer::screenHeight * 0.95 / 2.0);
		GUIDataContainer::coords.erase(GUIDataContainer::coords.begin(), GUIDataContainer::coords.end());
		GUIDataContainer::coords.push_back(test);

		GUIDataContainer::status.erase(GUIDataContainer::status.begin(), GUIDataContainer::status.end());
		GUIDataContainer::status.push_back((int)0);

		GUIDataContainer::endState.erase(GUIDataContainer::endState.begin(), GUIDataContainer::endState.end());
		GUIDataContainer::endState.push_back((int)50);

		GUIDataContainer::startTime.erase(GUIDataContainer::startTime.begin(), GUIDataContainer::startTime.end());
		GUIDataContainer::startTime.push_back((int)0);

		GUIDataContainer::riseTime.erase(GUIDataContainer::riseTime.begin(), GUIDataContainer::riseTime.end());
		GUIDataContainer::riseTime.push_back((int)1);

		GUIDataContainer::path.erase(GUIDataContainer::path.begin(), GUIDataContainer::path.end());
		GUIDataContainer::path.push_back(7);
		GUIDataContainer::count = 1;
	}
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_line_to(cr, 0, 0);
	cairo_line_to(cr, 0, GUIDataContainer::screenHeight);
	cairo_line_to(cr, GUIDataContainer::screenWidth, GUIDataContainer::screenHeight);
	cairo_line_to(cr, GUIDataContainer::screenWidth, 0);
	cairo_line_to(cr, 0, 0);
	cairo_fill(cr);

	double mousedY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::mouseY;
	double mousedX = GUIDataContainer::mouseX - GUIDataContainer::coords[GUIDataContainer::selectedTile][0];

	double mouseparam = mousedY / mousedX;
	double mouseslope = atan(mouseparam) * 180.0 / M_PI;
	double mousedist = sqrt(mousedY * mousedY + mousedX * mousedX);
	if (abs(mouseslope) >= 60)
	{
		if (mousedY < 0)
		{
			GUIDataContainer::highlightedSide = 0; // Bottom
		}
		else
		{
			GUIDataContainer::highlightedSide = 3; // Top
		}
	}
	else
	{
		if (mousedY < 0)
		{
			if (mousedX > 0)
			{
				GUIDataContainer::highlightedSide = 1;	// Bottom Right
			}
			else
			{
				GUIDataContainer::highlightedSide = 5;	// Bottom Left
			}
		}
		else
		{
			if (mousedX > 0)
			{
				GUIDataContainer::highlightedSide = 2;	// Top Right
			}
			else
			{
				GUIDataContainer::highlightedSide = 4;	// Top Left
			}
		}
	}
	cairo_set_source_rgb(cr, 0, 1, 0);
	cairo_set_line_width(cr, 2.0);
	for (int i = 0; i < GUIDataContainer::count; i++)	// Fill
	{
		if (i == GUIDataContainer::selectedTile)
		{
			switch (GUIDataContainer::status[i])
			{
			case 0:	// Healthy
				cairo_set_source_rgb(cr, 0, 100.0 / 255.0, 0);
				break;
			case 1:	// Congest 1
				cairo_set_source_rgb(cr, 150.0 / 255.0, 150.0 / 255.0, 0);
				break;
			case 2:	// Congest 2
				cairo_set_source_rgb(cr, 150.0 / 255.0, 75.0 / 255.0, 0);
				break;
			case 3:	// Fail
				cairo_set_source_rgb(cr, 150.0 / 255.0, 0, 0);
				break;
			default:
				cairo_set_source_rgb(cr, 1, 1, 1);
			}
		}
		else
		{
			switch (GUIDataContainer::status[i])
			{
			case 0:	// Healthy
				cairo_set_source_rgb(cr, 0, 200.0 / 255.0, 0);
				break;
			case 1:	// Congest 1
				cairo_set_source_rgb(cr, 200.0 / 255.0, 200.0 / 255.0, 0);
				break;
			case 2:	// Congest 2
				cairo_set_source_rgb(cr, 200.0 / 255.0, 100.0 / 255.0, 0);
				break;
			case 3:	// Fail
				cairo_set_source_rgb(cr, 200.0 / 255.0, 0, 0);
				break;
			default:
				cairo_set_source_rgb(cr, 1, 1, 1);
			}
		}
		cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));

		for (int j = 0; j <= 5; j++)
		{
			cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + j)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + j)));
		}
		cairo_fill(cr);
	}
	for (int i = 0; i < GUIDataContainer::count; i++)	// Border
	{
		cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
		for (int k = 0; k <= 5; k++)
		{
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_set_line_width(cr, 2.0);
			if (i == GUIDataContainer::selectedTile && k == GUIDataContainer::highlightedSide)
			{
				cairo_set_source_rgb(cr, 1, 0, 0);
				cairo_set_line_width(cr, 4.0);
			}
			cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
			cairo_stroke(cr);
			if (k < 5)
			{
				cairo_line_to(cr, GUIDataContainer::coords[i][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
			}
		}
	}
	cairo_line_to(cr, GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + 5)), GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + 5)));
	for (int k = 0; k <= 5; k++)
	{
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_set_line_width(cr, 2.0);
		if (k == GUIDataContainer::highlightedSide)
		{
			cairo_set_source_rgb(cr, 1, 0, 0);
			cairo_set_line_width(cr, 4.0);
		}
		cairo_line_to(cr, GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
		cairo_stroke(cr);
		if (k < 5)
		{
			cairo_line_to(cr, GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * sin(2 * M_PI / 6 * (0.5 + k)), GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * cos(2 * M_PI / 6 * (0.5 + k)));
		}
	}
	for (int i = 0; i < GUIDataContainer::count; i++)	// Numbers
	{
		cairo_set_font_size(cr, GUIDataContainer::sideLength / 2.0);
		
		string result, result2, result3;
		stringstream convert, convert2, convert3;
		convert << i;
		result = convert.str();
		const char* c = result.c_str();

		/*
		convert2 << GUIDataContainer::status[i];
		result2 = convert2.str();
		const char* c2 = result2.c_str();

		convert3 << GUIDataContainer::endState[i];
		result3 = convert3.str();
		const char* c3 = result3.c_str();
		

		cairo_set_source_rgb(cr, 1, 1, 1);
		if (i < 10)
		{
			cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 3.0);
		}
		else if (i < 100)
		{
			cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0 * 2.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 3.0);
		}
		
		*/

		cairo_set_source_rgb(cr, 0, 0, 1);
		
		cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 2.5);

		cairo_show_text(cr, c);
		
		/*
		cairo_set_source_rgb(cr, 0, 0, 1);
		if (GUIDataContainer::endState[i] < 10)
		{
			cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 3.0 - GUIDataContainer::sideLength / 2.0);
		}
		else if (GUIDataContainer::endState[i] < 100)
		{
			cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0 * 2.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 3.0 - GUIDataContainer::sideLength / 2.0);
		}
		else
		{
			cairo_move_to(cr, GUIDataContainer::coords[i][0] - GUIDataContainer::sideLength / 2.0 / 3.0 * 3.0, GUIDataContainer::coords[i][1] + GUIDataContainer::sideLength / 2.0 / 3.0 - GUIDataContainer::sideLength / 2.0);
		}
		
		cairo_show_text(cr, c3);
		
		*/
	}
}

static void button_clicked(GtkWidget * widget, gpointer data)
{
	/*system("reset");
	for (int i = 0; i < GUIDataContainer::count; i++)
	{
		printf("%i: (%f, %f, %i)\n", i, GUIDataContainer::coords[i][0], GUIDataContainer::coords[i][1], GUIDataContainer::status[i]);
	}*/
	getNeighbors();
	/*
	for (int n = 0; n < GUIDataContainer::count; n++)
	{
		printf("Base Station: %i\n\tCan be deleted: %s\n\tNeighbors: ", n, (deletionValid(n) ? "true" : "false"));
		for (int i = 0; i < GUIDataContainer::neighbors[n].size(); i++)
		{
			printf("(%i,%i)", GUIDataContainer::neighbors[n][i].first, GUIDataContainer::neighbors[n][i].second);
			if (i < GUIDataContainer::neighbors[n].size() - 1)
			{
				printf(", ");
			}
		}
		printf("\n");
	}
	printf("\n");*/
	goToSimParams();
}

static gboolean mouse_moved(GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
	if (event->type == GDK_MOTION_NOTIFY)
	{
		GdkEventMotion* e = (GdkEventMotion*)event;
		GUIDataContainer::mouseX = (guint32)e->x;
		GUIDataContainer::mouseY = (guint32)e->y;
		gtk_widget_queue_draw(widget);
	}
}

static gboolean mouse_clicked(GtkWidget * widget, GdkEventButton * event, gpointer user_data)
{
	bool changeScale = false;
	if (event->button == 1) //Left Mouse Click
	{
		double dY = (GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - event->y);
		double dX = (event->x - GUIDataContainer::coords[GUIDataContainer::selectedTile][0]);

		double param = dY / dX;
		double slope = atan(param) * 180.0 / M_PI;
		double setX, setY;
		int setPath;
		double dist = sqrt(dY * dY + dX * dX);

		bool changedTile = false;
		double distance = sqrt(dY * dY + dX * dX);
		double newdY, newdX, newDistance;
		for (int i = 0; i < GUIDataContainer::count; i++)
		{
			newdY = (GUIDataContainer::coords[i][1] - event->y);
			newdX = (event->x - GUIDataContainer::coords[i][0]);

			newDistance = sqrt(newdY * newdY + newdX * newdX);
			if (distance > newDistance)
			{
				distance = newDistance;
				if (distance < GUIDataContainer::sideLength * sqrt(3) / 2)	// If click is inside hex
				{
					GUIDataContainer::selectedTile = i;
					changedTile = true;
				}
			}
		}
		if (!changedTile)
		{
			if (dist > GUIDataContainer::sideLength* sqrt(3) / 2)
			{
				if (abs(slope) >= 60)
				{
					if (dY < 0)	// Bottom
					{
						setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0];
						setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * sqrt(3);
						setPath = 0;
					}
					else  // Top
					{
						setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0];
						setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::sideLength * sqrt(3);
						setPath = 3;
					}
				}
				else
				{
					if (dY < 0)
					{
						if (dX > 0)	// Bottom Right
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 5;
						}
						else	// Bottom Left
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] - GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 1;
						}
					}
					else
					{
						if (dX > 0)	// Top Right
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 4;
						}
						else	// Top Left
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] - GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 2;
						}
					}
				}
				bool pointExists = false;
				for (int i = 0; i < GUIDataContainer::count; i++)
				{
					if (round(GUIDataContainer::coords[i][0]) == round(setX) && round(GUIDataContainer::coords[i][1]) == round(setY))
					{
						pointExists = true;
						break;
					}
				}
				if (!pointExists)
				{
					vector<double> test;
					test.push_back(setX);
					test.push_back(setY);
					GUIDataContainer::coords.push_back(test);

					GUIDataContainer::status.push_back((int)0);
					GUIDataContainer::endState.push_back((int)50);
					GUIDataContainer::startTime.push_back((int)0);
					GUIDataContainer::riseTime.push_back((int)1);


					GUIDataContainer::path[GUIDataContainer::count - 1] = setPath;

					GUIDataContainer::selectedTile = GUIDataContainer::count;
					GUIDataContainer::count += 1;
					changeScale = true;
				}
			}
			else	// If inside the hexagon, cycle states
			{	
				GUIDataContainer::status[GUIDataContainer::selectedTile] = (GUIDataContainer::status[GUIDataContainer::selectedTile] + 1) % 4;
				// rotate from healthy -> user congested -> demand congested -> failing
				if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 0)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = 50;
				}
				else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 1)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = GUIDataContainer::congestionState;
				}
				else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 2)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = GUIDataContainer::congestionState;
				}
				else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 3)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = 0;
				}
				
			}
		}
	}
	if (event->button == 3)	// Right Mouse Click
	{
		double dY = (GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - event->y);
		double dX = (event->x - GUIDataContainer::coords[GUIDataContainer::selectedTile][0]);

		double param = dY / dX;
		double slope = atan(param) * 180.0 / M_PI;
		double distance = sqrt(dY * dY + dX * dX);
		double newdY, newdX, newDistance;
		bool foundClick = false;
		for (int i = 0; i < GUIDataContainer::count; i++)
		{
			if (!foundClick)
			{
				newdY = (GUIDataContainer::coords[i][1] - event->y);
				newdX = (event->x - GUIDataContainer::coords[i][0]);

				newDistance = sqrt(newdY * newdY + newdX * newdX);
				//printf("Pass: %i\n", i);
				if (distance > newDistance)
				{
					distance = newDistance;
					if (round(distance) < round(GUIDataContainer::sideLength * sqrt(3) / 2))	// If click is inside hex
					{
						foundClick = true;
						if (GUIDataContainer::count > 1)
						{
							getNeighbors();
							if (deletionValid(i))
							{
								//printf("Deleting: %i\n", i);
								GUIDataContainer::coords.erase(GUIDataContainer::coords.begin() + i);
								GUIDataContainer::status.erase(GUIDataContainer::status.begin() + i);
								GUIDataContainer::endState.erase(GUIDataContainer::endState.begin() + i);
								GUIDataContainer::startTime.erase(GUIDataContainer::startTime.begin() + i);
								GUIDataContainer::riseTime.erase(GUIDataContainer::riseTime.begin() + i);
								GUIDataContainer::count -= 1;
								GUIDataContainer::selectedTile = 0;
								changeScale = true;
							}
							else
							{
								//printf("Deletion invalid\n");
							}
						}
						else
						{
							//printf("Last tile cannot be deleted\n");
						}
					}
				}
				else if (round(distance) < round(GUIDataContainer::sideLength * sqrt(3) / 2))	// If click is inside hex
				{
					foundClick = true;
					if (GUIDataContainer::count > 1)
					{
						getNeighbors();
						if (deletionValid(i))
						{
							//printf("Deleting: %i\n", GUIDataContainer::selectedTile);
							GUIDataContainer::coords.erase(GUIDataContainer::coords.begin() + GUIDataContainer::selectedTile);
							GUIDataContainer::status.erase(GUIDataContainer::status.begin() + i);
							GUIDataContainer::endState.erase(GUIDataContainer::endState.begin() + i);
							GUIDataContainer::startTime.erase(GUIDataContainer::startTime.begin() + i);
							GUIDataContainer::riseTime.erase(GUIDataContainer::riseTime.begin() + i);
							GUIDataContainer::count -= 1;
							GUIDataContainer::selectedTile = 0;
							changeScale = true;
						}
						else
						{
							//printf("Deletion invalid\n");
						}
					}
					else
					{
						//printf("Last tile cannot be deleted\n");
					}
				}
			}
			else
			{
				break;
			}
		}
	}
	if (changeScale)	// Work on this part, needs to scale larger if tiles are deleted
	{
		float ratio = 1.0;
		float minX, maxX, minY, maxY, difX, difY, numX, numY;
		minX = GUIDataContainer::coords[0][0];
		maxX = GUIDataContainer::coords[0][0];
		minY = GUIDataContainer::coords[0][1];
		maxY = GUIDataContainer::coords[0][1];
		for (int i = 1; i < GUIDataContainer::count; i++)
		{
			if (minX > GUIDataContainer::coords[i][0])
			{
				minX = GUIDataContainer::coords[i][0];
			}
			if (maxX < GUIDataContainer::coords[i][0])
			{
				maxX = GUIDataContainer::coords[i][0];
			}
			if (minY > GUIDataContainer::coords[i][1])
			{
				minY = GUIDataContainer::coords[i][1];
			}
			if (maxY < GUIDataContainer::coords[i][1])
			{
				maxY = GUIDataContainer::coords[i][1];
			}
		}
		difX = abs(maxX - minX);
		difY = abs(maxY - minY);
		numX = ceil(difX / (GUIDataContainer::sideLength * sqrt(3) / 2)) / 2 + 1;
		numY = ceil(difY / (GUIDataContainer::sideLength * sqrt(3) / 2)) / 2 + 1;
		//printf("NumX: %f\nNumY: %f\n", numX, numY);

		double prevSideLength = GUIDataContainer::sideLength;

		if (GUIDataContainer::count == 1)	// Good
		{
			ratio = (GUIDataContainer::screenHeight * 0.25) / GUIDataContainer::sideLength;
			minX = GUIDataContainer::screenWidth * 0.95 / 2.0;
			maxX = GUIDataContainer::screenWidth * 0.95 / 2.0;
			minY = GUIDataContainer::screenHeight * 0.95 / 2.0;
			maxY = GUIDataContainer::screenHeight * 0.95 / 2.0;
			difX = 0;
			difY = 0;
			GUIDataContainer::coords[0][0] = GUIDataContainer::screenWidth * 0.95 / 2.0;
			GUIDataContainer::coords[0][1] = GUIDataContainer::screenHeight * 0.95 / 2.0;
		}
		// If shrinking
		else if ((GUIDataContainer::sideLength * sqrt(3) * numX) > GUIDataContainer::screenWidth * 0.95 || (GUIDataContainer::sideLength * sqrt(3) * numY) > GUIDataContainer::screenHeight * 0.95)
		{
			//printf("Shrinking\n");
			bool keepShrinking = true;
			while (keepShrinking)
			{
				if ((GUIDataContainer::sideLength * sqrt(3) * numX) > GUIDataContainer::screenWidth * 0.95 || (GUIDataContainer::sideLength * sqrt(3) * numY) > GUIDataContainer::screenHeight * 0.95)
				{
					GUIDataContainer::sideLength *= 0.999;
				}
				else
				{
					keepShrinking = false;
				}
			}
		}
		else
		{
			//printf("Growing\n");
			bool keepGrowing = true;
			while (keepGrowing)
			{
				if (!((GUIDataContainer::sideLength * sqrt(3) * numX) > GUIDataContainer::screenWidth * 0.95 || (GUIDataContainer::sideLength * sqrt(3) * numY) > GUIDataContainer::screenHeight * 0.95))
				{
					GUIDataContainer::sideLength *= 1.001;
				}
				else
				{
					keepGrowing = false;
				}
			}
		}
		ratio = GUIDataContainer::sideLength / prevSideLength;
		for (int i = 0; i < GUIDataContainer::count; i++)
		{
			GUIDataContainer::coords[i][0] = (GUIDataContainer::coords[i][0] - GUIDataContainer::screenWidth * 0.95 / 2.0) * ratio + GUIDataContainer::screenWidth * 0.95 / 2.0 - (minX + difX / 2.0 - GUIDataContainer::screenWidth * 0.95 / 2.0);
			GUIDataContainer::coords[i][1] = (GUIDataContainer::coords[i][1] - GUIDataContainer::screenHeight * 0.95 / 2.0) * ratio + GUIDataContainer::screenHeight * 0.95 / 2.0 + (GUIDataContainer::screenHeight * 0.95 / 2.0 - (maxY - difY / 2.0));
		}
	}

	gtk_entry_set_text(GTK_ENTRY(realTime.endStateEntry), std::to_string(GUIDataContainer::endState[GUIDataContainer::selectedTile]).c_str());
	gtk_entry_set_text(GTK_ENTRY(realTime.startTimeEntry), std::to_string(GUIDataContainer::startTime[GUIDataContainer::selectedTile]).c_str());
	gtk_entry_set_text(GTK_ENTRY(realTime.riseTimeEntry), std::to_string(GUIDataContainer::riseTime[GUIDataContainer::selectedTile]).c_str());
	if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 1)
	{
		gtk_label_set_text(GTK_LABEL(realTime.endStatusDisplayLbl), "User Congested"); // 1 = User Congested
	}
	else if(GUIDataContainer::status[GUIDataContainer::selectedTile] == 2)
	{
		gtk_label_set_text(GTK_LABEL(realTime.endStatusDisplayLbl), "Demand Congested"); // 2 = Demand Congested
	}
	else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 3)
	{
		gtk_label_set_text(GTK_LABEL(realTime.endStatusDisplayLbl), "Failing"); // 3 = Failing
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(realTime.endStatusDisplayLbl), "Normal"); // 0 = Normal
	}
	
	gtk_widget_queue_draw(widget);
	return TRUE;
}

static gboolean mouse_clicked_test(GtkWidget* widget, GdkEventButton* event, gpointer user_data) //-SJ
{

	bool changeScale = false;
	if (event->button == 1) //Left Mouse Click
	{
		double dY = (GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - event->y);
		double dX = (event->x - GUIDataContainer::coords[GUIDataContainer::selectedTile][0]);

		double param = dY / dX;
		double slope = atan(param) * 180.0 / M_PI;
		double setX, setY;
		int setPath;
		double dist = sqrt(dY * dY + dX * dX);

		bool changedTile = false;
		double distance = sqrt(dY * dY + dX * dX);
		double newdY, newdX, newDistance;
		for (int i = 0; i < GUIDataContainer::count; i++)
		{
			newdY = (GUIDataContainer::coords[i][1] - event->y);
			newdX = (event->x - GUIDataContainer::coords[i][0]);

			newDistance = sqrt(newdY * newdY + newdX * newdX);
			if (distance > newDistance)
			{
				distance = newDistance;
				if (distance < GUIDataContainer::sideLength * sqrt(3) / 2)	// If click is inside hex
				{
					GUIDataContainer::selectedTile = i;
					changedTile = true;
				}
			}
		}
		if (!changedTile)
		{
			if (dist > GUIDataContainer::sideLength * sqrt(3) / 2)
			{
				if (abs(slope) >= 60)
				{
					if (dY < 0)	// Bottom
					{
						setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0];
						setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * sqrt(3);
						setPath = 0;
					}
					else  // Top
					{
						setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0];
						setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::sideLength * sqrt(3);
						setPath = 3;
					}
				}
				else
				{
					if (dY < 0)
					{
						if (dX > 0)	// Bottom Right
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 5;
						}
						else	// Bottom Left
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] - GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] + GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 1;
						}
					}
					else
					{
						if (dX > 0)	// Top Right
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] + GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 4;
						}
						else	// Top Left
						{
							setX = GUIDataContainer::coords[GUIDataContainer::selectedTile][0] - GUIDataContainer::sideLength * 1.5;
							setY = GUIDataContainer::coords[GUIDataContainer::selectedTile][1] - GUIDataContainer::sideLength * sqrt(3) / 2;
							setPath = 2;
						}
					}
				}
				bool pointExists = false;
				for (int i = 0; i < GUIDataContainer::count; i++)
				{
					if (round(GUIDataContainer::coords[i][0]) == round(setX) && round(GUIDataContainer::coords[i][1]) == round(setY))
					{
						pointExists = true;
						break;
					}
				}
				if (!pointExists)
				{
					vector<double> test;
					test.push_back(setX);
					test.push_back(setY);
					GUIDataContainer::coords.push_back(test);

					GUIDataContainer::status.push_back((int)0);
					GUIDataContainer::endState.push_back((int)50);
					GUIDataContainer::startTime.push_back((int)0);
					GUIDataContainer::riseTime.push_back((int)1);


					GUIDataContainer::path[GUIDataContainer::count - 1] = setPath;

					GUIDataContainer::selectedTile = GUIDataContainer::count;
					GUIDataContainer::count += 1;
					changeScale = true;
				}
			}
			else	// If inside the hexagon, cycle states
			{
				GUIDataContainer::status[GUIDataContainer::selectedTile] = (GUIDataContainer::status[GUIDataContainer::selectedTile] + 1) % 4;
				// rotate from healthy -> user congested -> demand congested -> failing
				if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 0)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = 50;
				}
				else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 1)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = GUIDataContainer::congestionState;
				}
				else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 2)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = GUIDataContainer::congestionState;
				}
				else if (GUIDataContainer::status[GUIDataContainer::selectedTile] == 3)
				{
					GUIDataContainer::endState[GUIDataContainer::selectedTile] = 0;
				}

			}
		}
	}

	gtk_widget_queue_draw(widget);
	return TRUE;
}


static gboolean on_draw_event(GtkWidget * widget, cairo_t * cr, gpointer user_data)
{
	drawHex(cr);
	return FALSE;
}

static void getNeighbors()
{
	for (int k = 0; k < GUIDataContainer::count; k++)	// Wipes the neighbors so it doesn't retain old neighbors
	{
		GUIDataContainer::neighbors[k].erase(GUIDataContainer::neighbors[k].begin(), GUIDataContainer::neighbors[k].end());
	}
	for (int n = 0; n < GUIDataContainer::count; n++)
	{
		for (int i = 0; i < GUIDataContainer::count; i++)
		{
			if (i != n && round(distance(GUIDataContainer::coords[i][0], GUIDataContainer::coords[i][1], GUIDataContainer::coords[n][0], GUIDataContainer::coords[n][1])) <= round(GUIDataContainer::sideLength * sqrt(3)))
			{
				GUIDataContainer::neighbors[n].push_back(make_pair(i, side(GUIDataContainer::coords[i][0], GUIDataContainer::coords[i][1], GUIDataContainer::coords[n][0], GUIDataContainer::coords[n][1])));
			}
		}
	}
}

static bool deletionValid(int tile)
{
	bool* nodeExplored = new bool[GUIDataContainer::count];
	for (int n = 0; n < GUIDataContainer::count; n++)
		nodeExplored[n] = false;

	std::vector<int> nodesToExplore = { (tile + 1) % GUIDataContainer::count };	//element, guaranteed to exist.

	int N;
	while (!nodesToExplore.empty())
	{
		N = nodesToExplore.back();
		nodesToExplore.pop_back();

		if (N == tile)
			continue;

		nodeExplored[N] = true;

		for (int c = 0; c < GUIDataContainer::neighbors[N].size(); c++)
		{
			if (!nodeExplored[GUIDataContainer::neighbors[N][c].first])
				nodesToExplore.push_back(GUIDataContainer::neighbors[N][c].first);
		}
	}
	for (int n = 0; n < GUIDataContainer::count; n++)
		if (nodeExplored[n] == false && n != tile)
			return false;

	return true;
}

static float distance(double x1, double y1, double x2, double y2)
{
	double dY = y2 - y1;
	double dX = x2 - x1;
	double distance = sqrt(dY * dY + dX * dX);
	return distance;
}

static int side(double x1, double y1, double x2, double y2)
{
	double dY = y1 - y2;
	double dX = x1 - x2;
	double angle = atan(dY / dX);
	
	if (dX >= 0 && dY < 0)
		angle += 2*M_PI;
	else if (dX < 0)
		angle += M_PI;
	int side = ((int)(angle / (M_PI / 3)));
	side = (( 7 - side) % 6) + 1;
	return ((10 - side) % 6) + 1;
}

bool addParams()
{
	// get the text from each entry box and add the text to the glob structure
	try
	{
		// int
		GUIDataContainer::bsLen = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.baseStationSide)));
		GUIDataContainer::antNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.antennaNumber)));
		GUIDataContainer::transNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.transceiverNum)));
		GUIDataContainer::uePerAnt = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.userEquipPerAntenna)));
		GUIDataContainer::simLen = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationLength)));
		GUIDataContainer::simNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationNumber)));
		GUIDataContainer::simStartNum = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationStart)));
		GUIDataContainer::bufSizeInSeconds = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.bufferSize)));
		GUIDataContainer::alertState = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.alertEntry)));
		GUIDataContainer::congestionState = stoi(gtk_entry_get_text(GTK_ENTRY(entryBoxes.congestionEntry)));
		// string
		GUIDataContainer::simName = gtk_entry_get_text(GTK_ENTRY(entryBoxes.simulationSaveName));
		
		if(GUIDataContainer::bsLen < 5 || GUIDataContainer::bsLen > 20)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid Basestation length");
			return false;
		}
		if(GUIDataContainer::antNum < 1 || GUIDataContainer::antNum > 6)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid antenna number");
			return false;
		}
		if(GUIDataContainer::transNum < 80 || GUIDataContainer::transNum > 200)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid number of transceivers");
			return false;
		}
		if(GUIDataContainer::alertState < 70 || GUIDataContainer::alertState > 200)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid alert level");
			return false;
		}
		if(GUIDataContainer::congestionState < 70 || GUIDataContainer::congestionState > 200)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid congestion level");
			return false;
		}
		if(GUIDataContainer::simLen < 1)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid simulation length");
			return false;
		}
		if(GUIDataContainer::simNum < 1)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid simulation number");
			return false;
		}
		if(GUIDataContainer::simStartNum < 0 )
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid simulation start number");
			return false;
		}
		if(GUIDataContainer::bufSizeInSeconds < 1)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "Invalid buffer size");
			return false;
		}
		
		if(GUIDataContainer::simName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_") != std::string::npos)
		{
			UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "The name of the simulator contains one or more invalid characters");
			return false;
		}
		
		return true;
	}
	catch (const exception & ex)
	{
		UserMessage(GTK_WINDOW(WINDOWS.SimParamWindow), "An invalid parameter was entered, try again");
		return false;
	}

}

void getDimensions()
{
	// create screenGeo object that contains window length and width
	GdkRectangle screenGeo;
	GdkDisplay* gdkDisplay = gdk_display_get_default();
	GdkMonitor* monitor = gdk_display_get_primary_monitor(gdkDisplay);
	gdk_monitor_get_geometry(monitor, &screenGeo);

	// Add screen dimensions to screen struct object to be used across the project
	SCREEN.WIDTH = screenGeo.width;
	SCREEN.HEIGHT = screenGeo.height;
	
	// Use the modified CSS file if the screen is lower resolution
	if(SCREEN.WIDTH < 1920)
		StyleSheets.cssPath = "SHNSim_SmallScreen.css";
}

GtkWidget* UserMessage(GtkWindow* window, string message)
{	
	// declare and configure dialog window	
	GtkWidget* confirmation = gtk_dialog_new();
	gtk_window_set_modal(GTK_WINDOW(confirmation), true);
	gtk_window_set_transient_for(GTK_WINDOW(confirmation),GTK_WINDOW(window));
	gtk_container_set_border_width(GTK_CONTAINER(confirmation), 10);
	gtk_window_set_title(GTK_WINDOW(confirmation), "Message");
	
	// retrieve and configure content area portion of dialog window
	GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(confirmation));
	gtk_container_set_border_width(GTK_CONTAINER(contentArea), 10);
	
	// declare an output label with the passed message
	GtkWidget* processedMessage = gtk_label_new(message.c_str());
	
	// declare and configure "OK" button
	GtkWidget* okBtn = gtk_button_new_with_label("OK");
	g_signal_connect_swapped(okBtn, "clicked", G_CALLBACK(gtk_widget_destroy), confirmation);

	// create a container to hold dialog window objects
	GtkWidget* container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); 
	
	// pack widgets into the container
	gtk_box_pack_start(GTK_BOX(container), processedMessage, 0, 1, 15);
	gtk_box_pack_start(GTK_BOX(container), okBtn, 0, 1, 15);
	
	// add the container to the content area portion of the dialog window	
	gtk_container_add(GTK_CONTAINER(contentArea), container);
	
	// load color settings from appropriate css file
	gtk_widget_set_name(confirmation, "popupWindow");
	
	GtkCssProvider* provider = gtk_css_provider_new();
	if(gtk_css_provider_load_from_path(provider, StyleSheets.cssPath.c_str(), NULL))
	{	
		// window background
		gtk_style_context_add_provider(gtk_widget_get_style_context(confirmation), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// labels
		gtk_style_context_add_provider(gtk_widget_get_style_context(processedMessage), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		
		// button
		gtk_style_context_add_provider(gtk_widget_get_style_context(okBtn), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	
	// make window visible
	gtk_widget_show_all(confirmation);
	
	return confirmation;
}

void displayInformation(GtkWidget* widget, GtkWindow* window)
{
	
	string msg = "1st Window:\n   (1) Left Click to add eNodeB\n   (2) Left Click on numbers to change eNodeB condition\n   (3) Right Click on eNodeB to delete it";
	msg += "\n\nColor Code:\n   Green = Healthy\n   Yellow = User Congested\n   Orange = Demand Congested\n   Red = Failing";
	msg += "\n\n** See accompanying report for parameter descriptions and more details**";
	
	UserMessage(window, msg);
	
}

void updateBsParams()
{
	int START_TIME, RISE_TIME, END_STATE, ALERT_STATE, CONGESTION_STATE;
	string STATUS;
	
	// make sure entered values are integers
	try
	{
		ALERT_STATE = stoi(gtk_entry_get_text(GTK_ENTRY(alertLvlTxt)));
		CONGESTION_STATE = stoi(gtk_entry_get_text(GTK_ENTRY(congestionLvlTxt)));

		START_TIME = stoi(gtk_entry_get_text(GTK_ENTRY(realTime.startTimeEntry)));
		RISE_TIME = stoi(gtk_entry_get_text(GTK_ENTRY(realTime.riseTimeEntry)));
		END_STATE = stoi(gtk_entry_get_text(GTK_ENTRY(realTime.endStateEntry)));
		
		STATUS = gtk_label_get_text(GTK_LABEL(realTime.endStatusDisplayLbl));		
		
		// return false if entered values are outside of the required bounds
		if(START_TIME < 0 || RISE_TIME < 1 || (END_STATE > 200) || (ALERT_STATE < 55) || (ALERT_STATE > CONGESTION_STATE - 5) || (CONGESTION_STATE < ALERT_STATE + 5) || (CONGESTION_STATE > 200))
		{
			UserMessage(GTK_WINDOW(WINDOWS.DrawingWindow), "Make sure all entered parameters are within the specified bounds");
			return;
		}
		else if((GUIDataContainer::status[GUIDataContainer::selectedTile] == 1 || GUIDataContainer::status[GUIDataContainer::selectedTile] == 2) && END_STATE < CONGESTION_STATE || GUIDataContainer::status[GUIDataContainer::selectedTile] == 0 && END_STATE > ALERT_STATE - 5)
		{
			UserMessage(GTK_WINDOW(WINDOWS.DrawingWindow), "Make sure all entered parameters are within the specified bounds");
			return;
		}	
		// otherwise, add the parameters to their appropriate storage vector
		else
		{
			GUIDataContainer::alertState = ALERT_STATE;
			GUIDataContainer::congestionState = CONGESTION_STATE;
			GUIDataContainer::startTime[GUIDataContainer::selectedTile] = START_TIME;
			GUIDataContainer::riseTime[GUIDataContainer::selectedTile] = RISE_TIME;
			GUIDataContainer::endState[GUIDataContainer::selectedTile] = END_STATE;
			
			// Assign a number based on the string representing the current status
			if(STATUS == "Normal")
				GUIDataContainer::status[GUIDataContainer::selectedTile] = 0;
			if(STATUS == "User Congested")
				GUIDataContainer::status[GUIDataContainer::selectedTile] = 1;
			if(STATUS == "Demand Congested")
				GUIDataContainer::status[GUIDataContainer::selectedTile] = 2;
			if(STATUS == "Failing")
				GUIDataContainer::status[GUIDataContainer::selectedTile] = 3;
			
			UserMessage(GTK_WINDOW(WINDOWS.DrawingWindow), "       Updated       ");
		}
	}
	catch(exception &e)
	{
		UserMessage(GTK_WINDOW(WINDOWS.DrawingWindow), "Make sure all entered parameters are integers");
		return;
	}		
}
