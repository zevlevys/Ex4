#include "VirtualMemory.h"
#include "PhysicalMemory.h"

int VMreadIn(uint64_t pageNumber, uint64_t offset, int depth);

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

void VMinitialize() {
    clearTable(0);
}

uint64_t getXthBatch(uint64_t address, uint64_t batchNumber) {
    return (address >> batchNumber * OFFSET_WIDTH) % PAGE_SIZE;
}

int VMread(uint64_t virtualAddress, word_t *value) {
    return 1;
}

int traverse(uint64_t virtualAddress, word_t *value) {
    uint64_t pageNumber = getPageNumber(virtualAddress);
    uint64_t address = 0;
    uint64_t old_address;
    for (int i = TABLES_DEPTH; i > 0; i--) {
        old_address = address;
        uint64_t sub = getXthBatch(virtualAddress, i);
        PMread(address * PAGE_SIZE + sub, (word_t *) &address);
        if (address == 0) {
            uint64_t frameIndex = getAvailableFrame(...); //todo: dont eveict stuff we are using
            if (i > 0) { //write zeros in frameIndex if its a table
                clearTable(frameIndex);
            } else { //restore page from memory if its an actual page
                PMrestore(frameIndex, pageNumber);
            }
            PMread(old_address * PAGE_SIZE + sub, (word_t *) frameIndex);
            address = frameIndex;
        }
    }
    return address;
}


//        word_t addr = 0;
//        for (int i = 0; i < TABLES_DEPTH; i++) {
//            PMread(addr + 5, &addr);
//            if (addr == 0) {
//            }
//        }

/*
 * 1 split into offset and page number
 * 2 recursively on the page number (MSB first):
 *      2.1 if there is no mapping to the physical memory:
 *          2.1.1 if there is an unused page:
 *              2.1.1.1 map it
 *              2.1.1.2 call next iteration
 *          2.1.2 else:
 *              2.1.2.1 smart remove somehow
 *              2.1.2.2 map the evicted page
 *              2.1.2.3 call next iteration
 *      2.2 else:
 *          2.2.1 call next iteration
 * 3 when we get to the virtual memory
 * 4 if the frame is unused:
 *      4.1 call PMrestor
 * 5 call PMwrite
 */
//traverse tree TREE_DEPTH times, each time do:
//translate part of the offset: PMread(addr*page_size + x, addr)
//if(addr == 0) then
//Find an unused frame or evict a page from some frame. Suppose this frame number is f
//Clear it if it is table
//PMwrite(0+x, f)
//addr = f
//PMRead(addr * page_size + x, value)


int VMwrite(uint64_t virtualAddress, word_t value) {
    //traverse tree TREE_DEPTH times, each time do:
    //translate part of the offset: PMread(addr*page_size + x, addr)
    //if(addr == 0) then
    //Find an unused frame or evict a page from some frame. Suppose this frame number is f
    //Clear it if it is table
    //PMwrite(0+x, f)
    //addr = f
    //PMwrite(addr * page_size + x, value)

    return 1;
}
