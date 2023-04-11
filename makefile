all:
	c++ -std=c++17 -o MemoryManager.o -c MemoryManager.cpp
	ar cr libMemoryManager.a MemoryManager.o

clean:
	rm MemoryManager.o
	rm libMemoryManager.a