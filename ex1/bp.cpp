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
	   (!isGlobalTable || Shared != NOT_USING_SHARE)
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

	TableEntry(uint32_t pc, uint32_t target, unsigned btbSize, unsigned tagSize, unsigned* globalHistory, unsigned historySize, int fsmState) :
		tag(calcTagFromPc(pc, btbSize, tagSize)), pc(pc), target(target), history(0), globalHistory(globalHistory),
		historySize(historySize),valid(true)
	{
		int fsmsNum = std::pow(2, historySize);
		fsms.resize(fsmsNum);
		for(int i = 0; i < fsmsNum; i++)
			fsms[i] = Fsm(fsmState);
		if(globalHistory == nullptr)
			isGlobalHistory = false;
		else
			isGlobalHistory = true;
	}
	TableEntry(uint32_t pc, uint32_t target, unsigned btbSize, unsigned tagSize, unsigned* globalHistory, unsigned historySize, vector<Fsm> fsms) :
		tag(calcTagFromPc(pc, btbSize, tagSize)), pc(pc), target(target), 
		history(0), globalHistory(globalHistory), historySize(historySize), valid(true), fsms(fsms) 
	{
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
	// void print()
	// {
	// 	cout << std::hex; 
	// 	cout << "pc 0x" << pc << endl;
	// 	cout << "target 0x" << target << endl;
	// 	printBinary(history, "history");
	// 	cout << "valid = " << (valid ? "true" : "false") << endl;
	// }
};

// struct BranchPredictor
// {
// 	unsigned btbSize;
// 	unsigned historySize;
// 	unsigned tagSize;
// 	unsigned fsmState;
// 	bool isGlobalHist;
// 	bool isGlobalTable;
// 	int shared;
// 	vector<TableEntry> entries;

// 	BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
// 	bool isGlobalHist, bool isGlobalTable, int Shared) :
// 		btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(fsmState),
// 		isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), shared(Shared)
// 	{
// 		entries.resize(btbSize);
// 		for(unsigned int i = 0; i < entries.size(); i++)
// 		{
// 			entries[i] = TableEntry(historySize);
// 		}
// 	}
// };

// void testTableEntryLocalFsm()
// {
// 	uint32_t pc = 0x108;
// 	uint32_t target = 0x300;
// 	unsigned btbSize = 4;
// 	unsigned tagSize = 16;
// 	unsigned historySize = 4;
// 	TableEntry te = TableEntry(pc, target, btbSize, tagSize, historySize, STRONGLY_TAKEN);
// 	Fsm fsm = Fsm(STRONGLY_NOT_TAKEN);
// 	vector<Fsm> fsms(std::pow(2, historySize), fsm);
// 	TableEntry te1 = TableEntry(pc, target, btbSize, tagSize, historySize, fsms);
// 	if(te.predict())
// 		cout << "te predicts taken";
// 	else
// 		cout << "te predicts not taken";
// 	if(te1.predict())
// 		cout << "te1 predicts taken";
// 	else
// 		cout << "te1 predicts not taken";
// }

// void testTableEntry()
// {
// 	uint32_t pc = 0x108;
// 	uint32_t target = 0x300;
// 	unsigned btbSize = 4;
// 	unsigned tagSize = 16;
// 	unsigned historySize = 4;
// 	unsigned* gHist;
// 	*gHist = 2;


// 	TableEntry gHistTe(pc, target, btbSize, tagSize, gHist, historySize, STRONGLY_NOT_TAKEN);
// 	TableEntry gHistTe2(pc, target, btbSize, tagSize, gHist, historySize, STRONGLY_NOT_TAKEN);
// 	TableEntry lHistTe(pc, target, btbSize, tagSize, nullptr, historySize, WEAKLY_NOT_TAKEN);
// 	for(int i = 0; i < 8; i++)
// 	{
// 		gHistTe.update(false);
// 		gHistTe2.update(true);
// 		lHistTe.update(true);
// 	}
// 	lHistTe.print();
// 	cout << lHistTe.getHistory() << endl;
// }

// void testFsms()
// {
// 	Fsm fsm(STRONGLY_TAKEN);
// 	cout << fsm.state << endl;
// 	fsm.update(false);
// 	cout << fsm.state << endl;
// 	cout << "prediction: " << (fsm.predict() ? "taken" : "not taken") << endl;
// 	fsm.update(false);
// 	cout << "prediction: " << (fsm.predict() ? "taken" : "not taken") << endl;
// }

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
	return;
}

