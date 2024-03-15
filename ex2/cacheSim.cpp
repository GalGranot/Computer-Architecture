/*=============================================================================
* includes, usings, defines
=============================================================================*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <cassert>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

using std::vector;

#define NOT_FOUND -1

#define NOT_ACCESSED -1

#define FULLY_ASSOCIATIVE -1

#define READ 0
#define WRITE 1

#define READ_HIT 0
#define READ_MISS 1
#define READ_INSERT 2
#define WRITE_HIT 3
#define WRITE_MISS 4
#define WRITE_INSERT 5

#define WRITE_BACK 0
#define WRITE_THRU 1

#define NO_EVICT 0
#define EVICTED 1
#define EVICTED_DIRTY 2

#define L1_CACHE 1
#define L2_CACHE 2

#define debug 1

/*=============================================================================
* global functions & variables
=============================================================================*/
uint32_t getTag(uint32_t address, int offsetSize, int setSize, int tagSize)
{
    uint32_t _address = address;
    _address >>= offsetSize + setSize;
    int mask = (1 << tagSize) - 1;
    return _address & mask;
}

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
* AccessData
=============================================================================*/
struct AccessData
{
    int tL1;
    int tL2;
    int tMem;
    int l1Miss;
    int l2Miss;
    int accesses;
    AccessData(int tL1, int tL2, int tMem) : 
	tL1(tL1), tL2(tL2), tMem(tMem), l1Miss(0), l2Miss(0), accesses(0) {}

    void update(int result, int cache)
    {
        accesses++;
        if(result == READ_MISS || result == WRITE_MISS)
        {
            (cache == L1_CACHE) ? l1Miss++ : l2Miss++;
        }
    }
};

/*=============================================================================
* Line
=============================================================================*/
struct Line
{
    uint32_t address;
    uint32_t tag;
    uint32_t set;
    bool valid;
    bool dirty;
    int lastAccessed;
    Line() : valid(false), lastAccessed(NOT_ACCESSED) {}
    Line(uint32_t address, int accessNumber, int offsetSize, int setSize, int tagSize) : address(address), valid(true), lastAccessed(accessNumber), dirty(false)
    {
        uint32_t _address = address;
        _address >>= offsetSize;
        int mask = (1 << setSize) - 1;
        set = _address & mask;
        _address >>= setSize;
        mask = (1 << tagSize) - 1;
        tag = _address & mask;
    }
    Line(uint32_t address, uint32_t tag) : address(address), tag(tag), valid(valid) {} //don't use this
    void print()
	{
        cout << "address "; printBinary(address);
        cout << "set "; printBinary(set);
        cout << "tag "; printBinary(tag);
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
    int assoc;
    int offsetSize;
    int setSize;
    int tagSize;
    int cacheSize;
    int blockSize;
    int accessNumber;
    vector<Line> lines;

    Cache(int assoc, int offsetSize, int setSize, int tagSize, int cacheSize, int blockSize) : assoc(assoc), offsetSize(offsetSize), setSize(setSize), tagSize(tagSize), accessNumber(0), cacheSize(cacheSize), blockSize(blockSize)
    {
        lines = vector<Line>(std::pow(2, cacheSize - blockSize), Line());
    }

    int handleRequest(uint32_t address, bool readWrite)
    {
        Line l(address, NOT_ACCESSED, offsetSize, setSize, tagSize);
        if(isLineInCache(l))
            return (readWrite == READ) ? READ_HIT : WRITE_HIT;
        return (readWrite == READ) ? READ_MISS : WRITE_MISS;
    }
    
    bool isLineInCache(Line& l)
    {
        if(assoc == FULLY_ASSOCIATIVE)
        {
            for(Line& tl : lines)
            {
                if(tl.valid && tl.tag == l.tag)
                    return true;
            }
            return false;
        }
        else
        {
            return true; //TODO assoc
        }
    }

    int insert(uint32_t address)
    {
        Line l(address, accessNumber++, offsetSize, setSize, tagSize);
        if(assoc == FULLY_ASSOCIATIVE)
        {
            for(Line& tl : lines) //try and find empty spot
            {
                if(!tl.valid)
                {
                    tl = l;
                    return NO_EVICT;
                }
                int lruIndex = 0; //evict lru index
                for(int i = 1; i < lines.size(); i++)
                {
                    if(lines[lruIndex].lastAccessed > lines[i].lastAccessed)
                        lruIndex = i;
                }
                bool dirty = lines[lruIndex].dirty;
                lines[lruIndex] = l;
                return dirty ? EVICTED_DIRTY : EVICTED;
            }
        }
        else //TODO assoc
        {
            return 0;
        }
    }

    void print()
    {
        cout << "\n==================== printing cache ====================" << endl;
        int i = 0;
        for(Line& l : lines)
        {
            cout << "=== entry " << i++ << " ===" << endl;
            l.print();
        }
        cout << "======== cache parameters ========" << endl;
        cout << "accessNumber " << accessNumber << endl;
        cout << "cacheSize " << cacheSize << endl;
        cout << "assoc " << assoc << endl;
        cout << "==================== endof cache ====================\n" << endl;
    }
};

/*=============================================================================
* Memory
=============================================================================*/
// struct Memory
// {
//     Cache l1;
//     Cache l2;
//     AccessData data;
//     bool writeAllocate;

//     void handleRequest(uint32_t address, int readWrite)
//     {
//         int l1Result = l1.handleRequest(address, readWrite);
//         data.update(l1Result, L1_CACHE);
//         if(l1Result == READ_HIT)
//         {
//             return;
//         }
//         else if(l1Result == READ_MISS)
//         {
//             //search for block in l2
//             //bring block to l1. if evicted dirty, write evicted block to l2
//             int insertResult = l1.insert(address);
//             if(insertResult == EVICTED_DIRTY)
//                 l2.insert(address); // TODO split up to different functions
//         }
//         else if(l1Result == WRITE_HIT)
//         {
//             if(l1.writePolicy == WRITE_BACK)
//                 l1.markDirty(address);
//             else if(l1.writePolicy == WRITE_THRU)
//                 l2.insertOrUpdate(address);
//             return;
//         }
//         else if(l1Result == WRITE_MISS)
//         {
//             if(writeAllocate)
//             {
//                 int updateResult = l1.insertOrUpdate(address);
//                 if(updateResult == EVICTED_DIRTY)
//                     l2.insertOrUpdate(address);
//             }
//         }

//         int l2Result = l2.handleRequest(address, readWrite);
//         data.update(l2Result, L2_CACHE);
//         if(l2Result == READ_HIT)
//         {
//             return;
//         }
//         else if(l2Result == READ_MISS)
//         {
//             int updateResult = l2.insertOrUpdate(address);
//             if(updateResult == EVICTED_DIRTY)
//                 l1.evict(address);
//         }
//         else if(l2Result == WRITE_HIT)
//         {
//             if(l2.writePolicy == WRITE_BACK)
//                 l2.markDirty(address);
//             else if(l2.writePolicy == WRITE_THRU)
//                 ; //actually writing to main memory
//             return;
//         }
//         else if(l2Result == WRITE_MISS)
//         {
//             if(writeAllocate)
//             {
//                 int updateResult = l2.insertOrUpdate(address);
//                 if(updateResult == EVICTED_DIRTY)
//                     ; //actually writing dirty block to main memory
//             }
//         }
//     }
// };

/*=============================================================================
* Main
=============================================================================*/
int main()
{
    int assoc = FULLY_ASSOCIATIVE;
    int offsetSize = 4;
    int setSize = 0;
    int tagSize = 32 - offsetSize - setSize;
    int cacheSize = 2;
    int blockSize = 0;

    Cache c(assoc, offsetSize, setSize, tagSize, cacheSize, blockSize);
    uint32_t i = 0xFFFF;
    for(int j = 0; j < c.lines.size(); j++)
        c.insert(i + 0x1000*j);
    c.print();
    c.insert(i);
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
