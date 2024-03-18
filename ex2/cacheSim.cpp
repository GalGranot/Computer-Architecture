/*=============================================================================
* include, using, define
=============================================================================*/
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <cmath>
#include <cassert>

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

#define dbg 0

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
	double tL1;
    double tL2;
    double tMem;
    double l1Miss;
	double l1Hit;
    double l2Miss;
	double l2Hit;
    double accesses;
    AccessData(int tL1, int tL2, int tMem) : 
	tL1(tL1), tL2(tL2), tMem(tMem), l1Miss(0), l2Miss(0), l1Hit(0), l2Hit(0), accesses(0) {}

    void update(int result, int cache)
    {
		if(cache == L1_CACHE)
        	accesses++;
		bool hit = result == READ_HIT || result == WRITE_HIT;
		if(hit)
			(cache == L1_CACHE) ? l1Hit++ : l2Hit++;
		else
			(cache == L1_CACHE) ? l1Miss++ : l2Miss++;
    }
	double missRate(int cache)
	{
		double result;
		if(cache == L1_CACHE)
			result = l1Miss / (l1Hit + l1Miss);
		else if(cache == L2_CACHE)
			result = l2Miss / (l2Miss + l2Hit);
		else
			// result = ((l1Hit + l1Miss) * tL1 + (l2Hit + l2Miss) * tL2 + l2Miss * tMem) / accesses;
			result = ((l1Hit + l1Miss) * tL1 + l1Miss * tL2 + l2Miss * tMem) / (l1Hit + l1Miss);

		return result;
	}
	void dump()
	{
		std::cout << "tL1=" << tL1 << ", "
		<< "tL2=" << tL2 << ", "
		<< "tMem=" << tMem << ", "
		<< "l1Miss=" << l1Miss << ", "
		<< "l1Hit=" << l1Hit << ", "
		<< "l2Miss=" << l2Miss << ", "
		<< "l2Hit=" << l2Hit << ", "
		<< "accesses=" << accesses << std::endl;
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

// struct CacheDim
// {
// 	int cacheSize;
//     int blockSize;
// 	int assoc;
// 	int lineNumber;
// 	CacheDim(int cacheSize, int blockSize, int assoc) : cacheSize(cacheSize), blockSize(blockSize), assoc(assoc) {}
// };

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
		if(d.tagSize == 32)
			tag = address;
	}

	void print()
	{
        cout << "address 0x" << std::hex << address << std::dec << ", "; printBinary(address);
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
	AddrDim dim;
	int offsetSize;
	int setSize;
	int tagSize;
	int cacheSize;
    int blockSize;
	int assoc;
	int lineNumber;
	int accessNumber;
	Cache(AddrDim dim, int cacheSize, int blockSize, int assoc) : dim(dim),
	offsetSize(dim.offsetSize), setSize(dim.setSize), tagSize(dim.tagSize),
	cacheSize(cacheSize), blockSize(blockSize), assoc(assoc), accessNumber(0)
	{
		lineNumber = cacheSize - offsetSize;
		lines = vector<Line>(std::pow(2, cacheSize - dim.offsetSize));
		assert(blockSize == offsetSize && "offset == block in cache ctor");
	}

	vector<int> positionsInCache(uint32_t address)
	{
		Line l(address, dim, NOT_ACCESSED);
		vector<int> v;
		int setSize = std::pow(2, lineNumber - assoc);
		int setsNum = std::pow(2, assoc);
		for(int i = 0; i < setsNum; i++)
		{
			int index = i * setSize + l.set;
			v.push_back(index);
		}
		return v;
	}

    void print(int num)
    {
        cout << "\n==================== printing cache " << num << " ====================" << endl;
        for(int i = 0; i < lines.size(); i++)
        {
			Line& l = lines[i];
            cout << "=== entry " << i << " ===" << endl;
            l.print();
        }
        cout << "======== cache parameters ========" << endl;
        cout << "cacheSize " << cacheSize << endl;
		cout << "blockSize " << blockSize << endl;
        cout << "assoc " << assoc << endl;
        cout << "accessNumber " << accessNumber << endl;
        cout << "\n==================== endof cache " << num << " ====================" << endl;
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

	Memory(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc, unsigned L2Assoc, unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc) : //TODO fully associative set size calc 
	data(AccessData(L1Cyc, L2Cyc, MemCyc)), writeAllocate(WrAlloc),
	l1(Cache(AddrDim(BSize, L1Size - BSize - L1Assoc, 32 - BSize - (L1Size - BSize - L1Assoc)), L1Size, BSize, L1Assoc)),
	l2(Cache(AddrDim(BSize, L2Size - BSize - L2Assoc, 32 - BSize - (L2Size - BSize - L2Assoc)), L2Size, BSize, L2Assoc))
	{}

	int read(uint32_t address, int cache)
	{
		Cache& c = (cache == L1_CACHE) ? l1 : l2;
		Line l(address, c.dim, c.accessNumber++);
		vector<int> positions = c.positionsInCache(address);
		for(int j = 0; j < positions.size(); j++)
		{
			int i = positions[j];
			Line& cl = c.lines[i];
			if(cl.valid && cl.tag == l.tag) //read hit!
			{
				cl.lastAccessed = l.lastAccessed;
				return READ_HIT;
			}
		}
		//read miss - try inserting in invalid places
		for(int j = 0; j < positions.size(); j++)
		{
			int i = positions[j];
			Line& cl = c.lines[i];
			if(!cl.valid)
			{
				cl = l;
				return READ_MISS_INSERT_INVALID;
			}
		}
		//read miss - evict
		int lruIndex = positions[0];
		for(int j = 1; j < positions.size(); j++)
		{
			int i = positions[j];
			assert(c.lines[i].valid);
			if(c.lines[lruIndex].lastAccessed > c.lines[i].lastAccessed)
				lruIndex = i;
		}
		assert(c.lines[lruIndex].valid);
		if(c.lines[lruIndex].dirty)
		{
			dirty.first = true;
			dirty.second = c.lines[lruIndex].address;
			c.lines[lruIndex] = l;
			return READ_MISS_EVICT_DIRTY;
		}
		c.lines[lruIndex] = l;
		return READ_MISS_EVICT_CLEAN;
	}

	int write(uint32_t address, int cache)
	{
		Cache& c = (cache == L1_CACHE) ? l1 : l2;
		Line l(address, c.dim, c.accessNumber++);
		vector<int> positions = c.positionsInCache(address);
		for(int j = 0; j < positions.size(); j++)
		{
			int i = positions[j];
			Line& cl = c.lines[i];
			if(cl.valid && cl.tag == l.tag) //write hit!
			{
				cl.lastAccessed = l.lastAccessed;
				cl.dirty = true;
				return WRITE_HIT;
			}
		}
		//write miss
		if(!writeAllocate)
		{
			c.accessNumber--;
			return WRITE_MISS_NO_ALLOC;
		}
		//write miss w/ allocate - try inserting in invalid places
		l.dirty = true;
		for(int j = 0; j < positions.size(); j++)
		{
			int i = positions[j];
			Line& cl = c.lines[i];
			if(!cl.valid)
			{
				cl = l;
				return WRITE_MISS_INSERT_INVALID;
			}
		}
		//write miss - evict
		int lruIndex = positions[0];
		for(int j = 1; j < positions.size(); j++)
		{
			int i = positions[j];
			assert(c.lines[i].valid);
			if(c.lines[lruIndex].lastAccessed > c.lines[i].lastAccessed)
				lruIndex = i;
		}
		assert(c.lines[lruIndex].valid);
		if(c.lines[lruIndex].dirty)
		{
			dirty.first = true;
			dirty.second = c.lines[lruIndex].address;
			c.lines[lruIndex] = l;
			return WRITE_MISS_EVICT_DIRTY;
		}
		c.lines[lruIndex] = l;
		return WRITE_MISS_EVICT_CLEAN;
	}

	void evict(uint32_t address, int cache)
	{
		Cache& c = (cache == L1_CACHE) ? l1 : l2;
		Line l(address, c.dim, NOT_ACCESSED);
		vector<int> positions = c.positionsInCache(address);
		for(int j = 0; j < positions.size(); j++)
		{
			int i = positions[j];
			Line& cl = c.lines[i];
			if(cl.valid && cl.tag == l.tag)
			{
				cl.valid = false;
				return;
			}
		}
	}

	void memoryAccess(uint32_t address, int readWrite)
	{
		int result = INVALID_RESULT;
		if(readWrite == READ)
		{
			//l1
			if(dbg) cout << "l1 trying read from 0x" << std::hex << address << std::dec << endl;
			result = read(address, L1_CACHE);
			data.update(result, L1_CACHE);
			if(result == READ_HIT)
			{
				if(dbg) cout << "l1 read hit at 0x" << std::hex << address << std::dec << endl;
				return;
			}
			else if(result == READ_MISS_INSERT_INVALID || result == READ_MISS_EVICT_CLEAN)
			{
				if(dbg) cout << "l1 read miss insert invalid/evict clean at 0x" << std::hex << address << std::dec << endl;
			}
			else if(result == READ_MISS_EVICT_DIRTY)
			{
				if(dbg) cout << "l1 read miss evict dirty at 0x" << std::hex << address << std::dec << endl;
				write(dirty.second, L2_CACHE); //TODO by inclusiveness write will always be sucessful - assert this
				dirty.first = false;
			}

			//l2
			if(dbg) cout << "l2 trying read from 0x" << std::hex << address << std::dec << endl;
			result = read(address, L2_CACHE);
			data.update(result, L2_CACHE);
			if(result == READ_HIT)
			{
				if(dbg) cout << "l2 read hit at 0x" << std::hex << address << std::dec << endl;
				return;
			}
			if(result == READ_MISS_INSERT_INVALID)
			{
				if(dbg) cout << "l2 read miss insert invalid/evict clean at 0x" << std::hex << address << std::dec << endl;
				return;
			}
			else if(result == READ_MISS_EVICT_CLEAN)
			{
				if(dbg) cout << "l2 read miss insert invalid/evict clean at 0x" << std::hex << address << std::dec << endl;
			}
			else
				if(dbg) cout << "l2 read miss evict dirty at 0x" << std::hex << address << std::dec << endl;
			evict(dirty.second, L1_CACHE);
			dirty.first = false;
		}

		else if(readWrite == WRITE)
		{
			//l1
			if(dbg) cout << "l1 trying write to 0x" << std::hex << address << std::dec << endl;
			result = write(address, L1_CACHE);
			data.update(result, L1_CACHE);
			if(result == WRITE_HIT)
			{
				if(dbg) cout << "l1 write hit at 0x" << std::hex << address << std::dec << endl;
				return;
			}
			if(result == WRITE_MISS_NO_ALLOC)
			{
				if(dbg) cout << "l1 write miss w/o alloc 0x" << std::hex << address << std::dec << endl;
				result = write(address, L2_CACHE); //TODO fix write allocate
				data.update(result, L2_CACHE);
				return;
			}
			else if(result == WRITE_MISS_INSERT_INVALID || result == WRITE_MISS_EVICT_CLEAN)
			{
				if(dbg) cout << "l1 write miss w/ alloc insert invalid/evict clean at 0x" << std::hex << address << std::dec << endl;
			}
			else if(result == WRITE_MISS_EVICT_DIRTY)
			{
				if(dbg) cout << "l1 write miss w/ alloc evict dirty at 0x" << std::hex << address << std::dec << endl;
				write(dirty.second, L2_CACHE); //TODO by inclusiveness write will always be sucessful - assert this
				dirty.first = false;
			}

			//l2
			if(dbg) cout << "l2 trying write to 0x" << std::hex << address << std::dec << endl;
			result = write(address, L2_CACHE);
			data.update(result, L2_CACHE);
			if(result == WRITE_HIT)
			{
				if(dbg) cout << "l2 write hit at 0x" << std::hex << address << std::dec << endl;
				return;
			}
			if(result == WRITE_MISS_INSERT_INVALID)
			{
				if(dbg) cout << "l2 write miss w/ alloc insert invalid/evict clean at 0x" << std::hex << address << std::dec << endl;
				return;
			}
			else if(result == WRITE_MISS_EVICT_CLEAN || result == WRITE_MISS_INSERT_INVALID)
				if(dbg) cout << "l2 write miss w/ alloc insert invalid/evict clean at 0x" << std::hex << address << std::dec << endl;
			else
			{
				if(dbg) cout << "l2 write miss evict dirty at 0x" << std::hex << address << std::dec << endl;
			}
			evict(dirty.second, L1_CACHE);
			dirty.first = false;
		}
	}

	void print()
	{
		cout << "\n===================================== MEMORY =====================================\n";
		l1.print(1);
		l2.print(2);
		cout << "\n===================================== MEMORY =====================================\n";
	}

	uint32_t calcTag(uint32_t address)
	{
		Line l(address, l2.dim, NOT_ACCESSED);
		return l.tag;
	}

	void printTag(uint32_t address)
	{
		cout << "tag of 0x" << std::hex << address << " = "; printBinary(address);
	}
};

/*=============================================================================
* main
=============================================================================*/
// int main()
// {
// 	int offsetSize = 0;
// 	int cacheSize = 1;
// 	int assoc = 0;
// 	int blockSize = 0;
// 	Memory mem(0, blockSize, cacheSize, cacheSize, assoc, assoc, 0, 0, 0);
// 	uint32_t address = 0xCAFEBABA;
// 	mem.print();
// 	for(int i = 0; i < std::pow(2, cacheSize); i++)
// 	{
// 		mem.memoryAccess(address, READ);
// 		mem.memoryAccess(address, WRITE);
// 		mem.print();
// 		address++;
// 	}
// 	mem.memoryAccess(address, READ);
// 	mem.print();
// }

int main(int argc, char **argv)
{

	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument§
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

	Memory mem(MemCyc, BSize, L1Size, L2Size, L1Assoc, L2Assoc, L1Cyc, L2Cyc, WrAlloc);
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
		if(dbg) cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		if(dbg) cout << ", address (hex)" << cutAddress;

		unsigned long int num = 0;
		num = strtoul(cutAddress.c_str(), NULL, 16);
		
		// DEBUG - remove this line
		if(dbg) cout << " (dec) " << num << endl;

		int readWrite = (operation == 'r') ? READ : WRITE;
		mem.memoryAccess(num, readWrite);
	}

	double L1MissRate = mem.data.missRate(L1_CACHE);
	double L2MissRate = mem.data.missRate(L2_CACHE);
	double avgAccTime = mem.data.missRate(0);

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}