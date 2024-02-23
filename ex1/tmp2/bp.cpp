/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */
/*=============================================================================
* Need to take care of 5 options:
* 1) local history + local FSM + not using share
* 2) local history + global FSM + not using share
* 3) local history + global FSM + lshare
* 4) global history + global FSM + not using share
* 5) global history + global FSM + gshare
=============================================================================*/
#include "bp_api.h"
#include "bp_classes.h"
#include <stdio.h>
#include <stdlib.h>

constexpr int INVALID_ARGS = -1;
constexpr int NOT_USING_SHARE = 0;
constexpr int USING_SHARE_LSB = 1;
constexpr int USING_SHARE_MID = 2;

BranchPredictor* bp;

bool validateArgs(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if (btbSize < 2 || 
		btbSize % 2 != 0 ||
		historySize < 1 ||
		tagSize < 1 ||
		fsmState < STRONGLY_NOT_TAKEN ||
		fsmState > STRONGLY_TAKEN ||
		Shared < NOT_USING_SHARE ||
		Shared > USING_SHARE_MID ||
		(Shared != NOT_USING_SHARE && !isGlobalTable))
		return false;
	return true;
}

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared)
{
	printf("here1");
	if(!validateArgs(btbSize, historySize, tagSize, fsmState,
			isGlobalHist,isGlobalTable, Shared))
	{
		fprintf(stderr, "Invalid arguments");
		exit(INVALID_ARGS);
	}
	printf("here2");
	*bp = BranchPredictor(btbSize, historySize, tagSize, fsmState,
			isGlobalHist,isGlobalTable, Shared);
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	bool prediction = bp->bp1.predict(pc);
	//FIXME	update dst
	return prediction;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	bp->bp1.update(pc, targetPc, taken, pred_dst);
}

void BP_GetStats(SIM_stats *curStats)
{
	*curStats = bp->bp1.getStats();
}