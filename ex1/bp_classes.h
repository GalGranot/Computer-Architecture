//bp_classes.h

/*=============================================================================
* includes
=============================================================================*/
#include <vector>
#include "bp_api.h"

/*=============================================================================
* defines
=============================================================================*/
constexpr int STRONGLY_NOT_TAKEN = 0;
constexpr int WEAKLY_NOT_TAKEN = 1;
constexpr int WEAKLY_TAKEN = 2;
constexpr int STRONGLY_TAKEN = 3;

constexpr int MAX_HISTORY_SIZE = 8;
constexpr int MAX_TAG_SIZE = 30;

using std::vector;

/*=============================================================================
* FSM
=============================================================================*/

class Fsm
{
private:
    int state = WEAKLY_NOT_TAKEN;
public:
    Fsm(int state) : state(state) {}
    void updateState(bool taken);
    int getState();
};

/*=============================================================================
* BtbEntry
=============================================================================*/
class BtbEntry
{
private:
    int tableIndex;
    uint32_t tag;
    uint32_t target;
    uint8_t history = 0;
    int btbSize;
    int historySize;
    int tagSize;
    bool valid = false;

public:
    BtbEntry(uint32_t pc, int btbSize, int historySize, uint32_t _target);

    void updateHistory(bool taken);
    uint8_t getHistory();
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