#ifndef __BP_CLASSESS__
#define  __BP_CLASSESS__

#include <vector>
#include "bp_api.h"
using std::vector;

constexpr int INVALID_ARGS = -1;

constexpr int NOT_USING_SHARE = 0;
constexpr int USING_SHARE_LSB = 1;
constexpr int USING_SHARE_MID = 2;

constexpr int STRONGLY_NOT_TAKEN = 0;
constexpr int WEAKLY_NOT_TAKEN = 1;
constexpr int WEAKLY_TAKEN = 2;
constexpr int STRONGLY_TAKEN = 3;

constexpr int UNKNOWN_ENTRY = -1;

class Fsm
{
public:
	Fsm(int initalState);
	int state;
	void updateState(bool taken);
};

class BtbEntry
{
public:
	BtbEntry();
	BtbEntry(uint32_t pc, uint32_t target, unsigned btbSize);
	uint32_t pc;
	uint32_t tag;
	uint32_t target;
	uint8_t history;
	bool valid;

};

class BP
{
private:
	Fsm allocateFsm(uint32_t pc);
	uint8_t getHistory(int i, int j);

public:
	vector<BtbEntry> entries;
	vector<vector<uint8_t>> histories;
	vector<vector<Fsm>> fsms;
	int share;
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	SIM_stats stats;
	bool globalHistory;
	bool globalFsm;
	

	bool knownBranch(uint32_t pc);
	int findEntryNum(uint32_t pc);
	bool predict(uint32_t pc);
};


#endif  //__BP_CLASSESS__