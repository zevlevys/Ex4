#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}


/**
 * Gives a substr of the input, of length OFFSET_WIDTH
 * @param virtualAddress
 * @param level
 * @return
 */
uint64_t addressSubstr(uint64_t virtualAddress, int level) {
    return (((1LL << OFFSET_WIDTH) - 1) & (virtualAddress >> (level * OFFSET_WIDTH)));
}


void VMinitialize() {
    clearTable(0);
}

//todo: not finished I think (Zev)
uint64_t findUsableFrame(uint64_t frameIndex) {
    //look for empty table with DFS
    bool emptyFrame = true;
    for (uint64_t i = 0; i < PAGE_SIZE; i++){
        word_t nextTable;
        PMread(frameIndex + i, &nextTable);
        if ((uint64_t)nextTable != 0) {
            emptyFrame = false;
            uint64_t childReturn = findUsableFrame(nextTable);
            if(childReturn != 0){ //found an empty frame
                return childReturn;
            }
        }
    }
    if (emptyFrame){
        return frameIndex;
    }
    return 0;
}

//int isTableEmpty(int pageIndex) {
//    for (int j = 0; j < PAGE_SIZE; j++){
//        if
//    }
//}


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
