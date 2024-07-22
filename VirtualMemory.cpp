#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void VMinitialize(){
  // initialize everything to zero. the tree is yet to be created
  for (uint64_t i = 0; i < pow(2, PHYSICAL_ADDRESS_WIDTH); ++i)
  {
    PMwrite (i, 0);
  }
}


int VMread(uint64_t virtualAddress, word_t* value){

}


int VMwrite(uint64_t virtualAddress, word_t value){

}


