//bp.cpp

/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */

/*=============================================================================
* includes, usings
=============================================================================*/
#include "bp_api.h"
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>

using std::cout;
using std::endl;
using std::vector;

/*=============================================================================
* constexpressings, defines
=============================================================================*/
constexpr int MIN_FIELD_SIZE = 1;
constexpr int MAX_HISTORY_SIZE = 8;
constexpr int PC_SIZE = 30;

constexpr int STRONGLY_NOT_TAKEN = 0;
constexpr int WEAKLY_NOT_TAKEN = 1;
constexpr int WEAKLY_TAKEN = 2;
constexpr int STRONGLY_TAKEN = 3;

constexpr int NOT_USING_SHARE = 0;
constexpr int USING_SHARE_LSB = 1;
constexpr int USING_SHARE_MID = 2;

constexpr int INVALID_ARGS = -1;
constexpr int DEFAULT_FIELD = 0;

/*=============================================================================
* global functions
=============================================================================*/
// void printBinary(uint32_t num, char* name)
// {
// 	for (int i = 31; i >= 0; i--)
// 	{
// 		bool bitSet = num & (1 << i);
// 		printf("%d", bitSet ? 1 : 0);
// 		if (i % 4 == 0)
// 			printf(" ");
// 	}
// 	printf("%s\n", name);
// }
//  void printBinaryLimited(uint32_t num, char* name, int bitsNum)
//  {
//  	vector<int> v;
//  	for(int i = bitsNum; i >= 0; i--)
//  	{
//  		bool bitSet = num & (1 << i);
//  		v.push_back(bitSet ? 1 : 0);
//  	}
//  	int size = v.size();
//  	cout << "v szie = " << size;
//  	for(int i = 0; i < v.size() % 4; i++)
//  		cout << "0";
//  	for(int i = v.size(); i >= 0; i--)
//  	{
//  		cout << (v[i] ? "1" : "0");
//  	}
//  	printf(" %s\n", name);
//  }

uint32_t calcTagFromPc(uint32_t pc, unsigned btbSize, unsigned tagSize)
{
	uint32_t result = pc;
	int btbSizeInBits = std::log2(btbSize);
	result >>= (2 + btbSizeInBits);
	int mask = (1 << tagSize) - 1;
	result &= mask;
	return result;
}

int calcTableIndexFromPc(uint32_t pc, unsigned btbSize)
{
	uint32_t result = pc;
	int btbSizeInBits = std::log2(btbSize);
	result >>= 2;
	int mask = (1 << btbSizeInBits) - 1;
	result &= mask;
	return result;
}

int calcBtbBitSize(unsigned btbSize, unsigned historySize, bool isGlobalHist,
    bool isGlobalTable, int tagSize)
{
    int validBitsSize = 1 * btbSize;
    int targetSize = PC_SIZE;
    int fsmSize = 2 * std::pow(2, historySize);
    int isGlobalHistFlag = isGlobalHist ? 0 : 1;
    int fsmsSize = isGlobalTable ? fsmSize : fsmSize * btbSize;
	int tableSize = btbSize * (tagSize + targetSize + isGlobalHistFlag * historySize)
        + (1 - isGlobalHistFlag) * historySize;
    return tableSize + fsmsSize + validBitsSize;
}

/*=============================================================================
* classes
=============================================================================*/
struct Fsm
{
	int state;
	Fsm(int state) : state(state) {}
	void update(bool taken)
	{
		if (taken && state != STRONGLY_TAKEN)
			state++;
		else if (!taken && state != STRONGLY_NOT_TAKEN)
			state--;
	}
	bool predict() { return state >= WEAKLY_TAKEN; }
};

struct TableEntry
{
	unsigned history;
	unsigned* globalHistory;
	unsigned historySize;
	bool valid;
	vector<Fsm> fsms;
	vector<Fsm>* globalTable;
	bool isGlobalHistory;
	bool isGlobalTable;
	unsigned shared;
	uint32_t tag;
	uint32_t pc;
	uint32_t target;

	TableEntry(uint32_t pc, uint32_t target, unsigned btbSize, unsigned tagSize,
		unsigned* globalHistory, unsigned historySize, vector<Fsm>* globalTable,
        bool isGlobalTable, unsigned shared, int fsmState, bool valid) :
		pc(pc),
		target(target),
		history(0),
		globalHistory(globalHistory),
		historySize(historySize),
		valid(valid),
		fsms(vector<Fsm>(std::pow(2, historySize), Fsm(fsmState))),
		globalTable(globalTable),
		isGlobalHistory((globalHistory == nullptr) ? false : true),
		isGlobalTable(isGlobalTable),
		shared(shared),
		tag(calcTagFromPc(pc, btbSize, tagSize)) {}

	TableEntry(uint32_t pc, uint32_t target, unsigned btbSize, unsigned tagSize,
		unsigned* globalHistory, unsigned historySize, vector<Fsm>* globalTable,
        bool isGlobalTable, unsigned shared, vector<Fsm>& fsms, bool valid) :
		pc(pc),
		target(target),
		history(0),
		globalHistory(globalHistory),
		historySize(historySize),
		valid(valid),
		fsms(fsms),
		globalTable(globalTable),
		isGlobalHistory((globalHistory == nullptr) ? false : true),
		isGlobalTable(isGlobalTable),
		shared(shared),
		tag(calcTagFromPc(pc, btbSize, tagSize)) {}

	unsigned getHistory()
	{
		int mask = (1 << historySize) - 1;
		unsigned result = 0;
		if ((!isGlobalTable) || (shared == NOT_USING_SHARE))
			result = isGlobalHistory ? *globalHistory : history;
        
		else if (isGlobalTable && (shared == USING_SHARE_LSB))
		{
			uint32_t pcLsb = pc >> 2;
			pcLsb &= mask;
			result = isGlobalHistory ? (*globalHistory ^ pcLsb) : (history ^ pcLsb);
		}
		else if (isGlobalTable && (shared == USING_SHARE_MID))
		{
			uint32_t pcMid = pc >> 16;
			pcMid &= mask;
			result = isGlobalHistory ? (*globalHistory ^ pcMid) : (history ^ pcMid);
		}
		result &= mask;
		return result;
	}

	void update(bool taken)
	{
		unsigned fsmIndex = getHistory();

		if (isGlobalTable)
			(*globalTable)[fsmIndex].update(taken);
		else
			fsms[fsmIndex].update(taken);
        
		unsigned* historyPtr = isGlobalHistory ? globalHistory : &history;
		*historyPtr <<= 1;
		*historyPtr += taken ? 1 : 0;
	}

	bool predict() {
		unsigned result = 0;
		unsigned fsmIndex = getHistory();
		if (isGlobalTable) 
			result = (*globalTable)[fsmIndex].predict();
		else 
			result = fsms[fsmIndex].predict();
		return result;
	}

	// void print()
	// {
	// 	cout << std::hex;
	// 	cout << "0x" << pc << " pc\n";
	// 	cout << "0x" << tag << " tag\n";
	// 	cout << "0x" << target << " target\n";
	// 	printBinary(history, "history");
	// 	cout << "valid = " << (valid ? "true" : "false") << endl;
	// 	int i = 0;
	// 	if (isGlobalTable)
	// 	{
	// 		for (Fsm& fsm : (*globalTable))
	// 			cout << "Fsm " << i++ << " state = " << fsm.state << endl;
	// 	}
	// 	else {
	// 		for (Fsm& fsm : fsms)
	// 			cout << "Fsm " << i++ << " state = " << fsm.state << endl;
	// 	}
	// 	cout << endl;
	// }
};

struct BranchPredictor
{
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	unsigned fsmState;
	bool isGlobalHist;
	bool isGlobalTable;
	int shared;
	unsigned globalHistory;
	vector<TableEntry> entries;
	vector<Fsm> globalFsms;
	SIM_stats stats;

	BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize,
		unsigned fsmState, bool isGlobalHist, bool isGlobalTable, int Shared) :
		btbSize(btbSize),
		historySize(historySize),
		tagSize(tagSize),
		fsmState(fsmState),
		isGlobalHist(isGlobalHist),
		isGlobalTable(isGlobalTable),
		shared(Shared)
	{
		//if globalHistoryArg != nullptr, it's a pointer to the global history register
		//if globalHistoryArg == nullptr. the entries update their own history
		unsigned* globalHistoryArg = isGlobalHist ? &globalHistory : nullptr;

		if (isGlobalTable)
		{
			globalFsms = vector<Fsm>(std::pow(2, historySize), Fsm(fsmState));
			//construct entries with global fsm vector
			entries = vector<TableEntry>(btbSize, TableEntry(DEFAULT_FIELD,
				DEFAULT_FIELD, btbSize, tagSize, globalHistoryArg, historySize,
                &globalFsms, isGlobalTable, shared, globalFsms, false));
		}
		else
		{
			//construct entries with local fsm vector
			entries = vector<TableEntry>(btbSize, TableEntry(DEFAULT_FIELD,
				DEFAULT_FIELD, btbSize, tagSize, globalHistoryArg, historySize,
                nullptr, isGlobalTable, shared, fsmState, false));
		}

		// initialize stats:
		stats.br_num = 0;
		stats.flush_num = 0;
        //FIXME remove this
		// calculate size of BTB:
		// int validBitsSize = 1 * btbSize;
		// int targetSize = PC_SIZE;
		// int fsmSize = 2 * std::pow(2, historySize);
		// int isGlobalHistFlag = isGlobalHist ? 0 : 1;
		// int fsmsSize = isGlobalTable ? fsmSize : fsmSize * btbSize;
		// int tableSize = btbSize * (tagSize + targetSize + isGlobalHistFlag * historySize) + (1 - isGlobalHistFlag) * historySize;
        stats.size = calcBtbBitSize(btbSize, historySize, isGlobalHist, isGlobalTable, tagSize);
    }
	// void print()
	// {
	// 	cout << "\n\n========= Printing entries =========\n\n";
	// 	int i = 0;
	// 	for (TableEntry& te : entries)
	// 	{
	// 		cout << "=== entry " << i++ << " ===" << endl;
	// 		te.print();
	// 	}
	// 	cout << "\n\n========= End of entries =========\n";
	// }

	TableEntry& findEntryByPc(uint32_t pc) { return entries[calcTableIndexFromPc(pc, btbSize)]; }

	bool predict(uint32_t pc, uint32_t* dst)
	{
		TableEntry& te = findEntryByPc(pc);
		uint32_t newTag = calcTagFromPc(pc, btbSize, tagSize);
		bool availableTag = (tagSize == 0) || (te.tag == newTag);
		bool canPredict = (te.valid && availableTag);
		bool taken = canPredict ? te.predict() : false;
		*dst = taken ? te.target : pc + 4;
		return taken;
	}

	void updateStats(bool doFlush)
	{
		stats.br_num++;
		if (doFlush)
			stats.flush_num++;
	}

	void update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
	{
		TableEntry& te = findEntryByPc(pc);
		uint32_t newTag = calcTagFromPc(pc, btbSize, tagSize);

		if (!te.valid || (te.valid && (te.tag != newTag))) //new or indistinguishable jump - initialize it
		{
			if (!isGlobalTable) //if local table - init fsms
				te.fsms = vector<Fsm>(std::pow(2, historySize), Fsm(fsmState));
			if (!isGlobalHist)
				te.history = 0;
			te.tag = newTag;
			te.pc = pc;
			te.target = targetPc;
			te.valid = true;
		}
		bool prediction = te.predict();
		bool flush1 = (taken == true) && (pred_dst != targetPc);
		bool flush2 = (taken == false) && (pred_dst != pc + 4);
		bool flush = flush1 || flush2;
		if (te.valid && (te.tag == newTag))
			te.update(taken);
		te.pc = pc;
		te.target = targetPc;
		updateStats(flush);
	}
};

BranchPredictor* bp;

/*=============================================================================
* bp public functions
=============================================================================*/
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if (!isGlobalTable && Shared != NOT_USING_SHARE)
		Shared = NOT_USING_SHARE;
	bp = new BranchPredictor(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t* dst) { return bp->predict(pc, dst); }

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	bp->update(pc, targetPc, taken, pred_dst);
}

void BP_GetStats(SIM_stats* curStats)
{
	*curStats = bp->stats;
	delete bp;
	return;
}

