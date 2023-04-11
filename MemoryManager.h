#include <cmath>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>

struct Node
{
	int head;
	int len;
	bool isHole;

	Node(int _head, int _len, bool _isHole);
};

class MemoryManager
{
	private:
		unsigned natWordSize = 0;
		std::function<int(int, void *)> defAllocator;
		unsigned memoryLimit = 0;				// this should be wordSize * sizeInWords
		char *memBlock;							// byte-wise memory address of the beginning of the memory block
		std::vector<Node> holeList = {};		// vector to keep track of open holes

	public:
		MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
		~MemoryManager();
		void initialize(size_t sizeInWords);
		void shutdown();
		void *allocate(size_t sizeInBytes);
		void free(void *address);
		void setAllocator(std::function<int(int, void *)> allocator);
		int dumpMemoryMap(char *filename);
		void *getList();
		void *getBitmap();
		unsigned getWordSize();
		void *getMemoryStart();
		unsigned getMemoryLimit();
};

int bestFit(int sizeInWords, void *list);
int worstFit(int sizeInWords, void *list);
