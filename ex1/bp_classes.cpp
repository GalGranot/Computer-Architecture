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

uint8_t BtbEntry::getHistory()
{
	int bitsToClear = MAX_HISTORY_SIZE - historySize;
	int mask = (1 << bitsToClear) - 1;
	history &= mask;
	return history;
}

BtbEntry::BtbEntry(uint32_t pc, int _btbSize, int _historySize, uint32_t _target)
{
	target = _target;
	history = 0;
	historySize = _historySize;
	btbSize = _btbSize;

	pc <<= 2; //erase 00 at beginning of each pc
	int btbSizeInBits = 0;
	for(int i = btbSize; i > 1; i /= 2) //calc log_2 of btbSize
		btbSizeInBits++;
	//tableIndexMask example:
	//Btb size is 16 -> 4 bits for representation. Required mask: 0000 1111
	//1 << btbSizeInBits = 1 << 4 -> 0001 0000, sub 1 -> 0000 1111
	int tableIndexMask = (1 << btbSizeInBits) - 1;
	tableIndex = pc & tableIndexMask;

	tag = 0; //FIXME calc tag
}
/*=============================================================================
* Btb
=============================================================================*/


/*=============================================================================
* BranchPredictor
=============================================================================*/
SIM_stats BranchPredictor::getStats() { return stats; }