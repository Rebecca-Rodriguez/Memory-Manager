# Memory Manager

Memory Manager is a C++ class that provides memory management functionality, including allocation and deallocation of memory blocks. It is designed to manage a block of memory divided into fixed-sized words, where each word is aligned to the native word size of the system.

## Features
* Supports allocation of memory blocks of varying sizes.
* Uses different allocation algorithms such as best fit and worst fit.
* Keeps track of allocated and free memory blocks using a list of nodes.
* Provides methods to initialize and shutdown the memory manager.
* Allows retrieval of the memory block address, memory limit, word size, and memory map.
* Supports freeing of allocated memory blocks for reuse.
* Provides the option to set a custom allocator function for finding memory holes.

## Usage

1. Create an instance of the `MemoryManager` class by passing the native word size (in bytes) and an allocator function as arguments to the constructor.
2. Initialize the memory manager by calling the `initialize` method and passing the desired size of the memory block in words.
3. Allocate memory by calling the `allocate` method and passing the size of the memory block in bytes.
4. Use the allocated memory as needed.
5. Free the allocated memory by calling the `free` method and passing the memory block address.
6. Shutdown the memory manager by calling the `shutdown` method when no longer needed.

```
cpp

#include "MemoryManager.h"

// Create an instance of MemoryManager with native word size of 2 bytes and best fit allocator
MemoryManager memoryManager(2, bestFit);

// Initialize the memory manager with a memory block of size 1024 words (2048 bytes)
memoryManager.initialize(1024);

// Allocate a memory block of size 512 bytes
void* ptr = memoryManager.allocate(512);

// Use the allocated memory block as needed

// Free the allocated memory block
memoryManager.free(ptr);

// Shutdown the memory manager
memoryManager.shutdown();
```

## Note

* The code provided includes two allocation algorithms: best fit and worst fit. You can choose the allocator function by passing it to the `MemoryManager` constructor or by calling the `setAllocator` method.
* The `dumpMemoryMap` method can be used to generate a memory map in a file, which shows the allocated and free memory blocks.
* The `getList` and `getBitmap` methods can be used to retrieve the list of nodes and the bitmap of the memory block, respectively.
* The `getWordSize` method can be used to retrieve the native word size.
* The `getMemoryStart` and `getMemoryLimit` methods can be used to retrieve the memory block address and memory limit, respectively.

Please refer to the comments in the code for detailed explanations of the methods and data members.
