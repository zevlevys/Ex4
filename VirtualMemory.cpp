#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include "math.h"

typedef struct{
    uint64_t first;
    uint64_t second;
} uint_pair;
typedef struct{
    uint64_t first;
    uint64_t second;
    uint64_t third;
} uint_triple;



int traverse(uint64_t virtualAddress);

uint64_t getPageNumber(uint64_t address);

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

uint64_t getXthBatch(uint64_t address, uint64_t batchNumber) {
    return (address >> batchNumber * OFFSET_WIDTH) % PAGE_SIZE;
}

uint64_t getPageNumber(uint64_t address) {
    return address >> OFFSET_WIDTH;
}

int traverse(uint64_t virtualAddress) {
    uint64_t pageNumber = getPageNumber(virtualAddress);
    uint64_t address = 0;
    uint64_t old_address;
    for (int i = TABLES_DEPTH; i >= 0; i--) {
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


uint_triple chooseEvicted(uint64_t pageToSwapIn, uint64_t frameIndex, int depth, int page_number){
    //reached leaf
    if (depth >= TABLES_DEPTH){
        uint_triple triple;
        triple.first = page_number;
        triple.second = cycDistance(page_number, pageToSwapIn);
        triple.third = frameIndex;
        return triple;
    }
    uint64_t maxCycDistance= 0;
    for (uint64_t j = 0; j < PAGE_SIZE; j++){
        word_t nextTable;
        PMread(frameIndex + j, &nextTable);
        if ((uint64_t)nextTable != 0) {
            //todo: check this calculation of the page offset concat
            uint_triple ret;
            ret =  chooseEvicted(pageToSwapIn, nextTable, depth+1, concatInts(j, page_number, OFFSET_WIDTH * (depth+1)));
            maxCycDistance = fmaxl(ret.second, maxCycDistance);
        }
    }
    uint_triple final;
    final.first = page_number;
    final.second = maxCycDistance;
    final.third = frameIndex;
    return final;
}


uint_pair chooseEvictedWrapper(uint64_t _pageToSwap){
    uint_triple evict_ret = chooseEvicted(_pageToSwap, 0, 0, 0);
    uint_pair final;
    final.first = evict_ret.first;
    final.second = evict_ret.third;
    return final;
}


uint64_t chooseFrame(uint64_t pageToSwap, uint64_t){
    uint64_t usableFrame = findUsableFrameWrapper();
    if (usableFrame != 0){
        return usableFrame;
    }
    uint_pair evictionCandidate = chooseEvictedWrapper(pageToSwap);
    //todo: make sure we don't anything we want for this page/table
    PMevict(evictionCandidate.second, evictionCandidate.first);
    return evictionCandidate.second;
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
    uint64_t frame_address = traverse(virtualAddress);
    PMwrite(frame_address, value);
    return 1;
}

int VMread(uint64_t virtualAddress, word_t *value) {
    uint64_t frame_address = traverse(virtualAddress);
    PMread(frame_address, value);
    return 1;
}
