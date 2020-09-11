output: Antenna.o BaseStation.o BSFailureParams.o DataRateTable.o EnvironmentController.o EnvironmentInitialization.o ErrorTracer.o UERecord.o FileIO.o \
	GUIDataContainer.o GUIMain.o IRPManager.o NetworkLogBuffer.o Setup.o Simulator.o Transceivers.o Transmission.o UEDataBase.o UELogData.o UserEquipment.o
	g++ Antenna.o BaseStation.o BSFailureParams.o DataRateTable.o EnvironmentController.o EnvironmentInitialization.o ErrorTracer.o UERecord.o FileIO.o \
	GUIDataContainer.o GUIMain.o IRPManager.o NetworkLogBuffer.o Setup.o Simulator.o Transceivers.o Transmission.o UEDataBase.o UELogData.o UserEquipment.o -g -o output `pkg-config --cflags --libs gtk+-3.0`

	
	
Antenna.o: Antenna.cpp Antenna.h
	g++ -g -c Antenna.cpp

BaseStation.o: BaseStation.cpp BaseStation.h
	g++ -g -c BaseStation.cpp

BSFailureParams.o: BSFailureParams.cpp BSFailureParams.h
	g++ -g -c BSFailureParams.cpp

DataRateTable.o: DataRateTable.cpp DataRateTable.h
	g++ -g -c DataRateTable.cpp 
	
EnvironmentController.o: EnvironmentController.cpp EnvironmentController.h
	g++ -g -c EnvironmentController.cpp

EnvironmentInitialization.o: EnvironmentInitialization.cpp
	g++ -g -c EnvironmentInitialization.cpp

ErrorTracer.o: ErrorTracer.cpp ErrorTracer.h
	g++ -g -c ErrorTracer.cpp
	
FileIO.o: FileIO.cpp FileIO.h
	g++ -g -c FileIO.cpp
	
GUIDataContainer.o: GUIDataContainer.cpp
	g++ -g -c GUIDataContainer.cpp

GUIMain.o: GUIMain.cpp GUIMain.h
	g++ -g -c GUIMain.cpp `pkg-config --cflags --libs gtk+-3.0`
	
IRPManager.o: IRPManager.cpp IRPManager.h
	g++ -g -c IRPManager.cpp
	
NetworkLogBuffer.o: NetworkLogBuffer.cpp NetworkLogBuffer.h
	g++ -g -c NetworkLogBuffer.cpp
	
Setup.o: Setup.cpp Setup.h
	g++ -g -c Setup.cpp
	
Simulator.o: Simulator.cpp Simulator.h
	g++ -g -c Simulator.cpp

Transceivers.o: Transceivers.cpp Transceivers.h
	g++ -g -c Transceivers.cpp

Transmission.o: Transmission.cpp Transmission.h
	g++ -g -c Transmission.cpp

UEDataBase.o: UEDataBase.cpp UEDataBase.h
	g++ -g -c UEDataBase.cpp

UELogData.o: UELogData.cpp UELogData.h
	g++ -g -c UELogData.cpp

UERecord.o: UERecord.cpp UERecord.h
	g++ -g -c UERecord.cpp

UserEquipment.o: UserEquipment.cpp UserEquipment.h
	g++ -g -c UserEquipment.cpp
	
clean:
	rm *.o
