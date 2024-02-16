/*=============================================================================
* includes
=============================================================================*/
#include <vector>
#include "bp_api.h"

/*=============================================================================
* defines
=============================================================================*/
int constexpr STRONGLY_NOT_TAKEN = 0;
int constexpr WEAKLY_NOT_TAKEN = 1;
int constexpr WEAKLY_TAKEN = 2;
int constexpr STRONGLY_TAKEN = 3;

int constexpr MAX_HISTORY_SIZE = 8;
using std::vector;

/*=============================================================================
* FSM
=============================================================================*/

class Fsm
{
private:
    int state = WEAKLY_NOT_TAKEN;
public:
    Fsm() : state(WEAKLY_NOT_TAKEN) {}
    void updateState(bool taken);
    int getState();
};

/*=============================================================================
* BtbEntry
=============================================================================*/

class BtbEntry
{
private:
    const uint32_t tag;
    const uint32_t target;
    uint8_t history = 0;
    int historySize;
    bool valid = false;
    void clearUpperHistoryBits();

public:
    BtbEntry(uint32_t tag, uint32_t target, uint32_t history)
        : tag(tag), target(target), history(history) {}
    BtbEntry() : tag(0), target(0), historySize(0), valid(false) {}
    void updateHistory(bool taken);
    uint8_t getHistory(int historySize);
    uint32_t getTag();
};

/*=============================================================================
* Btb
=============================================================================*/
class Btb
{
private:
    bool type;
    bool share;
    int btbSize;
    vector<BtbEntry> entries;

public:
    Btb(bool type, unsigned btbSize, bool share);
    ~Btb();
    BtbEntry& getEntry(const uint32_t tag);
    void handleEntry(BtbEntry& entry);
};

/*=============================================================================
* BranchPredictor
=============================================================================*/
class BranchPredictor
{
private:
    int btbSize;
    int historySize;
    int tagSize;
    int fsmState;
    bool isGlobalHist;
    bool isGlobalTable;
    int Shared;
    SIM_stats stats;
    Btb btb;

public:
    BranchPredictor();
    bool predict(uint32_t pc, uint32_* dst);
    void update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst);
    SIM_stats getStats();
};