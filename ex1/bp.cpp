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
#include <bitset>
#include <vector>
using std::cout;
using std::endl;
using std::vector;

/*=============================================================================
* constexpressings, defines
=============================================================================*/
#define MIN_FIELD_SIZE 1
#define MAX_HISTORY_SIZE 8

#define STRONGLY_NOT_TAKEN 0
#define WEAKLY_NOT_TAKEN 1
#define WEAKLY_TAKEN 2
#define STRONGLY_TAKEN 3

#define NOT_USING_SHARE 0
#define USING_SHARE_LSB 1
#define USING_SHARE_MID 2

#define INVALID_ARGS -1

#define DEFAULT_FIELD 0

/*=============================================================================
* global functions
=============================================================================*/
void printBinary(uint32_t num, char* name)
{
	for(int i = 31; i >= 0; i--)
	{
		bool bitSet = num & (1 << i);
		printf("%d", bitSet ? 1 : 0);
		if(i % 4 == 0)
			printf(" ");
	}
	printf("%s\n", name);
}
// void printBinaryLimited(uint32_t num, char* name, int bitsNum)
// {
// 	vector<int> v;
// 	for(int i = bitsNum; i >= 0; i--)
// 	{
// 		bool bitSet = num & (1 << i);
// 		v.push_back(bitSet ? 1 : 0);
// 	}
// 	int size = v.size();
// 	cout << "v szie = " << size; yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy
// 	for(int i = 0; i < v.size() % 4; i++)
// 		cout << "0";
// 	for(int i = v.size(); i >= 0; i--)
// 	{
// 		cout << (v[i] ? "1" : "0");
// 	}
// 	printf(" %s\n", name);
// }

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

bool validateArgs(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if(btbSize < MIN_FIELD_SIZE ||
	   (btbSize % 2 != 0) ||
	   historySize < MIN_FIELD_SIZE ||
	   historySize > MAX_HISTORY_SIZE ||
	   fsmState < STRONGLY_NOT_TAKEN ||
	   fsmState > STRONGLY_TAKEN ||
	   (!isGlobalTable && Shared != NOT_USING_SHARE)
	)
		return false;
	return true;
}

/*=============================================================================
* classes
=============================================================================*/
struct Fsm
{
	int state;
	Fsm() : state(STRONGLY_NOT_TAKEN) {}
	Fsm(int state) : state(state) {}
	void update(bool taken)
	{
		if(taken && state != STRONGLY_TAKEN)
			state++;
		else if(!taken && state != STRONGLY_NOT_TAKEN)
			state--;
	}
	bool predict() { return state >= WEAKLY_TAKEN; }
};

struct TableEntry
{
	uint32_t tag;
	uint32_t pc;
	uint32_t target;
	unsigned history;
	unsigned* globalHistory;
	unsigned historySize;
	bool valid;
	vector<Fsm> fsms;
	bool isGlobalHistory;

	TableEntry(uint32_t pc, uint32_t target, unsigned btbSize, unsigned tagSize, unsigned* globalHistory, unsigned historySize, int fsmState, bool valid) :
		pc(pc), target(target), history(0), globalHistory(globalHistory),
		historySize(historySize), valid(valid)
	{
		tag = calcTagFromPc(pc, btbSize, tagSize);
		int fsmsNum = std::pow(2, historySize);
		fsms.resize(fsmsNum);
		for(int i = 0; i < fsmsNum; i++)
			fsms[i] = Fsm(fsmState);
		if(globalHistory == nullptr)
			isGlobalHistory = false;
		else
			isGlobalHistory = true;
	}
	TableEntry(uint32_t pc, uint32_t target, unsigned btbSize, unsigned tagSize, unsigned* globalHistory, unsigned historySize, vector<Fsm>& fsms, bool valid) :
		pc(pc), target(target), 
		history(0), globalHistory(globalHistory), historySize(historySize), valid(false), fsms(fsms)
	{
		this->valid = valid;
		tag = calcTagFromPc(pc, btbSize, tagSize);
		if(globalHistory == nullptr)
			isGlobalHistory = false;
		else
			isGlobalHistory = true;
	}

	unsigned getHistory()
	{
		unsigned result = isGlobalHistory? *globalHistory : history;
		int mask = (1 << historySize) - 1;
		result &= mask;
		return result;
	}
	void update(bool taken)
	{
		unsigned* historyPtr = isGlobalHistory ? globalHistory : &history;
		*historyPtr <<= 1;
		*historyPtr += taken ? 1 : 0;
	}
	bool predict()
	{
		return fsms[getHistory()].predict();
	}

	void print()
	{
		printBinary(pc, "pc");
		printBinary(tag, "tag");
		printBinary(target, "target");
		printBinary(history, "history");
		cout << "valid = " << (valid ? "true" : "false") << endl;
	}
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

	BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared) :
		btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(fsmState),
		isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), shared(Shared)
	{
		stats = SIM_stats{.br_num = 0, .flush_num = 0, .size = 0};
		unsigned* globalHistoryArg = isGlobalHist ? &globalHistory : nullptr;
		if(isGlobalTable)
		{
			for(int j = 0; j < std::pow(2, historySize); j++)
				globalFsms[j] = Fsm(fsmState);
			for(unsigned int i = 0; i < btbSize; i++)
				entries.push_back(TableEntry(DEFAULT_FIELD, DEFAULT_FIELD, btbSize, tagSize, globalHistoryArg, historySize, globalFsms, false));
			//construct entries with global fsm
		}
		else
		{
			for(unsigned int i = 0; i < btbSize; i++)
				entries.push_back(TableEntry(DEFAULT_FIELD, DEFAULT_FIELD, btbSize, tagSize, globalHistoryArg, historySize, fsmState, false));
		}
	}

	void print()
	{
		cout << "\n\n========= Printing entries =========\n\n";
		cout << "BP Paramaters:" << endl <<
		"btbSize = " << btbSize << endl <<
		"historySize = " << historySize << endl <<
		"tagSize = " << tagSize << endl <<
		"fsmState = " << fsmState << endl <<
		"isGlobalHist = " << isGlobalHist << endl <<
		"isGlobalTable = " << isGlobalTable << endl <<
		"shared = " << shared << "\n" << endl;
		int i = 0;
		for(TableEntry& te : entries)
		{
			cout << "=== entry " << i++ << " ===" << endl;
			te.print();
		}
		cout << "\n\n========= End of entries =========\n";
	}

	TableEntry& findEntryByPc(uint32_t pc) { return entries[calcTableIndexFromPc(pc, btbSize)]; }

	bool predict(uint32_t pc, uint32_t* dst)
	{
		TableEntry& te = findEntryByPc(pc);
		bool taken = false;
		if(te.valid) //known jump - use predict
		{
			taken = te.predict();
			*dst = te.target;
		}
		else //unknown junp
			*dst = pc + 4;
		return taken;
		if(!te.valid)
		{
			//handle new entry

		}
		else
			taken = te.predict();
		*dst = taken ? te.target : (pc + 4);
		return taken;
	}

	void updateStats(bool predictionCorrect)
	{
		stats.br_num++;
		if(!predictionCorrect)
			stats.flush_num++;
	}

	void update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
	{
		TableEntry& te = findEntryByPc(pc);
		if(!te.valid) //new jump - add it
		{
			te.tag = calcTagFromPc(pc, btbSize, tagSize);
			te.pc = pc;
			te.target = targetPc;
			te.history = 0;
			te.valid = true;
		}
		te.update(taken);
		bool predictionCorrect = targetPc == targetPc;
		updateStats(predictionCorrect);
	}
};

BranchPredictor* bp;

/*=============================================================================
* bp public functions
=============================================================================*/
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if(!validateArgs(btbSize, historySize, tagSize, fsmState,
		isGlobalHist, isGlobalTable, Shared))
	{
		fprintf(stderr, "invalid arguments\n");
		return INVALID_ARGS;
	}
	bp = new BranchPredictor(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
	bp->print();
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	return;
}

void BP_GetStats(SIM_stats *curStats)
{
	delete bp;
	return;
}

