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

/*=============================================================================
* BtbEntry
============================================================================*/
BtbEntry::BtbEntry(uint32_t pc, uint32_t target, vector<Fsm> fsms, int historySize)
    : pc(pc), target(target), fsms(fsms), valid(true)
{
    int historySizePower2 = 1;
    for(int i = 0; i < historySize; i++)
        historySizePower2 *= 2;
    fsms.resize(historySizePower2)
    //calculate tag from pc
    
}

BtbEntry::BtbEntry(uint32_t pc, uint32_t target, int historySize, int fsmState)
    : pc(pc), target(target), valid(true)
{
    int historySizePower2 = 1;
    for(int i = 0; i < historySize; i++)
        historySizePower2 *= 2;
    fsms.resize(historySizePower2);
    //calculate tag from pc
    for(int i = 0; i < fsms.size(); i++)
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
    stats->flush_num += wrongPrediction ? 1 : 0;
    stats->br_num++;
}

void LocalHistoryLocalFsmBP::addEntry(uint32_t pc, uint32_t target)
{
    int btbIndex = placeBtb(pc);
    entries[btbIndex] = BtbEntry(pc, target, historySize, fsmState);
}

LocalHistoryLocalFsmBP::LocalHistoryLocalFsmBP(int btbSize)
{
    entries.resize(btbSize);
}