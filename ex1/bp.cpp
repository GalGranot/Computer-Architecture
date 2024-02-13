/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */
#include "bp_api.h"
#include <vector>
using std::vector;

#define STRONGLY_NOT_TAKEN 0
#define WEAKLY_NOT_TAKEN 1
#define WEAKLY_TAKEN 2
#define STRONGLY_TAKEN 3
#define LOCAL 0
#define GLOBAL 1

class Fsm
{
private:
	unsigned state = WEAKLY_NOT_TAKEN;
public:
	Fsm() : state(WEAKLY_NOT_TAKEN) {}
	void updateState(bool taken)
	{
		if(taken && state != STRONGLY_TAKEN)
			state += 1;
		else if(!taken && state != STRONGLY_NOT_TAKEN)
			state -= 1;
	}
	unsigned getState() { return state; }
} Fsm;

class BtbEntry
{
private:
	const uint32_t tag;
	const uint32_t target;
	uint32_t history = 0;
public:
	BtbEntry(uint32_t tag, uint32_t target, uint32_t history)
		: tag(tag), target(target), history(history) {}
	
	void updateHistory(bool taken)
	{
		history <<= 1;
		history += (taken ? 1 : 0);
	}
	uint32_t getTag { return tag; }
};

class Btb
{
private:
	bool type;
	unsigned btbSize;
	bool share;
	vector<BtbEntry> btbEntries;
public:
	Btb(bool type, unsigned btbSize, bool share) : type(type), btbSize(btbSize), share(share) {}
	BtbEntry getEntry(const uint32_t tag)
	{
		if found return BtbEntry
		if not found return null
	}
	void handleEntry(BtbEntry& btbEntry)
	{
		if(getEntry(btbEntry.getTag()) == nullptr)
		{
			add entry;
			return;
		}
		update history or handle collison;
		
	}
};

class BranchPredictor
{
private:
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	unsigned fsmState;
	bool isGlobalHist;
	bool isGlobalTable;
	int Shared;
	SIM_stats stats;

	Btb btb;




public:
	BranchPredictor() {}
	BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared) :
			btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(fsmState),
			isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), Shared(Shared), stats(/*fixme*/)
			{}
	
	bool predict(uint32_t pc, uint32_t* dst)
	{
		//do computation
		if(jump)
			return true;
		else
			return false;
	}

	void update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
	{

	}
	SIM_stats getStats()
	{

	}


} BranchPredictor;

BranchPredictor bp();

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