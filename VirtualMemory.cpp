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

typedef struct {
    uint64_t first;
    uint64_t second;
    uint64_t third;
    uint64_t fourth;
} uint_quad;

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

uint_pair
findUsableFrame(uint64_t frameIndex, uint64_t *maxFrame, int depth, uint64_t prevPage, uint64_t parentAddress);

uint_pair findUsableFrameWrapper(uint64_t prevPage);

uint_quad chooseEvicted(uint64_t pageToSwapIn, uint64_t frameIndex, int depth, int page_number, uint64_t parentAddress);

uint64_t cycDistance(uint64_t page, uint64_t page_to_swap_in);


uint_triple chooseEvictedWrapper(uint64_t _pageToSwap);

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
    bool dont_evict = false;
    for (int i = TABLES_DEPTH; i > 0; i--) {
//        std::cout << "in traverse, i = " << i << std::endl;
        old_address = address;
        uint64_t sub = addressSubstr(virtualAddress, i);
        PMread(address * PAGE_SIZE + sub, (word_t *) &address);
        if (address == 0) {
//            std::cout << "sub = " << sub << std::endl;
            uint64_t frameIndex;
            //todo: how do we choose when not to evict the parent node of what ever we are looking for?
            if (dont_evict) {
                frameIndex = chooseFrame(pageNumber, old_address); //todo: dont eveict stuff we are using
            } else {
                frameIndex = chooseFrame(pageNumber, -1); //todo: dont eveict stuff we are using
            }
//            std::cout << "frameIndex = " << frameIndex << std::endl;
            clearTable(frameIndex);
            PMwrite(old_address * PAGE_SIZE + sub, (word_t) frameIndex);
            address = frameIndex;
            _printRAM();
            dont_evict = true;
        } else {
            dont_evict = false;
        }
    }
    PMrestore(address, pageNumber);
//    uint64_t sub = addressSubstr(virtualAddress, 0);
//    PMwrite(address * PAGE_SIZE + sub, (word_t) address);

    return address;
}


//todo: not finished- only looks for empty
uint_pair
findUsableFrame(uint64_t frameIndex, uint64_t *maxFrame, int depth, uint64_t prevPage, uint64_t parentAddress) {
    //look for empty table with DFS
    if (depth >= TABLES_DEPTH) {
        uint_pair pair;
        pair.first = 0;
        pair.second = parentAddress;
        return pair;
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

            uint_pair childReturn = findUsableFrame(nextTable, maxFrame, depth + 1, prevPage,
                                                    frameIndex * PAGE_SIZE + i);
            if (nextTable == prevPage) {
                continue;
            }
            if (childReturn.first != 0) { //found an empty frame
                return childReturn;
            }
        }
    }
    uint_pair pair;
    pair.second = parentAddress;
    if (emptyFrame) {
        pair.first = frameIndex;
    } else {
        pair.first = 0;
    }
    return pair;
}

uint_pair findUsableFrameWrapper(uint64_t prevPage) {
    uint64_t maxFrameFound = 0;
    uint_pair ret = findUsableFrame(0, &maxFrameFound, 0, prevPage, 0);
    if (ret.first == 0 && maxFrameFound + 1 < NUM_FRAMES) {
        uint_pair pair;
        pair.first = maxFrameFound + 1;
        pair.second = -1;
        return pair;
    }
    return ret; //may be zero
}


uint64_t cycDistance(uint64_t page, uint64_t page_to_swap_in) {
    return fmin(NUM_PAGES - labs(page_to_swap_in - page), labs(page_to_swap_in - page));
}


uint_quad
chooseEvicted(uint64_t pageToSwapIn, uint64_t frameIndex, int depth, int page_number, uint64_t parentAddress) {
    //reached leaf
    if (depth >= TABLES_DEPTH) {
        uint_quad quad;
        quad.first = page_number;
        quad.second = cycDistance(page_number, pageToSwapIn);
        quad.third = frameIndex;
        quad.fourth = parentAddress;
        return quad;
    }
    uint64_t maxCycDistance = 0;
    uint64_t maxPageNumber = 0;
    uint64_t maxFrameIndex = 0;
    uint64_t maxParentAddress = 0;
    for (uint64_t j = 0; j < PAGE_SIZE; j++) {
        word_t nextTable;
        PMread(frameIndex * PAGE_SIZE + j, &nextTable);
        if ((uint64_t) nextTable != 0) {
            //todo: check this calculation of the page offset concat
            uint_quad ret;
            ret = chooseEvicted(pageToSwapIn, nextTable, depth + 1,
                                concatInts(page_number, j, OFFSET_WIDTH), frameIndex * PAGE_SIZE + j);
            maxCycDistance = fmaxl(ret.second, maxCycDistance);
            if (maxCycDistance == ret.second) {
                maxPageNumber = ret.first;
                maxFrameIndex = ret.third;
                maxParentAddress = ret.fourth;
            }
        }
    }
    uint_quad final;
    final.first = maxPageNumber;
    final.second = maxCycDistance;
    final.third = maxFrameIndex;
    final.fourth = maxParentAddress;
    return final;
}


uint_triple chooseEvictedWrapper(uint64_t _pageToSwap) {
    uint_quad evict_ret = chooseEvicted(_pageToSwap, 0, 0, 0, 0);
    uint_triple final;
    final.first = evict_ret.first;
    final.second = evict_ret.third;
    final.third = evict_ret.fourth;
    return final;
}


uint64_t chooseFrame(uint64_t pageToSwap, uint64_t _prevPage) {
    uint_pair usableFrame = findUsableFrameWrapper(_prevPage);
    if (usableFrame.first != 0) {
        if (usableFrame.second != -1) {
            PMwrite(usableFrame.second, 0);
        }
        return usableFrame.first;

    }
    uint_triple evictionCandidate = chooseEvictedWrapper(pageToSwap);
    //todo: make sure we don't anything we want for this page/table
    PMevict(evictionCandidate.second, evictionCandidate.first);
    PMwrite(evictionCandidate.third, 0);
    return evictionCandidate.second;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    uint64_t frame_address = traverse(virtualAddress);
    uint64_t sub = addressSubstr(virtualAddress, 0);
    PMwrite(frame_address * PAGE_SIZE + sub, value);
    return 1;
}

int VMread(uint64_t virtualAddress, word_t *value) {
    uint64_t frame_address = traverse(virtualAddress);
    uint64_t sub = addressSubstr(virtualAddress, 0);
    PMread(frame_address * PAGE_SIZE + sub, value);
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
std::vector<word_t> _getRam() {
    std::vector<word_t> RAM(RAM_SIZE);
    word_t tempWord;
    for (uint64_t i = 0; i < NUM_FRAMES; i++) {
        for (uint64_t j = 0; j < PAGE_SIZE; j++) {
            PMread(i * PAGE_SIZE + j, &tempWord);
            RAM[i * PAGE_SIZE + j] = tempWord;
        }
    }
    return RAM;
}


/**
 * print the current state of the pysical memory. feel free to use this function is Virtual Memory for debuging.
 */
void _printRAM() {
    std::cout << "---------------------" << '\n';
    std::cout << "Physical Memory:" << '\n';
    std::vector<word_t> RAM = _getRam();

    if (printType) {
        for (uint64_t i = 0; i < NUM_FRAMES; i++) {
            std::cout << "frame " << i << ":\n";
            for (uint64_t j = 0; j < PAGE_SIZE; j++) {
                std::cout << "(" << j << ") " << RAM[i * PAGE_SIZE + j] << "\n";
            }
            std::cout << "-----------" << "\n";
        }
    } else {

        std::cout << "FRAME INDICES -\t";
        for (uint64_t i = 0; i < NUM_FRAMES; i++) {
            std::cout << "F" << i << ": (";
            for (uint64_t j = 0; j < PAGE_SIZE - 1; j++) {
                std::cout << j << ",\t";
            }
            std::cout << PAGE_SIZE - 1 << ")\t";
        }
        std::cout << '\n';
        std::cout << "DATA -\t\t\t";
        for (uint64_t i = 0; i < NUM_FRAMES; i++) {
            std::cout << "     ";
            for (uint64_t j = 0; j < PAGE_SIZE - 1; j++) {
                std::cout << RAM[i * PAGE_SIZE + j] << ",\t";
            }
            std::cout << RAM[i * PAGE_SIZE + PAGE_SIZE - 1] << " \t";
        }
        std::cout << '\n';
    }

    std::cout << "---------------------" << '\n';
}
