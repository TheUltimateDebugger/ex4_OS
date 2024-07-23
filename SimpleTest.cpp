#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <cstdio>
#include <iostream>
#include <cassert>

int main(int argc, char **argv) {
    VMinitialize();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        printf("writing to %llu\n", (long long int) i);
        VMwrite(5 * i * PAGE_SIZE, i);
//        printRam();
//        printf("-----------------------\n");

    }
    printRam();
    for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
        word_t value;
        VMread(5 * i * PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
        assert(uint64_t(value) == i);
    }
    printf("success\n");

    return 0;
}

int main2(int argc, char **argv)
{
    VMinitialize();
    printRam();
    VMwrite(13, 3);
    printRam();
    word_t val1;
    PMread(9, &val1);
    printRam();
    std::cout << "should be 3 val1: " << val1 << std::endl;
    word_t val2;
    VMread(13, &val2);
    printRam();
    std::cout << "should be 3 val2: " << val2 << std::endl;

    word_t val3;
    VMread(6, &val3);
    std::cout << "should be <>> val3: " << val3 << std::endl;
    printRam();


    std::cout << "____________________" << std::endl;
    word_t val4;
    VMread(31, &val4);
    std::cout << "should be <>> val4: " << val4 << std::endl;
    printRam();
    return 0;
}