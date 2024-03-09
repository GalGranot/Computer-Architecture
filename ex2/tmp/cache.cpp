#include <vector>
using std::vector;

constexpr int FULLY_ASSOCIATIVE = -1;
constexpr int NOT_ACCESSED = -1;

struct Entry
{
    uint32_t tag;
    bool valid;
    bool dirty;
    int lastAccessed;

    Entry() : tag(0), valid(false), dirty(false), lastAccessed(NOT_ACCESSED) {}
    Entry(uint32_t tag, int accessNumber) : tag(tag), valid(true), dirty(false), lastAccessed(accessNumber) {}
};

struct AccessData
{
    int tL1;
    int tL2;
    int tMem;
    int l1Miss;
    int l2Miss;
    int totalAccesses;

    AccessData(int tL1, int tL2, int tMem) : tL1(tL1), tL2(tL2), tMem(tMem),
        l1Miss(0), l2Miss(0), totalAccesses(0) {}
};

struct Cache
{
    vector<Entry> entries;
    AccessData data;
    int cacheSize;
    int blockSize;
    int assoc;
    bool writeAllocate;
    int accessNumber;
    Entry& findEntryByAddress(uint32_t address)
    {

    }

};

struct Memory
{
    Cache l1;
    Cache l2;
    Entry& findEntryByAddress(uint32_t address)
    {

    }
    void handleAccess(bool readWrite, uint32_t address)
    {

    }
};