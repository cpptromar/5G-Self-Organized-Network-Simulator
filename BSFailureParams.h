#pragma once
#include <vector>
#include "BaseStation.h"


/*	
 *	The Transmission struct functions as a simple information container that has 
 *	everything the EnvironmentController needs to maintain a BS. Each BS has a corresponding BSFailureParams.
 *	The BSFailureParams are stored in the Environment Controller's BSRegionControlInfo in the same order as the BSs
 *	are within the Simulators BaseStationList
 *
 *	bsID: the ID of the BS the BSFailureParams pertains to
 *	currentState = (sum of all UE demands in the UEsInRegion for the previous tick) / (Maximum BS datarate)
 *	endState: the target goal, this is the number that the currentState should approach once failure is activated. If the endState is unacheiveable, 
 *			  perhaps due to being unable to add enough users to a BS, the endState will adjust itself to the currentState that was acheived.
 *	currentStatus: this is the current status, this is set to the endstatus at the end of its risetime.
 *	endStatus: this is the target status.
 *	startTime: this is the time that the BS should begin its transition to the endState
 *	riseTime: this is the time it will take for the BS to reach its endState after starting its rise. Must be >= 1 or div/0 will occur.
 *	UEsInregion: This contains all the userIDs that are present within the BSs range. For illustration purposes: suppose the IRPManager does not 
 *				ever transfer any UEs between BSs, then the UEsInRegion will perfectly match the UEs linked to the BS by the EnvironmentController.
 *				If the IRPManager were to transfer a UE from its initial BS to another BS, the UE would still be present in the initial BS's UEsInRegion.
 *				Reason why I did this: Pretend that the EnvironmentController is a force of nature that ensures that a certain BS is Congested. To acheive this
 *				the EnvironmentController magically creates UEs to load down the BS to raise the demand placed upon it. Now consider the IRPManager. 
 *				Its job is basically shuffle users around to allieviate load, and does so by directly looking at the UEs linked to the BS. 
 *				Now, from the POV of the EnvironmentController, the "load" it's trying to place on the BS is measured by looking at the UEsInRegion 
 *				and summing all their demands and comparing that to the max BS Datarate. If the EnvironmentController were measuring the demands of the users
 *				based off those that are DEFINITELY linked to the BS, then it would try and add users or adjust their demands at the same time the IRPManager 
 *				is trying to transfer users based off the same measurement. The result would be the continual addition of users while the IRPManager does the exact opposite.
 *				Clearly this is undesirable. Therefore, the currentState that is measured is only the theoretical load the BS would be under if nothing were done by the IRPManager.
 *				Thus, ideally, with a functioning IRPManager, the BS could be in a healthy or less congested state while the EnvironmentController thinks that the BS
 *				is congested. 
 */

struct BSFailureParams
{
	size_t bsID;
	float currentState, endState;
	BSstatus currentStatus, endStatus;
	uint32_t startTime, riseTime;
	std::vector<size_t> UEsInRegion;
	
	BSFailureParams();
	BSFailureParams(const size_t& bsID, const float&, const float&, const BSstatus&, const BSstatus&, const uint32_t&, const uint32_t&, const std::vector<size_t>);
	BSFailureParams(const BSFailureParams&);
};