/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */
#include <vector>
#include "bp_api.h"
#include "bp_classes.h"

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared)
{
	//check arguments
	if(args invalid)
		return -1;
	bp = new bp....
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	return bp.predict(pc, dst);
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	bp.update(pc, targetPc, taken, pred_dst);
}

void BP_GetStats(SIM_stats *curStats)
{
	*curStats = bp.getStats();
	return;
}