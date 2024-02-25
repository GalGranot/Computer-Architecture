//bp_classes.cpp
#include "bp_classes.h"
/*=============================================================================
* Fsm
============================================================================*/

int Fsm::getState() { return state; }
void Fsm::update(bool taken)
{
    if(taken && state != STRONGLY_TAKEN)
        state++;
    if(!taken && state != STRONGLY_NOT_TAKEN)
        state--;
}
bool Fsm::predict() { return state > WEAKLY_NOT_TAKEN; }
Fsm::Fsm(int initialState) : state(initialState) {}
Fsm::Fsm() : state(STRONGLY_NOT_TAKEN) {}

/*=============================================================================
* BtbEntry
============================================================================*/
BtbEntry::BtbEntry() : history(INVALID_ENTRY), pc(INVALID_ENTRY),
    target(INVALID_ENTRY), tag(INVALID_ENTRY), fsms(0), valid(false) {}

BtbEntry::BtbEntry(uint32_t pc, uint32_t target, vector<Fsm> fsms, int historySize)
    : pc(pc), target(target), fsms(fsms), valid(true)
{
    int historySizePower2 = 1;
    for(int i = 0; i < historySize; i++)
        historySizePower2 *= 2;
    fsms.resize(historySizePower2);
    //calculate tag from pc
    
}

BtbEntry::BtbEntry(uint32_t pc, uint32_t target, int historySize, int fsmState, int btbSize)
    : pc(pc), target(target), valid(true)
{
    int historySizePower2 = 1;
    for(int i = 0; i < historySize; i++)
        historySizePower2 *= 2;
    fsms.resize(historySizePower2);
    tag = calcTagFromPc(pc, btbSize);
    for(unsigned i = 0; i < fsms.size(); i++)
        fsms[i] = Fsm(fsmState);
}

bool BtbEntry::predict()
{
    return fsms[history].predict();
}

void BtbEntry::updateHistory(bool taken)
{
    history <<= 1;
    history += taken ? 1 : 0;
}

uint8_t BtbEntry::getHistory() { return history; }

/*=============================================================================
* LocalHistoryLocalFsmBP
============================================================================*/
int LocalHistoryLocalFsmBP::placeBtb(uint32_t pc)
{
    pc <<= 2;
    int btbSizeInBits = btbSize;
    for(int i = 0; i < btbSize; i++)
        btbSizeInBits /= 2;
    int mask = (1 << btbSizeInBits) - 1;
    pc &= mask;
    return pc;
}

bool LocalHistoryLocalFsmBP::predict(uint32_t pc)
{
    int btbIndex = placeBtb(pc);
    if(btbIndex < 0)
        return false;
    return entries[btbIndex].predict();
}

void LocalHistoryLocalFsmBP::update(uint32_t pc, uint32_t target, bool taken, uint32_t predDest)
{
    int btbIndex = placeBtb(pc);
    bool wrongPrediction = false;
    if(btbIndex < 0)
        addEntry(pc, target);
    else
    {
        BtbEntry entry = entries[btbIndex];
        if(taken != entry.predict())
            wrongPrediction = true;
        entry.updateHistory(taken);
    }
    updateStats(wrongPrediction);
}

void LocalHistoryLocalFsmBP::updateStats(bool wrongPrediction)
{
    stats.flush_num += wrongPrediction ? 1 : 0;
    stats.br_num++;
}

void LocalHistoryLocalFsmBP::addEntry(uint32_t pc, uint32_t target)
{
    int btbIndex = placeBtb(pc);
    entries[btbIndex] = BtbEntry(pc, target, historySize, fsmState, btbSize);
}

SIM_stats LocalHistoryLocalFsmBP::getStats() { return stats; }

LocalHistoryLocalFsmBP::LocalHistoryLocalFsmBP(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
    bool isGlobalHist, bool isGlobalTable, int Shared) :
    btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(fsmState)
    
{
    entries.resize(btbSize);
    stats.br_num = 0;
    stats.flush_num = 0;
    stats.size = 0; //FIXME init size of predictor
}

/*=============================================================================
* BranchPredictor
============================================================================*/
BranchPredictor::BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
    bool isGlobalHist, bool isGlobalTable, int Shared)
    : bp1(btbSize,historySize, tagSize, fsmState,
    isGlobalHist, isGlobalTable, Shared)
{

}

BranchPredictor::BranchPredictor() : bp1(0, 0, 0, 0, false, false, 0)
{

}

/*=============================================================================
* Global Functions
============================================================================*/
uint32_t calcTagFromPc(uint32_t pc, int btbSize)
{
    pc <<= 2;
    int btbSizeInBits = btbSize;
    for(int i = 0; i < btbSize; i++)
        btbSizeInBits /= 2;
    pc <<= btbSizeInBits;
    return pc;
}