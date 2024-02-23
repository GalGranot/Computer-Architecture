//bp_classes.h
#ifndef __BP_CLASSES_H__
#define __BP_CLASSES_H__
#include <vector>
#include "bp_api.h"
using std::vector;

constexpr int STRONGLY_NOT_TAKEN = 0;
constexpr int WEAKLY_NOT_TAKEN = 1;
constexpr int WEAKLY_TAKEN = 2;
constexpr int STRONGLY_TAKEN = 3;
constexpr int INVALID_ENTRY = -1;

uint32_t calcTagFromPc(uint32_t pc, int btbSize);

class Fsm
{
private:
    int state;

public:
    Fsm();
    Fsm(int initialState);
    int getState();
    void update(bool taken);
    bool predict();
};

class BtbEntry
{
private:
    uint8_t history = 0;
    uint32_t pc;
    uint32_t target;
    uint32_t tag;
    vector<Fsm> fsms;
    bool valid = false;
public:
    BtbEntry();
    BtbEntry(uint32_t pc, uint32_t target, vector<Fsm> fsms, int historySize);
    BtbEntry(uint32_t pc, uint32_t target, int historySize, int fsmState, int btbSize);
    void updateHistory(bool taken);
    uint8_t getHistory();
    bool predict();
};

class LocalHistoryLocalFsmBP
{
private:
    int btbSize;
    int historySize;
    int tagSize;
    int fsmState;

    vector<BtbEntry> entries;
    SIM_stats stats;
    int placeBtb(uint32_t pc);
    void updateStats(bool wrongPrediction);
public:
    LocalHistoryLocalFsmBP(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
        bool isGlobalHist, bool isGlobalTable, int Shared);
    bool predict(uint32_t pc);
    void update(uint32_t pc, uint32_t target, bool taken, uint32_t predDest);
    void addEntry(uint32_t pc, uint32_t target);
    SIM_stats getStats();
};

class LocalHistoryGlobalFsmBP
{
private:
public:
};

class GlobalHistoryGlobalFsmBP
{
private:
public:
};

class BranchPredictor
{
public:
    BranchPredictor();
    BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared);
    LocalHistoryLocalFsmBP bp1;
    // LocalHistoryGlobalFsmBP bp2;
    // GlobalHistoryGlobalFsmBP bp3;
};

#endif //__BP_CLASSES_H__