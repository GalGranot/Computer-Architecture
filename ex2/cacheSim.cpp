/*=============================================================================
* includes, usings, defines
=============================================================================*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

using std::vector;

#define NOT_ACCESSED -1
#define NOT_FOUND -1
#define INVALID -1
#define FULLY_ASSOCIATIVE - 1
#define READ false
#define WRITE true
#define WRITE_BACK false
#define WRITE_THROUGH true
#define READ_HIT 0
#define READ_MISS 1
#define WRITE_HIT 2
#define WRITE_MISS 3

/*
* equations for calculating bit sizes:
* offset = block size
* set = cache size - block size - assoc
* tag = 32 - offset - set
*/

/*=============================================================================
* global functions & variables
=============================================================================*/
void printBinary(uint32_t n)
{
	for(int i = 31; i >= 0; i--)
	{
		int digit = (n >> i) & 1;
		cout << digit;
		if(i % 4 == 0)
			cout << " ";
	}
	cout << endl;
}

/*=============================================================================
* CacheDim
=============================================================================*/
struct CacheDim
{
    int offsetSize;
    int setSize;
    int tagSize;
    int cacheSize;
    int assoc;

    CacheDim() : offsetSize(INVALID), setSize(INVALID), tagSize(INVALID), cacheSize(INVALID), assoc(INVALID) {}
    CacheDim(int offsetSize, int setSize, int tagSize, int cacheSize, int assoc) : offsetSize(offsetSize), setSize(setSize), tagSize(tagSize), cacheSize(cacheSize), assoc(assoc) {}
};

/*=============================================================================
* AccessData
=============================================================================*/
struct AccessData
{
    int tL1;
    int tL2;
    int tMem;
    int l1Miss;
    int l2Miss;
    int totalAccesses;

	AccessData(int tL1, int tL2, int tMem) : 
	tL1(tL1), tL2(tL2), tMem(tMem), l1Miss(0), l2Miss(0), totalAccesses(0) {}

    void update(int result, int cacheLevel)
    {
        totalAccesses++;
        if(cacheLevel == 1 && (result == READ_MISS || result == WRITE_MISS))
            l1Miss++;
        else if(cacheLevel == 2 && (result == READ_MISS || result == WRITE_MISS))
            l2Miss++;
    }
};

/*=============================================================================
* Entry
=============================================================================*/
struct Entry
{
    uint32_t address;
    bool valid;
    bool dirty;
    int lastAccessed;
    CacheDim dim;

    Entry() : address(0xFFFFFFFF), valid(false), dirty(false), lastAccessed(NOT_ACCESSED), dim(CacheDim(INVALID, INVALID, INVALID, INVALID, INVALID)) {}
    Entry(uint32_t address, int accessNumber, CacheDim dim) :
    address(address), valid(true), dirty(false), lastAccessed(accessNumber), dim(dim) {}

    int set()
    {
        uint32_t _address = address;
        _address >>= dim.offsetSize;
        int mask = (1 << dim.setSize) - 1;
        return _address & mask;
    }
    int tag()
    {
        uint32_t _address = address;
        _address >>= dim.offsetSize + dim.setSize;
        int mask = (1 << dim.tagSize) - 1;
        return _address & mask;
    }

    	void print()
	{
        cout << "address "; printBinary(address);
        cout << "set "; printBinary(set());
        cout << "tag "; printBinary(tag());
		cout << "valid " << valid << endl;
		cout << "dirty " << dirty << endl;
		cout << "last accessed " << lastAccessed << endl;
	}
};

/*=============================================================================
* Cache
=============================================================================*/
struct Cache
{
    CacheDim dim;
    vector<Entry> entries;
    int accessNumber;

    Cache(int blockSize, int cacheSize, int assoc) : accessNumber(0)
    {
        dim.assoc = assoc;
        dim.cacheSize = cacheSize;
        dim.offsetSize = blockSize;
        dim.setSize = cacheSize - assoc;
        dim.tagSize = 32 - dim.setSize - dim.offsetSize;

        int entriesNum = std::pow(2, cacheSize - blockSize);
        entries = vector<Entry>(entriesNum, Entry());
    }

    vector<int> placeEntry(Entry& e)
    {
        vector<int> indices;
        if(dim.assoc == FULLY_ASSOCIATIVE)
        {
            for(int i = 0; i < entries.size(); i++) //search for empty spots or spots where entry appears
            {
                if(entries[i].tag() == e.tag())
                {
                    vector<int> pos(1, i);
                    return pos;
                }
                if(!entries[i].valid)
                    indices.push_back(i);
            }
        }
        else //associative cache
        {
            int offset = std::pow(2, dim.cacheSize - dim.assoc);
            int positionInSet = e.set();
            for(int i = 0; i < dim.assoc; i++)
            {
                int positionInTable = i * offset + positionInSet;
                if(entries[positionInTable].tag() == e.tag())
                {
                    vector<int> pos(1, positionInTable);
                    return pos;
                }
                if(!entries[positionInTable].valid)
                    indices.push_back(positionInTable);
            }
        }
        return indices;
    }

    void evictAndReplace(Entry& e)
    {
        int lruIndex = 0;
        if(dim.assoc == FULLY_ASSOCIATIVE)
        {
            for(int i = 1; i < entries.size(); i++)
                lruIndex = (entries[lruIndex].lastAccessed > entries[i].lastAccessed) ? i : lruIndex;
        }
        else //associative cache
        {
            int offset = std::pow(2, dim.cacheSize - dim.assoc);
            int positionInSet = e.set();
            for(int i = 1; i < dim.assoc; i++)
            {
                int positionInTable = i * offset + positionInSet;
                lruIndex = (entries[lruIndex].lastAccessed > entries[positionInTable].lastAccessed) ? positionInTable : lruIndex;
            }
        }
        entries[lruIndex] = e;
    }

    int handleRequest(uint32_t address, bool readWrite)
    {
        Entry e = Entry(address, accessNumber, dim);
        vector<int> positionsForEntry = placeEntry(e);
        if(!positionsForEntry.empty()) //read/write hit!
        {
            for(int i : positionsForEntry)
            {
                if(!entries[i].valid)
                    entries[i] = e;
            }
            return (readWrite == READ) ? READ_HIT : WRITE_HIT;
        }
        evictAndReplace(e);
        return (readWrite == READ) ? READ_MISS : WRITE_MISS;
    }

    void print()
    {
        cout << "==================== printing cache ====================" << endl;
        int i = 0;
        for(Entry& e : entries)
        {
            cout << "=== printing entry " << i++ << " ===" << endl;
            e.print();
        }
        cout << "======== cache parameters ========" << endl;
        cout << "accessNumber " << accessNumber << endl;
        cout << "cacheSize " << dim.cacheSize << endl;
        cout << "assoc " << dim.assoc << endl;
        cout << "==================== endof cache ====================" << endl;
    }
};

/*=============================================================================
* Memory
=============================================================================*/
struct Memory
{
    Cache l1;
    Cache l2;
    bool writeAllocate;
    bool writeBackThrough;
    AccessData data;

    void handleRequest(uint32_t address, bool readWrite)
    {
        int l1Result = l1.handleRequest(address, readWrite);
        if(l1Result == READ_HIT)
        {
            data.update(READ_HIT, 1);
            return;
        }
        else if(l1Result == READ_MISS)
        {
            data.update(READ_MISS, 1);
            //TODO go to l2
        }
        else if(l1Result == WRITE_HIT)
        {
            data.update(WRITE_HIT, 1);
            return;
        }
        else if(l1Result == WRITE_MISS)
        {
            data.update(WRITE_MISS, 1);
            //TODO go to l2
        }

        //if here - l1 miss
        int l2Result = l2.handleRequest(address, readWrite);
        if(l2Result == READ_HIT)
        {
            data.update(READ_HIT, 2);
            return;
        }
        else if(l2Result == READ_MISS)
        {
            data.update(READ_MISS, 2);
            //TODO go to memory
        }
        else if(l2Result == WRITE_HIT)
        {
            data.update(WRITE_HIT, 2);
            return;
        }
        else if(l2Result == WRITE_MISS)
        {
            data.update(WRITE_MISS, 2);
            //TODO go to memory
        }
    }
};

/*=============================================================================
* Main
=============================================================================*/

int main()
{
    Cache c(0, 3, FULLY_ASSOCIATIVE);
    c.print();
    for(int i = 0x0; i < 0x4; i++)
    {
        c.handleRequest(i, READ);
    }
    c.print();
}

// int main(int argc, char **argv) {

// 	if (argc < 19) {
// 		cerr << "Not enough arguments" << endl;
// 		return 0;
// 	}

// 	// Get input arguments

// 	// File
// 	// Assuming it is the first argument
// 	char* fileString = argv[1];
// 	ifstream file(fileString); //input file stream
// 	string line;
// 	if (!file || !file.good()) {
// 		// File doesn't exist or some other error
// 		cerr << "File not found" << endl;
// 		return 0;
// 	}

// 	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
// 			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

// 	for (int i = 2; i < 19; i += 2) {
// 		string s(argv[i]);
// 		if (s == "--mem-cyc") {
// 			MemCyc = atoi(argv[i + 1]);
// 		} else if (s == "--bsize") {
// 			BSize = atoi(argv[i + 1]);
// 		} else if (s == "--l1-size") {
// 			L1Size = atoi(argv[i + 1]);
// 		} else if (s == "--l2-size") {
// 			L2Size = atoi(argv[i + 1]);
// 		} else if (s == "--l1-cyc") {
// 			L1Cyc = atoi(argv[i + 1]);
// 		} else if (s == "--l2-cyc") {
// 			L2Cyc = atoi(argv[i + 1]);
// 		} else if (s == "--l1-assoc") {
// 			L1Assoc = atoi(argv[i + 1]);
// 		} else if (s == "--l2-assoc") {
// 			L2Assoc = atoi(argv[i + 1]);
// 		} else if (s == "--wr-alloc") {
// 			WrAlloc = atoi(argv[i + 1]);
// 		} else {
// 			cerr << "Error in arguments" << endl;
// 			return 0;
// 		}
// 	}

// 	while (getline(file, line)) {

// 		stringstream ss(line);
// 		string address;
// 		char operation = 0; // read (R) or write (W)
// 		if (!(ss >> operation >> address)) {
// 			// Operation appears in an Invalid format
// 			cout << "Command Format error" << endl;
// 			return 0;
// 		}

// 		// DEBUG - remove this line
// 		cout << "operation: " << operation;

// 		string cutAddress = address.substr(2); // Removing the "0x" part of the address

// 		// DEBUG - remove this line
// 		cout << ", address (hex)" << cutAddress;

// 		unsigned long int num = 0;
// 		num = strtoul(cutAddress.c_str(), NULL, 16);

// 		// DEBUG - remove this line
// 		cout << " (dec) " << num << endl;

// 	}

// 	double L1MissRate;
// 	double L2MissRate;
// 	double avgAccTime;

// 	//your code here

// 	printf("L1miss=%.03f ", L1MissRate);
// 	printf("L2miss=%.03f ", L2MissRate);
// 	printf("AccTimeAvg=%.03f\n", avgAccTime);

// 	return 0;
// }
