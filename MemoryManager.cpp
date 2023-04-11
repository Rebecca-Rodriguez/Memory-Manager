#include "MemoryManager.h"

Node::Node(int _head, int _len, bool _isHole)
{
	this->head = _head;
	this->len = _len;
	this->isHole = _isHole;
}

MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator)
{
	// Constructor; sets native word size (in bytes, for alignment) 
	// and default allocator for finding a memory hole.
	
	this->natWordSize = wordSize;
	this->defAllocator = allocator;
}

MemoryManager::~MemoryManager()
{
	// Releases all memory allocated by this object without leaking memory.
	
	shutdown();
	this->natWordSize = 0;
	this->memoryLimit = 0;
}

void MemoryManager::initialize(size_t sizeInWords)
{
	// Instantiates block of requested size, no larger than 65536 words;
	// cleans up previous block if applicable. (sizeInWords = numberOfWords)

	if(sizeInWords > 65536 || sizeInWords < 0)
	{
		return;
	}

	// cleans up previous block if applicable.
	if(!holeList.empty())
	{
		shutdown();
	}

	memoryLimit = sizeInWords * natWordSize;
	memBlock = new char[memoryLimit];

	Node firstNode = Node(0, sizeInWords, true);
	holeList.push_back(firstNode);
}

void MemoryManager::shutdown()
{
	// Releases memory block acquired during initialization, if any.
	// This should only include memory created for long term use not 
	// those that returned such as getList() or getBitmap() as whatever
	// is calling those should delete it instead.
	
	delete [] memBlock;
	memBlock = nullptr;
	holeList.clear();
}

void* MemoryManager::allocate(size_t sizeInBytes)
{
	// Allocates a memory using the allocator function. If no memory is
	// available or size is invalid, returns nullptr.

	sizeInBytes = (sizeInBytes - 1)/natWordSize + 1;
	uint16_t* list = static_cast<uint16_t*>(getList());
	int offset = defAllocator(sizeInBytes, list);
	delete[] list;

	if(offset == -1)
	{
		return nullptr;
	}

	int offsetIndex = -1;

	// traverse the list to find which hole is equal to offset
	for(int i = 0; i < holeList.size(); i++)
	{
		if (holeList[i].head == offset)
		{
			offsetIndex = i;
			break;
		}
	}

	// if matching offset not found
	if(offsetIndex == -1)
	{
		return nullptr;
	}

	if(holeList[offsetIndex].len == sizeInBytes)
	{
		holeList[offsetIndex].isHole = false;
	}
	else
	{
		int offsetHead = holeList[offsetIndex].head;
		Node insertNode = Node(offsetHead, sizeInBytes, false);
		holeList.insert(holeList.begin() + offsetIndex, insertNode);
		holeList[offsetIndex + 1].head = offsetHead + sizeInBytes;
		holeList[offsetIndex + 1].len = holeList[offsetIndex + 1].len - sizeInBytes;
	}

	return (void *)(memBlock + holeList[offsetIndex].head * natWordSize);
}

void MemoryManager::free(void *address)
{
	// Frees the memory block within the memory manager so that it can be reused.

	int offset = ceil(((char *) address - memBlock) / natWordSize);
	int offsetIndex = -1;

	// traverse the list to find which hole is equal to offset
	for(int i = 0; i < holeList.size(); i++)
	{
		if(holeList[i].head == offset)
		{
			offsetIndex = i;
			break;
		}
	}

	// if matching offset not found
	if(offsetIndex == -1)
	{
		return;
	}

	// Combine Holes
	// check to see if the NEXT node is a hole; remove it
	if(offsetIndex + 1 < holeList.size() && holeList[offsetIndex + 1].isHole)
	{
		holeList[offsetIndex].len += holeList[offsetIndex + 1].len;
		holeList.erase(holeList.begin() + offsetIndex + 1);
	}

	// check to see if the PREV node is a hole; remove it
	if(offsetIndex - 1 >=  0 && holeList[offsetIndex - 1].isHole)
	{
		holeList[offsetIndex].head = holeList[offsetIndex - 1].head;
		holeList[offsetIndex].len += holeList[offsetIndex - 1].len;
		holeList.erase(holeList.begin() + offsetIndex - 1);
		offsetIndex--;
	}
	holeList[offsetIndex].isHole = true;
}

void MemoryManager::setAllocator(std::function<int(int, void *)> allocator)
{
	// Changes the allocation algorithm to identifying the
	// memory hole to use for allocation.

	this->defAllocator = allocator;	
}

int MemoryManager::dumpMemoryMap(char *filename)
{
	// Uses standard POSIX calls to write hole list to filename as text,
	// returning -1 on error and 0 if successful.
	// Format: "[START, LENGTH] - [START, LENGTH] …", e.g., "[0, 10] - [12, 2] - [20, 6]"

	// call to creat() is equivalent to calling open() with flags equal to O_CREAT|O_WRONLY|O_TRUNC
	int file = creat(filename, 0666);

	// if file is not opened correctly
	if(file < 0)
	{
		return -1;
	}

	std::string memMap;

	// traverse list and only record the holes
	for (int i = 0; i < holeList.size(); i++)
	{
		if(holeList[i].isHole)
		{
			memMap += "[" + std::to_string(holeList[i].head) + ", " + std::to_string(holeList[i].len) + "] - ";
		}
	}

	memMap = memMap.substr(0, memMap.size() - 3);			// delete the last three characters from the end

	const char* buff = const_cast<char *>(memMap.c_str());
	if(write(file, buff, memMap.length()) != memMap.length())
	{
		return -1;
	}

	close(file);

	return 0;
}

void* MemoryManager::getList()
{
	// Returns an array of information (in decimal) about holes
	// for use by the allocator function (little-Endian).
	// Offset and length are in words. If no memory has been allocated,
	// the function should return a NULL pointer.

	std::vector<uint16_t> holes = {};
	uint16_t lenHoles = 0;

	// traverse list and get holes
	for(int i = 0; i < holeList.size(); i++)
	{
		if(holeList[i].isHole)
		{
			lenHoles++;
			holes.push_back(holeList[i].head);
			holes.push_back(holeList[i].len);
		}
	}

	auto listSize = (2 * lenHoles) + 1;
	uint16_t *list = new uint16_t[listSize];
	
	// the first value will hold the length of the holes
	list[0] = lenHoles;

	for(uint16_t i = 0; i < (2 * lenHoles); i++)
	{
		list[i + 1] = holes[i];
	}

	return list;
}

void* MemoryManager::getBitmap()
{
	// Returns a bit-stream of bits in terms of an array
	// representing whether words are used (1) or free (0).
	// The first two bytes are the size of the bitmap (little-Endian);
	// the rest is the bitmap, word-wise.

	int numByte = (int)((memoryLimit/natWordSize + 7)/8);
	uint8_t *list = new uint8_t[2 + numByte];
	list[0] = numByte % 256;
	list[1] = numByte >> 8;
	int numEight = 0;
	int bite = 0;

	// traverse the list of holes
	for(int i = 0; i < holeList.size(); i++)
	{
		int currLen = holeList[i].len;
		int bit = 1;

		// mark those that are free
		if(holeList[i].isHole)
		{
			bit = 0;
		}

		for(int j = 0; j < currLen; j++)
		{
			// left shift
			bite += (bit << numEight%8);
			numEight++;
			if(numEight%8 == 0)
			{
				list[numEight/8 + 1] = bite;
				bite = 0;
			}
		}
	}

	// add any straglers to the end
	if(numEight%8 != 0)
	{
		list[numByte + 1] = bite;
	}
	return list;
}

unsigned MemoryManager::getWordSize()
{
	// Returns the word size used for alignment.

	return natWordSize;
}

void* MemoryManager::getMemoryStart()
{
	// Returns the byte-wise memory address of the beginning of the memory block.

	if (memBlock)
	{
		return memBlock;
	}
	return nullptr;
}

unsigned MemoryManager::getMemoryLimit()
{
	// Returns the byte limit of the current memory block.

	return memoryLimit;
}

int bestFit(int sizeInWords, void *list)
{
	// Returns word offset of hole selected by the best fit
	// memory allocation algorithm, and -1 if there is no fit.

	int minLen = 65536;
	int offset = -1;

	uint16_t *holeList = static_cast<uint16_t *>(list);
	uint16_t holeListlength = *holeList++;

	for(uint16_t i = 1; i < (holeListlength) * 2; i += 2)
	{
		if(holeList[i] >= sizeInWords && holeList[i] < minLen)
		{
			offset = holeList[i - 1];
			minLen = holeList[i];
		}
	}

	return offset;
}

int worstFit(int sizeInWords, void *list)
{
	// Returns word offset of hole selected by the worst fit
	// memory allocation algorithm, and -1 if there is no fit.

	int maxLen = -1;
	int offset = -1;

	uint16_t *holeList = static_cast<uint16_t *>(list);
	uint16_t holeListlength = *holeList++;

	for(uint16_t i = 1; i < (holeListlength) * 2; i += 2)
	{
		if(holeList[i] >= sizeInWords && holeList[i] > maxLen)
		{
			offset = holeList[i - 1];
			maxLen = holeList[i];
		}
	}

	return offset;
}