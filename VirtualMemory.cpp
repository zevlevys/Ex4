#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include "math.h"

typedef struct{
    uint64_t first;
    uint64_t second;
} uint_pair;

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

uint64_t concatInts(uint64_t first, uint64_t second, uint64_t width){
    return  (first << width) | second;
}


void VMinitialize() {
    clearTable(0);
}


//todo: not finished- only looks for empty
uint64_t findUsableFrame(uint64_t frameIndex, uint64_t* maxFrame, int depth) {
    //look for empty table with DFS
    if (depth >= TABLES_DEPTH){
        return 0;
    }
    bool emptyFrame = true;
    for (uint64_t i = 0; i < PAGE_SIZE; i++){
        word_t nextTable;
        PMread(frameIndex + i, &nextTable);
        if ((uint64_t)nextTable != 0) {
            if ((uint64_t)nextTable > *maxFrame){
                *maxFrame = (uint64_t)nextTable;
            }
            emptyFrame = false;
            uint64_t childReturn = findUsableFrame(nextTable, maxFrame, depth + 1);
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

uint64_t findUsableFrameWrapper(){
    uint64_t maxFrameFound = 0;
    uint64_t ret = findUsableFrame(0, &maxFrameFound, 0);
    if (ret == 0 && maxFrameFound + 1 < NUM_FRAMES){
        return maxFrameFound + 1;
    }
    return ret; //may be zero
}


//int isTableEmpty(int pageIndex) {
//    for (int j = 0; j < PAGE_SIZE; j++){
//        if
//    }
//}


uint64_t cycDistance(uint64_t page, uint64_t page_to_swap_in){
    return fmin(NUM_PAGES - labs(page_to_swap_in - page), labs(page_to_swap_in - page));
}


uint_pair chooseEvicted(uint64_t pageToSwapIn, uint64_t frameIndex, int depth, int page_number){
    //reached leaf
    if (depth >= TABLES_DEPTH){
        uint_pair pair;
        pair.first = page_number;
        pair.second = cycDistance(page_number, pageToSwapIn)
        return pair;
    }
    uint64_t maxCycDistance= 0;
    for (uint64_t j = 0; j < PAGE_SIZE; j++){
        word_t nextTable;
        PMread(frameIndex + j, &nextTable);
        if ((uint64_t)nextTable != 0) {
            //todo: check this calculation of the page offset concat
            uint_pair ret;
            ret =  chooseEvicted(pageToSwapIn, nextTable, depth+1, concatInts(j, page_number, OFFSET_WIDTH * (depth+1)));
            maxCycDistance = fmaxl(ret.second, maxCycDistance);
        }
    }
    uint_pair final;
    final.first = page_number;
    final.second = maxCycDistance;
    return final;
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
