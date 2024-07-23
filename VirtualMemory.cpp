#include <cmath>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

//the offset of the smaller and top layer of the tree (in bytes)
#define SMALL_OFFSET (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) % OFFSET_WIDTH

uint64_t min(uint64_t a, uint64_t b) {
  return (a < b) ? a : b;
}

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

uint64_t find_unused_frame_recursive(uint64_t current_page_number,
                                     uint_least64_t
*max_used, uint64_t depth, uint64_t safe_frame){
  word_t value;
  uint64_t page_num, result = NUM_PAGES + 2;

  //updates max used tree
  *max_used += 1;


  if (depth == TABLES_DEPTH) //page is not part of the tree
    return NUM_PAGES + 1;

  for (int i = 0; i < pow(2, OFFSET_WIDTH); ++i)
  {
    PMread (current_page_number * PAGE_SIZE + i, &value);
    page_num = value;
    if (page_num != 0)
    {
      result = find_unused_frame_recursive (page_num, max_used, depth + 1,
                                            safe_frame);
      if (result != NUM_PAGES + 1 && result != NUM_PAGES + 2) //we have a
        // good result
        return result;
    }
  }
  //if result has not changed then current_page has no sons
  if (result == NUM_PAGES + 2)
  {
    //this is a page without children that is untouchable
    if (safe_frame == current_page_number)
      return NUM_PAGES + 1;
    result = current_page_number;
  }

  return result;
}

uint64_t find_unused_frame(uint64_t safe_frame)
{
  uint64_t max_used = 1;
  word_t value;
  uint64_t page_num;
  uint64_t result = NUM_PAGES + 2;

  uint64_t offset_width = SMALL_OFFSET;
  if (offset_width == 0)
    offset_width = OFFSET_WIDTH;
  for (int i = 0; i < pow (2, offset_width); ++i)
  {
    PMread (i, &value);
    page_num = value;
    if (page_num != 0)
    {
      result = find_unused_frame_recursive (page_num, &max_used, 1, safe_frame);

      if (result != NUM_PAGES + 1 && result != NUM_PAGES + 2) //we have a
        // good result
        return result;
    }
  }
  //if result has not changed then 0 has no sons
  if (result == NUM_PAGES + 2)
  {
    if (max_used < NUM_PAGES)
      return 1;
    result = NUM_PAGES + 1;
  }
  if (max_used < NUM_FRAMES)
    return max_used;
  else
    return NUM_PAGES + 1;
}


bool evict_frame_recursive(uint64_t current_page_number, uint_least64_t
wanted_virtual_address, uint64_t *largest_distance, uint64_t *best_frame,
uint64_t *pointer_to_best_frame, uint64_t depth, uint64_t current_virtual_addr)
{
  if (depth == TABLES_DEPTH)
  {
    uint64_t current_distance = min(abs(current_virtual_addr -
                                        wanted_virtual_address),NUM_PAGES -
                                                                abs(current_virtual_addr - wanted_virtual_address));
    if (current_distance > *largest_distance)
    {
      *largest_distance = current_distance;
      *best_frame = current_page_number;
    }
    return true;
  }

  word_t value = 0;
  uint64_t page_num;


  for (uint64_t i = 0; i < pow(2, OFFSET_WIDTH); ++i)
  {
    PMread (current_page_number * PAGE_SIZE + i, &value);
    page_num = value;
    if (page_num != 0)
    {
      if (evict_frame_recursive(page_num, wanted_virtual_address, largest_distance,
                            best_frame, pointer_to_best_frame, depth+1,
                            current_virtual_addr * PAGE_SIZE +i))
      {
        *pointer_to_best_frame = current_page_number * PAGE_SIZE + i;
      }
    }
  }
  return false;
}

uint64_t evict_frame(uint64_t wanted_virtual_address)
{

  uint64_t largest_distance = 0;
  uint64_t best_frame = 0;
  uint64_t pointer_to_best_frame = 0;
  word_t value = 0;
  uint64_t page_num;

  uint64_t offset_width = SMALL_OFFSET;
  if (offset_width == 0)
    offset_width = OFFSET_WIDTH;
  for (uint64_t i = 0; i < pow(2, offset_width); ++i)
  {
    PMread (i, &value);
    page_num = value;
    if (page_num != 0)
    {
      if (evict_frame_recursive(page_num, wanted_virtual_address,
                                &largest_distance,
                            &best_frame, &pointer_to_best_frame, 1, i))
      {
        pointer_to_best_frame = i;
      }
    }
  }

  PMwrite (pointer_to_best_frame, 0);
  word_t value_to_remember;
  PMread (best_frame, &value_to_remember);
  PMevict (value_to_remember, best_frame);
  return best_frame;
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
    uint64_t safe_frame = NUM_PAGES + 1;

    for (int depth = 0; depth < TABLES_DEPTH; ++depth) {
        uint64_t page = getPage(currentAddress, depth);
        word_t entry;
        PMread(frame * PAGE_SIZE + page, &entry);

        if (entry == 0) {
            // Allocate a new frame if not already mapped
            uint64_t newFrame = find_unused_frame(safe_frame);
            if (newFrame == NUM_PAGES + 1){
                newFrame = evict_frame(virtualAddress >> OFFSET_WIDTH);
            }
            clearFrame(newFrame);
            PMwrite(frame * PAGE_SIZE + page, newFrame);
            entry = newFrame;
        }

        frame = entry;
        safe_frame = frame;
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
    uint64_t safe_frame = NUM_PAGES + 1;

    for (int depth = 0; depth < TABLES_DEPTH; ++depth) {
        uint64_t page = getPage(currentAddress, depth);
        word_t entry;
        PMread(frame * PAGE_SIZE + page, &entry);

        if (entry == 0) {
            // Allocate a new frame if not already mapped
            uint64_t newFrame = find_unused_frame(safe_frame);
            if (newFrame == NUM_PAGES + 1){
                newFrame = evict_frame(virtualAddress >> OFFSET_WIDTH);
            }
            clearFrame(newFrame);
            PMwrite(frame * PAGE_SIZE + page, newFrame);
            entry = newFrame;
        }

        frame = entry;
        safe_frame = frame;
    }

    PMwrite(frame * PAGE_SIZE + offset, value);
    return 1;
}


