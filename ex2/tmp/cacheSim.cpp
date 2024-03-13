#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

#include <vector>
using std::vector;
using std::pow;

constexpr int FULLY_ASSOCIATIVE = -1;
constexpr int NOT_ACCESSED = -1;
constexpr bool READ = false;
constexpr bool WRITE = true;

/*32bits of address:
* x bits of block size -> offset
* y bits of cache size - y - x bits for numer of lines in cache = set bits
* z = 32 - y - x tag bits
*/

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
};

struct Address
{
	uint32_t address;
	uint32_t offset;
	uint32_t set;
	uint32_t tag;
	Address(uint32_t address, unsigned offsetSize, unsigned setSize, unsigned tagSize) : address(address)
	{
		int offsetMask = (1 << offsetSize) - 1;
		offset = address & offsetMask;
		address >>= offsetSize;
		if(setSize > 0)
		{
			int setMask = (1 << setSize) - 1;
			set = address & setMask;
			address >>= setSize;
		}
		else
			set = 0;
		int tagMask = (1 << tagSize) - 1;
		tag = address & tagMask;
	}
	void print()
	{
		cout ;
		cout << std::hex << "address " << "0x" << address << endl << std::dec;
		cout << 
	}
};

struct Entry
{
    uint32_t tag;
    bool valid;
    bool dirty;
    int lastAccessed;
    Entry() : tag(0), valid(false), dirty(false), lastAccessed(NOT_ACCESSED) {}
    Entry(uint32_t tag, int accessNumber) : tag(tag), valid(true), dirty(false), lastAccessed(accessNumber) {}
};


struct Cache
{
	unsigned cacheSize;
	unsigned blockSize;
	unsigned offsetSize;
	unsigned setSize;
	unsigned tagSize;
    vector<Entry> entries;
    int assoc;
    int accessNumber;

	Cache(int cacheSize, int blockSize, int assoc) :
	cacheSize(cacheSize), blockSize(blockSize), assoc(assoc)
	{
		unsigned lineSize = blockSize;
		unsigned lineNumber = cacheSize - lineSize;
		entries.resize(pow(2, lineNumber));
		setSize = assoc == FULLY_ASSOCIATIVE ? 0 : lineNumber - assoc;
		tagSize = 32 - offsetSize - setSize;
	}
	
    Entry& findEntryByAddress(uint32_t _address)
    {
		Address address = Address(_address, offsetSize, setSize, tagSize);
		if(assoc == FULLY_ASSOCIATIVE)
		{
			for(Entry& entry : entries)
			{
				if(entry.tag == address.tag && entry.valid)
					return entry;
			}
		}
		else //n way associative
		{
			unsigned partition = entries.size() / pow(2, assoc);
			for(int i = 0; i < std::pow(2, assoc); i++)
			{
				unsigned addressInCache = i * partition + pow(2, address.set); //TODO make sure pow() is necessary here
				if(entries[addressInCache].tag == address.tag)
					return entries[addressInCache];
			}
		} 
		Entry notFoundEntry = Entry();
		return notFoundEntry;
    }

	// vector<unsigned>& entryIndexOptions(uint32_t address)
	// {
	// 	vector<unsigned> options;
	// 	if(assoc == FULLY_ASSOCIATIVE)
	// 	{
	// 		for(int i = 0; i < entries.size(); i++)
	// 	}
	// }

	void placeEntry(uint32_t address)
	{
		Entry& entry = findEntryByAddress(address);

	}

	Entry& lruEntry()
	{
		Entry& lruEntry = entries[0];
		for(Entry& entry : entries)
		{
			if(lruEntry.lastAccessed > entry.lastAccessed)
				lruEntry = entry;
		}
		return lruEntry;
	}

};

struct Memory
{
    AccessData data;
    Cache l1;
    Cache l2;
    bool writeAllocate;
	bool writeBack;
    // Entry& findEntryByAddress(uint32_t address)
    // {

    // }
    void handleAccess(bool readWrite, uint32_t address) //TODO add logging
    {
		Entry& entry = l1.findEntryByAddress(address);
		if(entry.valid) //entry in l1
		{
			if(readWrite == READ) // read l1 hit
			{
				entry.lastAccessed = l1.accessNumber++;
				return;
			}
			else if(readWrite == WRITE) //write l1 hit
			{
				entry.lastAccessed = l1.accessNumber++;
				if(writeBack)
				{
					entry.dirty = true;
					return;
				}
				else //write through
				{
					;//write to l2 + memory
					return;
				}
			}
		}
		entry = l2.findEntryByAddress(address);
		if(entry.valid)
		{
			if(readWrite == READ) // read l2 hit
			{
				entry.lastAccessed = l2.accessNumber++; //TODO check if when fixing l1 read miss do we evict from l2
				//TODO handle read miss in l1 cache
				return;
			}
			else if(readWrite == WRITE) //write l2 hit
			{
				entry.lastAccessed = l2.accessNumber++;
				if(writeBack)
				{
					entry.dirty = true;
				}
				else //write through
				{
					;//write to memory
				}
				//TODO handle write miss in l1 cache //TODO check if when fixing l1 read miss do we evict from l2
			}
		}
		//handle read/write miss in both caches
		if(readWrite == READ)
		{
			//TODO bring from memory to l1, l2
			//create entry, update accessNumber of cache and lastAccessed of entry
			return;
		}
		else if(readWrite == WRITE)
		{
			if(!writeAllocate)
			{
				//write to memory
			}
			else //write allocate
			{
				//TODO bring from memory to l1, l2
				//create entry, update accessNumber of cache and lastAccessed of entry
				if(writeBack)
				{
					//TODO dirty bit on new entry
				}
				else //write through
				{
					//TODO write to l2 + memory
				}
			}
		}
    }
};

int main()
{
	uint32_t address = 0x1234;
	printBinary(address);
	// Address add = Address(address);

}

int _main(int argc, char **argv) {

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

	//your code here

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
