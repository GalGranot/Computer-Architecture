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

BP bp;


bool checkArgs(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared)
{
	//FIXME implement
	return true;
}

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if (checkArgs(btbSize, historySize, tagSize, fsmState,
		isGlobalHist, isGlobalTable, Shared) == false)
	{
		fprintf(stderr, "Invalid arguments");
		exit(INVALID_ARGS);
	}
	bp.share = Shared;
	bp.btbSize = btbSize;
	bp.globalFsm = isGlobalTable;
	bp.globalHistory = isGlobalHist;
	bp.historySize = historySize;

	bp.entries.resize(btbSize);
	for (int i = 0; i < bp.entries.size(); i++)
		entries[i] = BtbEntry();
	
	bp.histories.resize(btbSize);
	for (int i = 0; i < bp.histories.size(); i++)
		bp.histories[i] = 0;

	bp.fsms.resize(btbSize);
	int numOfFsms = 1;
	for (int i = 0; i < historySize; i++)
		numOfFsms *= 2;
	for (vector<Fsm> fsmList : fsms)
		fsmList.resize(numOfFsms);

	for (int i = 0; i < bp.fsms.size(); i++)
		for (int j = 0; j < bp.fsms[i].size(); j++)
			bp.fsms[i][j] = Fsm(fsmState);
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	const BtbEntry* entryPtr = bp.findEntry(pc);
	if(entryPtr == nullptr)
	{
		*dst = pc + 4;
		return false;
	}
	if (entryPtr->predict() == nullptr)
	{
		*dst = pc + 4;
		return false;
	}
	pc = entryPtr->target;
	return true;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	BtbEntry* entryPtr = bp.findEntry(pc);
	if (entryPtr == nullptr)
	{
		int entryIndex = bp.calcEntryIndex(pc);
		bp.entries[entryIndex] = BtbEntry();
		update stats;
	}

	update history;
	update stats;
	if applicable - enter entry into btb;
}

void BP_GetStats(SIM_stats *curStats)
{
	*curStats = bp.stats;
	return;
}