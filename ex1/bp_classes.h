#include <vector>
#include <memory>
using std::vector;
using std::unique_ptr;

constexpr int STRONGLY_NOT_TAKEN = 0;
constexpr int WEAKLY_NOT_TAKEN = 1;
constexpr int WEAKLY_TAKEN = 2;
constexpr int STRONGLY_TAKEN = 3;

class Fsm
{
private:
    int state;

public:
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
    BtbEntry(uint32_t pc, uint32_t target, vector<Fsm> fsms, int historySize);
    BtbEntry(uint32_t pc, uint32_t target, int historySize, int fsmState);
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
    SIM_stats* stats;
    int placeBtb(uint32_t pc);
    void updateStats(bool wrongPrediction);
public:
    LocalHistoryLocalFsmBP(int btbSize);
    bool predict(uint32_t pc);
    void update(uint32_t pc, uint32_t target, bool taken, uint32_t predDest);
    void addEntry(uint32_t pc, uint32_t target);
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