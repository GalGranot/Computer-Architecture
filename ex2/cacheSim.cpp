/*=============================================================================
* include, using, define
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

#define READ 0
#define WRITE 1

#define INVALID_RESULT -1
#define READ_HIT 0
#define READ_MISS 1
#define READ_MISS_INSERT_INVALID 2
#define READ_MISS_EVICT_CLEAN 3
#define READ_MISS_EVICT_DIRTY 4
#define WRITE_HIT 5
#define WRITE_MISS 6
#define WRITE_MISS_NO_ALLOC 7
#define WRITE_MISS_INSERT_INVALID 8
#define WRITE_MISS_EVICT_CLEAN 9
#define WRITE_MISS_EVICT_DIRTY 10

#define L1_CACHE 1
#define L2_CACHE 2

#define NOT_ACCESSED -1

/*=============================================================================
* global functions
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
* AccessData
=============================================================================*/
struct AccessData
{
	int tL1;
    int tL2;
    int tMem;
    int l1Miss;
	int l1Hit;
    int l2Miss;
	int l2Hit;
    int accesses;
    AccessData(int tL1, int tL2, int tMem) : 
	tL1(tL1), tL2(tL2), tMem(tMem), l1Miss(0), l2Miss(0), l1Hit(0), l2Hit(0), accesses(0) {}

    void update(int result, int cache)
    {
        accesses++;
        if(result == READ_MISS || result == WRITE_MISS)
            (cache == L1_CACHE) ? l1Miss++ : l2Miss++;
		else
			(cache == L1_CACHE) ? l1Hit++ : l2Hit++;
    }

	double missRate(int cache)
	{
		double result;
		if(cache == L1_CACHE)
			result = l1Miss / (l1Miss + l1Hit);
		else if(cache == L2_CACHE)
			result = l2Miss / (l2Miss + l2Hit);
		else
			result = ((l1Hit + l1Miss) * tL1 + (l2Hit + l2Miss) * tL2 + l2Miss * tMem) / accesses;
		return result;
	}
};

/*=============================================================================
* Dimension Structs
=============================================================================*/
struct AddrDim
{
	int offsetSize;
	int setSize;
	int tagSize;
	AddrDim(int offsetSize, int setSize, int tagSize) : offsetSize(offsetSize), setSize(setSize), tagSize(tagSize) {}
};

struct CacheDim
{
	int cacheSize;
    int blockSize;
	int assoc;
	int lineNumber;
	CacheDim(int cacheSize, int blockSize, int assoc) : cacheSize(cacheSize), blockSize(blockSize), assoc(assoc) {}
}

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
	Line(uint32_t address, AddrDim d, int accessNumber) : address(address), valid(true), dirty(false), lastAccessed(accessNumber)
	{
        uint32_t _address = address;
        _address >>= d.offsetSize;
        int mask = (1 << d.setSize) - 1;
        set = _address & mask;
        _address >>= d.setSize;
        mask = (1 << d.tagSize) - 1;
        tag = _address & mask;
	}

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
	vector<Line> lines;
	CacheDim cDim;
	AddrDim aDim;
	Cache(CacheDim cDim, AddrDim aDim) : cDim(cDim), aDim(aDim)
	{
		int lineNumber = cDim.cacheSize - aDim.offsetSize;
		cDim.lineNumber = lineNumber;
		lines = vector<Line>(std::pow(2, cDim.cacheSize - aDim.offsetSize));
	}

	vector<int>& positionsInCache(uint32_t address)
	{
		Line l(address, aDim, NOT_ACCESSED);
		vector<int> v;
		int sets = std::pow(2, cDim.lineNumber - cDim.assoc);
		int offset = std::pow(2, cDim.cacheSize) / sets;
		int indexInSet = l.set; //TODO maybe this needs 2^ too?
		for(int i = 0; i < sets; i++)
		{
			v.push_back(i * offset + indexInSet);
		}
		return v;
	}
};

/*=============================================================================
* Memory
=============================================================================*/
struct Memory
{
	Cache l1;
	Cache l2;
	AccessData data;
	bool writeAllocate;
	std::pair<bool, uint32_t> dirty;

	int read(uint32_t address, int cache)
	{
		Cache& c = (cache == L1_CACHE) ? l1 : l2;
	}

	int write(uint32_t address, int cache)
	{
		Cache& c = (cache == L1_CACHE) ? l1 : l2;
	}

	void evict(uint32_t address, int cache)
	{
		Cache& c = (cache == L1_CACHE) ? l1 : l2;
	}



	void memoryAccess(uint32_t address, int readWrite)
	{
		int result = INVALID_RESULT;
		if(readWrite == READ)
		{
			//l1
			result = read(address, L1_CACHE);
			if(result == READ_HIT)
			{
				data.update(READ_HIT, L1_CACHE);
				return;
			}
			else if(result == READ_MISS_INSERT_INVALID || result == READ_MISS_EVICT_CLEAN)
			{
				data.update(READ_MISS, L1_CACHE);
			}
			else if(result == READ_MISS_EVICT_DIRTY)
			{
				data.update(READ_MISS, L1_CACHE);
				write(dirty.second, L2_CACHE); //TODO by inclusiveness write will always be sucessful - assert this
				dirty.first = false;
			}

			//l2
			result = read(address, L2_CACHE);
			if(result == READ_HIT)
			{
				data.update(READ_HIT, L2_CACHE);
				return;
			}
			data.update(READ_MISS, L2_CACHE);
			if(result == READ_MISS_INSERT_INVALID)
				return;
			evict(dirty.second, L1_CACHE);
			dirty.first = false;
		}
		else if(readWrite == WRITE)
		{
			//l1
			result = write(address, L1_CACHE);
			if(result == WRITE_HIT)
			{
				data.update(WRITE_HIT, L1_CACHE);
				return;
			}
			if(result == WRITE_MISS_NO_ALLOC)
			{
				data.update(WRITE_MISS, L1_CACHE);
				write(address, L2_CACHE); //TODO by inclusiveness write will always be sucessful - assert this
				return;
			}
			else if(result == READ_MISS_INSERT_INVALID || result == READ_MISS_EVICT_CLEAN)
			{
				data.update(WRITE_MISS, L1_CACHE);
			}
			else if(result == WRITE_MISS_EVICT_DIRTY)
			{
				data.update(WRITE_MISS, L1_CACHE);
				write(dirty, L2_CACHE); //TODO by inclusiveness write will always be sucessful - assert this
				dirty.first = false;
			}

			//l2
			result = write(address, L2_CACHE);
			if(result == WRITE_HIT)
			{
				data.update(WRITE_HIT, L2_CACHE);
				return;
			}
			data.update(WRITE_MISS, L2_CACHE);
			if(result == WRITE_MISS_INSERT_INVALID)
				return;
			evict(dirty.second, L1_CACHE);
			dirty.first = false;
		}
	}
};

/*=============================================================================
* main
=============================================================================*/
int main(int argc, char **argv)
{

	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument
	char* fileString = argv[1];
	ifstream file(fileString); //input file stream
	string line;
	if (!file || !file.good()) {
		// File doesn't exist or some other error
		cerr << "File not found" << endl;
		return 0;
	}

	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

	for (int i = 2; i < 19; i += 2) {
		string s(argv[i]);
		if (s == "--mem-cyc") {
			MemCyc = atoi(argv[i + 1]);
		} else if (s == "--bsize") {
			BSize = atoi(argv[i + 1]);
		} else if (s == "--l1-size") {
			L1Size = atoi(argv[i + 1]);
		} else if (s == "--l2-size") {
			L2Size = atoi(argv[i + 1]);
		} else if (s == "--l1-cyc") {
			L1Cyc = atoi(argv[i + 1]);
		} else if (s == "--l2-cyc") {
			L2Cyc = atoi(argv[i + 1]);
		} else if (s == "--l1-assoc") {
			L1Assoc = atoi(argv[i + 1]);
		} else if (s == "--l2-assoc") {
			L2Assoc = atoi(argv[i + 1]);
		} else if (s == "--wr-alloc") {
			WrAlloc = atoi(argv[i + 1]);
		} else {
			cerr << "Error in arguments" << endl;
			return 0;
		}
	}

	while (getline(file, line)) {

		stringstream ss(line);
		string address;
		char operation = 0; // read (R) or write (W)
		if (!(ss >> operation >> address)) {
			// Operation appears in an Invalid format
			cout << "Command Format error" << endl;
			return 0;
		}

		// DEBUG - remove this line
		cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		cout << ", address (hex)" << cutAddress;

		unsigned long int num = 0;
		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		cout << " (dec) " << num << endl;

	}

	double L1MissRate;
	double L2MissRate;
	double avgAccTime;

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
