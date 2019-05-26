#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

void VMinitialize() {
    clearTable(0);
}


int VMread(uint64_t virtualAddress, word_t* value) {
    //traverse tree TREE_DEPTH times, each time do:
        //translate part of the offset: PMread(addr*page_size + x, addr)
        //if(addr == 0) then
        //Find an unused frame or evict a page from some frame. Suppose this frame number is f
        //Clear it if it is table
        //PMwrite(0+x, f)
        //addr = f
    //PMRead(addr * page_size + x, value)
    return 1;
}


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
