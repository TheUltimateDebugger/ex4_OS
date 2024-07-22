#include <cmath>
#include "VirtualMemory.h"
#include "PhysicalMemory.h"

//the offset of the smaller and top layer of the tree (in bytes)
#define SMALL_OFFSET (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) % OFFSET_WIDTH

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


void VMinitialize(){
  // initialize everything to zero. the tree is yet to be created
  for (uint64_t i = 0; i < RAM_SIZE; ++i)
  {
    PMwrite (i, 0);
  }
}


int VMread(uint64_t virtualAddress, word_t* value){

}


int VMwrite(uint64_t virtualAddress, word_t value){

}


