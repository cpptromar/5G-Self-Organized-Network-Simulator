output: Antenna.o BaseStation.o BSFailureParams.o DataRateTable.o EnvironmentController.o EnvironmentInitialization.o ErrorTracer.o UERecord.o FileIO.o \
	GUIDataContainer.o GUIMain.o IRPManager.o NetworkLogBuffer.o Setup.o Simulator.o Transceivers.o Transmission.o UEDataBase.o UELogData.o UserEquipment.o
	g++ Antenna.o BaseStation.o BSFailureParams.o DataRateTable.o EnvironmentController.o EnvironmentInitialization.o ErrorTracer.o UERecord.o FileIO.o \
	GUIDataContainer.o GUIMain.o IRPManager.o NetworkLogBuffer.o Setup.o Simulator.o Transceivers.o Transmission.o UEDataBase.o UELogData.o UserEquipment.o -g -o output `pkg-config --cflags --libs gtk+-3.0`

	
	
Antenna.o: Antenna.cpp Antenna.h
	g++ -g -c Antenna.cpp `pkg-config --cflags --libs gtk+-3.0`

BaseStation.o: BaseStation.cpp BaseStation.h
	g++ -g -c BaseStation.cpp `pkg-config --cflags --libs gtk+-3.0`

BSFailureParams.o: BSFailureParams.cpp BSFailureParams.h
	g++ -g -c BSFailureParams.cpp `pkg-config --cflags --libs gtk+-3.0`

DataRateTable.o: DataRateTable.cpp DataRateTable.h
	g++ -g -c DataRateTable.cpp `pkg-config --cflags --libs gtk+-3.0`
	
EnvironmentController.o: EnvironmentController.cpp EnvironmentController.h
	g++ -g -c EnvironmentController.cpp `pkg-config --cflags --libs gtk+-3.0`

EnvironmentInitialization.o: EnvironmentInitialization.cpp
	g++ -g -c EnvironmentInitialization.cpp `pkg-config --cflags --libs gtk+-3.0`

ErrorTracer.o: ErrorTracer.cpp ErrorTracer.h
	g++ -g -c ErrorTracer.cpp `pkg-config --cflags --libs gtk+-3.0`
	
FileIO.o: FileIO.cpp FileIO.h
	g++ -g -c FileIO.cpp `pkg-config --cflags --libs gtk+-3.0`
	
GUIDataContainer.o: GUIDataContainer.cpp
	g++ -g -c GUIDataContainer.cpp `pkg-config --cflags --libs gtk+-3.0`

GUIMain.o: GUIMain.cpp GUIMain.h
	g++ -g -c GUIMain.cpp `pkg-config --cflags --libs gtk+-3.0`
	
IRPManager.o: IRPManager.cpp IRPManager.h
	g++ -g -c IRPManager.cpp `pkg-config --cflags --libs gtk+-3.0`
	
NetworkLogBuffer.o: NetworkLogBuffer.cpp NetworkLogBuffer.h
	g++ -g -c NetworkLogBuffer.cpp `pkg-config --cflags --libs gtk+-3.0`
	
Setup.o: Setup.cpp Setup.h
	g++ -g -c Setup.cpp `pkg-config --cflags --libs gtk+-3.0`
	
Simulator.o: Simulator.cpp GUIDataContainer.h
	g++ -g -c Simulator.cpp `pkg-config --cflags --libs gtk+-3.0`

Transceivers.o: Transceivers.cpp Transceivers.h
	g++ -g -c Transceivers.cpp `pkg-config --cflags --libs gtk+-3.0`

Transmission.o: Transmission.cpp Transmission.h
	g++ -g -c Transmission.cpp `pkg-config --cflags --libs gtk+-3.0`

UEDataBase.o: UEDataBase.cpp UEDataBase.h
	g++ -g -c UEDataBase.cpp `pkg-config --cflags --libs gtk+-3.0`

UELogData.o: UELogData.cpp UELogData.h
	g++ -g -c UELogData.cpp `pkg-config --cflags --libs gtk+-3.0`

UERecord.o: UERecord.cpp UERecord.h
	g++ -g -c UERecord.cpp `pkg-config --cflags --libs gtk+-3.0`

UserEquipment.o: UserEquipment.cpp UserEquipment.h
	g++ -g -c UserEquipment.cpp `pkg-config --cflags --libs gtk+-3.0`
	
clean:
	rm *.o