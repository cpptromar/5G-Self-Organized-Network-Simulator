#pragma once

#include <stdint.h> // needed to use uint32_t on Linux
#include "NetworkLogBuffer.h"
#include "BaseStation.h"

#include "GUIDataContainer.h" // retreive size of eNodeB
#include "ErrorTracer.h"

enum class IRP_BSStatus { normal = 0, almostcongested, congestionDemand, congestionUsers, failure };
enum class IRP_UEMobility { stationary = 0, walking, car };

typedef struct { size_t bsID;  IRP_BSStatus bsStatus; float bsStateDemand; float bsStateSent; } IRP_BSInfo;

class IRPManager
{
protected:
	NetworkLogBuffer Buffer;
	std::vector<IRP_BSInfo> networkStatuses;
	float percentDecrease;
	bool doneHealing;					// Used to tell whether healing portion is done (for offloadUserKPIs)
	
	std::vector<size_t> helperBSs;		// store BSs that can provide aid
	std::vector<size_t> disabledBSs; 	// store BSs that are in need of aid
	
public:
	void InitializeIRPManager();

	const NetworkLogBuffer& getBuffer() const;

	void dataAnalysis();
	void IRPManagerUpdate();
	void IRPDataCollection();
	void PRINTDEBUG();

	void checkStatus(); 	// check whether a BS should be a helper or if it is disabled
	void offloadUser(); 	// specifies the user that should be offloaded, the source BS to offload from, and the destination BS to offload to
	void offloadUserKPIs();	// new self-healing algorithm based on KPIs instead of only BS status
	void finishMovingUsers(); // moves users to which ever base station is closest to them, ignores all self-healing
};


