#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include "math.h"
#include <iostream>
#include <vector>
typedef struct {
    uint64_t first;
    uint64_t second;
} uint_pair;

typedef struct {
    uint64_t first;
    uint64_t second;
    uint64_t third;
} uint_triple;


/**
 * Gives a substr of the input, of length OFFSET_WIDTH
 * @param virtualAddress
 * @param level
 * @return
 */
//todo:erase this declaration
void _printRAM();

uint64_t addressSubstr(uint64_t virtualAddress, int level);

int traverse(uint64_t virtualAddress);

uint64_t getPageNumber(uint64_t address);

uint64_t concatInts(uint64_t first, uint64_t second, uint64_t width);

uint64_t findUsableFrame(uint64_t frameIndex, uint64_t *maxFrame, int depth, uint64_t prevPage);

uint64_t findUsableFrameWrapper(uint64_t prevPage);

uint_triple chooseEvicted(uint64_t pageToSwapIn, uint64_t frameIndex, int depth, int page_number);

uint64_t cycDistance(uint64_t page, uint64_t page_to_swap_in);


uint_pair chooseEvictedWrapper(uint64_t _pageToSwap);

uint64_t chooseFrame(uint64_t pageToSwap, uint64_t _prevPage);

////////////////////////////////////

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}

uint64_t addressSubstr(uint64_t virtualAddress, int level) {
    return (virtualAddress >> (level * OFFSET_WIDTH)) % PAGE_SIZE;
}

uint64_t concatInts(uint64_t first, uint64_t second, uint64_t width) {
    return (first << width) | second;
}


void VMinitialize() {
    clearTable(0);
}


uint64_t getPageNumber(uint64_t address) {
    return address >> OFFSET_WIDTH;
}

int traverse(uint64_t virtualAddress) {
    uint64_t pageNumber = getPageNumber(virtualAddress);
    uint64_t address = 0;
    uint64_t old_address;
    for (int i = TABLES_DEPTH; i >= 0; i--) {
//        std::cout << "in traverse, i = " << i << std::endl;
        old_address = address;
        uint64_t sub = addressSubstr(virtualAddress, i);
        PMread(address * PAGE_SIZE + sub, (word_t *) &address);
        if (address == 0) {
//            std::cout << "sub = " << sub << std::endl;
            uint64_t frameIndex = chooseFrame(pageNumber, old_address); //todo: dont eveict stuff we are using
//            std::cout << "frameIndex = " << frameIndex << std::endl;
            if (i > 0) { //write zeros in frameIndex if its a table
                clearTable(frameIndex);
            } else { //restore page from memory if its an actual page
                PMrestore(frameIndex, pageNumber);
            }
            PMwrite(old_address * PAGE_SIZE + sub, (word_t) frameIndex);
            address = frameIndex;
            _printRAM();
        }
    }
    return address;
}


//todo: not finished- only looks for empty
uint64_t findUsableFrame(uint64_t frameIndex, uint64_t *maxFrame, int depth, uint64_t prevPage) {
    //look for empty table with DFS
    if (depth >= TABLES_DEPTH) {
        return 0;
    }
    bool emptyFrame = true;
    for (uint64_t i = 0; i < PAGE_SIZE; i++) {
        word_t nextTable;
        PMread(frameIndex * PAGE_SIZE + i, &nextTable);
        if ((uint64_t) nextTable != 0) {
            if ((uint64_t) nextTable > *maxFrame) {
                *maxFrame = (uint64_t) nextTable;
            }
            emptyFrame = false;
            if(nextTable == prevPage){
                continue;
            }
            uint64_t childReturn = findUsableFrame(nextTable, maxFrame, depth + 1, prevPage);
            if (childReturn != 0) { //found an empty frame
                return childReturn;
            }
        }
    }
    if (emptyFrame) {
        return frameIndex;
    }
    return 0;
}

uint64_t findUsableFrameWrapper(uint64_t prevPage) {
    uint64_t maxFrameFound = 0;
    uint64_t ret = findUsableFrame(0, &maxFrameFound, 0, prevPage);
    if (ret == 0 && maxFrameFound + 1 < NUM_FRAMES) {
        return maxFrameFound + 1;
    }
    return ret; //may be zero
}


uint64_t cycDistance(uint64_t page, uint64_t page_to_swap_in) {
    return fmin(NUM_PAGES - labs(page_to_swap_in - page), labs(page_to_swap_in - page));
}


uint_triple chooseEvicted(uint64_t pageToSwapIn, uint64_t frameIndex, int depth, int page_number) {
    //reached leaf
    if (depth >= TABLES_DEPTH) {
        uint_triple triple;
        triple.first = page_number;
        triple.second = cycDistance(page_number, pageToSwapIn);
        triple.third = frameIndex;
        return triple;
    }
    uint64_t maxCycDistance = 0;
    for (uint64_t j = 0; j < PAGE_SIZE; j++) {
        word_t nextTable;
        PMread(frameIndex + j, &nextTable);
        if ((uint64_t) nextTable != 0) {
            //todo: check this calculation of the page offset concat
            uint_triple ret;
            ret = chooseEvicted(pageToSwapIn, nextTable, depth + 1,
                                concatInts(j, page_number, OFFSET_WIDTH * (depth + 1)));
            maxCycDistance = fmaxl(ret.second, maxCycDistance);
        }
    }
    uint_triple final;
    final.first = page_number;
    final.second = maxCycDistance;
    final.third = frameIndex;
    return final;
}


uint_pair chooseEvictedWrapper(uint64_t _pageToSwap) {
    uint_triple evict_ret = chooseEvicted(_pageToSwap, 0, 0, 0);
    uint_pair final;
    final.first = evict_ret.first;
    final.second = evict_ret.third;
    return final;
}


uint64_t chooseFrame(uint64_t pageToSwap, uint64_t _prevPage) {
    uint64_t usableFrame = findUsableFrameWrapper(_prevPage);
    if (usableFrame != 0) {
        return usableFrame;
    }
    uint_pair evictionCandidate = chooseEvictedWrapper(pageToSwap);
    //todo: make sure we don't anything we want for this page/table
    PMevict(evictionCandidate.second, evictionCandidate.first);
    return evictionCandidate.second;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    uint64_t frame_address = traverse(virtualAddress);
//    std::cout << "in VMwrite" << std::endl;
    PMwrite(frame_address, value);
    return 1;
}

int VMread(uint64_t virtualAddress, word_t *value) {
    uint64_t frame_address = traverse(virtualAddress);
    PMread(frame_address, value);
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

//todo:erase this whole part and remove io, vector includes
//taken from flowexamplesimulation

static bool printType = false; // You may change this to True if you want.
std::vector<word_t> _getRam()
{
    std::vector<word_t> RAM(RAM_SIZE);
    word_t tempWord;
    for (uint64_t i = 0; i < NUM_FRAMES; i++)
    {
        for (uint64_t j = 0; j < PAGE_SIZE; j++)
        {
            PMread(i * PAGE_SIZE + j, &tempWord);
            RAM[i * PAGE_SIZE + j] = tempWord;
        }
    }
    return RAM;
}


/**
 * print the current state of the pysical memory. feel free to use this function is Virtual Memory for debuging.
 */
void _printRAM()
{
    std::cout << "---------------------" << '\n';
    std::cout << "Physical Memory:" << '\n';
    std::vector<word_t> RAM = _getRam();

    if (printType)
    {
        for (uint64_t i = 0; i < NUM_FRAMES; i++)
        {
            std::cout << "frame " << i << ":\n";
            for (uint64_t j = 0; j < PAGE_SIZE; j++)
            {
                std::cout << "(" << j << ") " << RAM[i * PAGE_SIZE + j] << "\n";
            }
            std::cout << "-----------" << "\n";
        }
    } else
    {

        std::cout << "FRAME INDICES -\t";
        for (uint64_t i = 0; i < NUM_FRAMES; i++)
        {
            std::cout << "F" << i << ": (";
            for (uint64_t j = 0; j < PAGE_SIZE - 1; j++)
            {
                std::cout << j << ",\t";
            }
            std::cout << PAGE_SIZE - 1 << ")\t";
        }
        std::cout << '\n';
        std::cout << "DATA -\t\t\t";
        for (uint64_t i = 0; i < NUM_FRAMES; i++)
        {
            std::cout << "     ";
            for (uint64_t j = 0; j < PAGE_SIZE - 1; j++)
            {
                std::cout << RAM[i * PAGE_SIZE + j] << ",\t";
            }
            std::cout << RAM[i * PAGE_SIZE + PAGE_SIZE - 1] << " \t";
        }
        std::cout << '\n';
    }

    std::cout << "---------------------" << '\n';
}
