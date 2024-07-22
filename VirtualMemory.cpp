#include "VirtualMemory.h"
#include "PhysicalMemory.h"

//the offset of the smaller and top layer of the tree (in bytes)
#define SMALL_OFFSET (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) % OFFSET_WIDTH

uint64_t find_unused_frame_recursive(uint64_t current_page, uint_least64_t
*max_used){
  word_t value;
  uint64_t addr, result = -1;
  for (int i = 0; i < pow(2, OFFSET_WIDTH); ++i)
  {
    PMread (current_page, &value);
    addr = value;
    result = find_unused_frame_recursive (addr, max_used);
  }
  if (result == -1)
    if (max_used < NUM_FRAMES + 1)
      return max_used + 1;
    else
      return -1;
  return -1;
}

uint64_t find_unused_frame(){
  uint64_t max_used = 0;
  word_t value;
  uint64_t addr, result = -1;
  for (int i = 0; i < pow(2, SMALL_OFFSET); ++i)
  {
    PMread (0, &value);
    addr = value;
    result = find_unused_frame_recursive (addr, &max_used);

  }
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


