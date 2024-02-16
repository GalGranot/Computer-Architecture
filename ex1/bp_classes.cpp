//bp_classes.cpp

#include "bp_classes.h"

/*=============================================================================
* FSM
=============================================================================*/
void Fsm::updateState(bool taken)
{
	if (taken && state != STRONGLY_TAKEN)
		state++;
	else if (!taken && state != STRONGLY_NOT_TAKEN)
		state--;
}

int Fsm::getState() { return state; }

/*=============================================================================
* BtbEntry
=============================================================================*/
void BtbEntry::updateHistory(bool taken)
{
	history <<= 1;
	history += taken ? 1 : 0;
}

uint8_t BtbEntry::getHistory(int historySize)
{
	clearUpperHistoryBits();
	return history;
}

void BtbEntry::clearUpperHistoryBits()
{
	int bitsToClear = MAX_HISTORY_SIZE - historySize;
	int mask = (1 << bitsToClear) - 1;
	history &= mask;
}

uint32_t BtbEntry::getTag() { return tag; }
/*=============================================================================
* Btb
=============================================================================*/
Btb::Btb(bool type, unsigned btbSize, bool share)
	: type(type), share(share), btbSize(btbSize)
{
	entries.resize(btbSize);
	for (int i = 0; i < btbSize; i++)
		entries[i] = new BtbEntry();
}

Btb::~Btb()
{
	for (int i = 0; i < btbSize; i++)
		delete entries[i];
}

BtbEntry& Btb::getEntry(const uint32_t tag)
{
	
}

void Btb::handleEntry(BtbEntry& entry)
{

}

/*=============================================================================
* BranchPredictor
=============================================================================*/
SIM_stats BranchPredictor::getStats() { return stats; }