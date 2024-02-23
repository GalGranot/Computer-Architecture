#include "bp_classes.h"

/*=============================================================================
* Fsm
=============================================================================*/
Fsm::Fsm(int initialState) : state(initialState) {}

void Fsm::updateState(bool taken)
{
	if (taken && state != STRONGLY_TAKEN)
		state++;
	else if (!taken && state != STRONGLY_NOT_TAKEN)
		state--;
}

/*=============================================================================
* BtbEntry
=============================================================================*/
Btb::BtbEntry() : pc(UNKNOWN_ENTRY), tag(UNKNOWN_ENTRY), target(UNKNOWN_ENTRY),
	history(UNKNOWN_ENTRY), valid(false) {}
Btb::BtbEntry(uint32_t pc, uint32_t target, unsigned btbSize) : pc(pc),
	target(target), history(0), valid(true)
{
	int btbSizeInBits = 0;
	for (int i = btbSize; i > 1; i /= 2)
		btbSizeInBits++;
	tag = pc >> (2 + btbSize);
}


/*=============================================================================
* BP
=============================================================================*/
Fsm BP::allocateFsmNum(uint32_t pc)
{
	uint8_t history = 0;
	uint32_t fsmNum = 0;
	if (globalHistory)
		history = getHistory(0, 0);
	else
		history = getfrompc;
	if (globalFsm)
		fsmNum = 0;
	else
		fsmNum = getfrompc;
	return fsms[fsmNum][history];
}

bool BP::predict(uint32_t pc)
{
	Fsm fsm = allocateFsm(pc);
	if (fsm.state < WEAKLY_TAKEN)
		return false;
	else
		return true;
}

int BP::findEntryNum(uint32_t pc)
{
	pc <<= 2;
	int mask = (1 << btbSize) - 1;
	return pc & mask;
}

bool BP::knownBranch(uint32_t pc) 
{
	BtbEntry entry = entries[findEntryNum(pc)];
	return entry.pc == pc;
}
