#include "common.h"
#include <bitset>
#include <utility>
#include <list>

#define EXAMPLE

int main () {
  /*
            -------\
   0 -\    /        -> 9
       -> 3 -> 4-\ /
   1 -/    \      ---> 7
            -> 6   \       -> 8
   2 ------/    \   -> 5 -/
                 \       /
                  -------
 */

#if (EXAMPLE+0) == 1  // szomszédsági lista
  std::vector<std::vector<int>> graph {
        /* 0 -> */ {3},
        /* 1 -> */ {3},
        /* 2 -> */ {6},
        /* 3 -> */ {4, 6, 9},
        /* 4 -> */ {5, 7, 9},
        /* 5 -> */ {8},
        /* 6 -> */ {8},
        /* 7 -> */ {},
        /* 8 -> */ {},
        /* 9 -> */ {},
  };

  std::cout << "example 1: ";
#elif (EXAMPLE+0) == 2  // szomszédsági mátrix
    char d[] =    //0123456789
        /* 0 -> */ "0001000000"
        /* 1 -> */ "0001000000"
        /* 2 -> */ "0000001000"
        /* 3 -> */ "0000101001"
        /* 4 -> */ "0000010101"
        /* 5 -> */ "0000000010"
        /* 6 -> */ "0000000010"
        /* 7 -> */ "0000000000"
        /* 8 -> */ "0000000000"
        /* 9 -> */ "0000000000";
    std::reverse(std::begin(d), std::prev(std::end(d)));

    std::bitset<sizeof(d)/sizeof(d[0]) - 1> graph{d};

    std::cout << "example 2: ";
#elif (EXAMPLE+0) == 3  // éllista
    std::pair<std::string, std::string> graph[] {
          {"0", "3"},
          {"1", "3"},
          {"2", "6"},
          {"3", "4"},
          {"3", "6"},
          {"3", "9"},
          {"4", "5"},
          {"4", "7"},
          {"4", "9"},
          {"5", "8"},
          {"6", "8"},
/*
          {"9", "0"},
          {"8", "1"},
          {"7", "2"}
 */
    };

    std::cout << "example 3: ";
#else
#error Define EXAMPLE macro to 1..3
#endif
  common::run_example(graph);
}