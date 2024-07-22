#include <cmath>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

//the offset of the smaller and top layer of the tree (in bytes)
#define SMALL_OFFSET (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) % OFFSET_WIDTH

// Helper functions
uint64_t getOffset(uint64_t address) {
    return address & ((1 << OFFSET_WIDTH) - 1);
}

uint64_t getPage(uint64_t address, int depth) {
    return (address >> (OFFSET_WIDTH * (TABLES_DEPTH - depth))) & ((1 << OFFSET_WIDTH) - 1);
}

void clearFrame(uint64_t frame) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frame * PAGE_SIZE + i, 0);
    }
}

uint64_t find_unused_frame_recursive(uint64_t current_page, uint_least64_t
*max_used, uint64_t depth){
  word_t value;
  uint64_t addr, result = -2;

  if (*max_used < current_page) //updates max used tree
    *max_used = current_page;

  if (depth == TABLES_DEPTH) //page is not part of the tree
    return -1;

  for (int i = 0; i < pow(2, OFFSET_WIDTH); ++i)
  {
    PMread (current_page, &value);
    addr = value;
    if (addr != 0)
    {
      result = find_unused_frame_recursive (addr, max_used, depth + 1);

      if (result != -1 && result != -2) //we have a good result
        return result;
    }
  }
  //if result has not changed then current_page has no sons
  if (result == -2)
    result = current_page;

  return result;
}

uint64_t find_unused_frame(){
  uint64_t max_used = 1;
  word_t value;
  uint64_t addr, result = -2;
  for (int i = 0; i < pow(2, SMALL_OFFSET); ++i)
  {
    PMread (0, &value);
    addr = value;
    if (addr != 0)
    {
      result = find_unused_frame_recursive (addr, &max_used, 1);

      if (result != -1 && result != -2) //we have a good result
        return result;
    }
  }
  //if result has not changed then 0 has no sons
  if (result == -2)
    return 0; //the current frame is 0
  if (result == -1)
    if (max_used < NUM_FRAMES + 1)
      return max_used + 1;
    else
      return -1;
}


uint64_t avict_frame()
{
    return 0;
}


void VMinitialize(){
  // initialize everything to zero. the tree is yet to be created
  for (uint64_t i = 0; i < RAM_SIZE; ++i)
  {
    PMwrite (i, 0);
  }
}


int VMread(uint64_t virtualAddress, word_t* value){
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
    {
        return 0; // Invalid address
    }

    uint64_t frame = 0; // Start from the root page table in frame 0
    uint64_t offset = getOffset(virtualAddress);
    uint64_t currentAddress = virtualAddress;

    for (int depth = 0; depth < TABLES_DEPTH; ++depth) {
        uint64_t page = getPage(currentAddress, depth);
        word_t entry;
        PMread(frame * PAGE_SIZE + page, &entry);

        if (entry == 0) {
            // Allocate a new frame if not already mapped
            uint64_t newFrame = find_unused_frame();
            if (newFrame == -1){
                newFrame = avict_frame();
            }
            clearFrame(newFrame);
            PMwrite(frame * PAGE_SIZE + page, newFrame);
            entry = newFrame;
        }

        frame = entry;
    }

    PMread(frame * PAGE_SIZE + offset, value);
    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
        return 0; // Invalid address
    }

    uint64_t frame = 0; // Start from the root page table in frame 0
    uint64_t offset = getOffset(virtualAddress);
    uint64_t currentAddress = virtualAddress;

    for (int depth = 0; depth < TABLES_DEPTH; ++depth) {
        uint64_t page = getPage(currentAddress, depth);
        word_t entry;
        PMread(frame * PAGE_SIZE + page, &entry);

        if (entry == 0) {
            // Allocate a new frame if not already mapped
            uint64_t newFrame = find_unused_frame();
            if (newFrame == -1){
                newFrame = avict_frame();
            }
            clearFrame(newFrame);
            PMwrite(frame * PAGE_SIZE + page, newFrame);
            entry = newFrame;
        }

        frame = entry;
    }

    PMwrite(frame * PAGE_SIZE + offset, value);
    return 1;
}


