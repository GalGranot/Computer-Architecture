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

constexpr int INVALID_ARGS = -1;

LocalHistoryLocalFsmBP bp1;
LocalHistoryGlobalFsmBP bp2;
GlobalHistoryGlobalFsmBP bp3;

bool checkArgs()
{
	return true;
}

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if(checkArgs() == false)
	{
		fprintf(stderr, "Invalid arguments");
		exit(INVALID_ARGS);
	}
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	return true;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{

}

void BP_GetStats(SIM_stats *curStats)
{
	return;
}