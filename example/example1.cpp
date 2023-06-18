#include "common.h"

#include <vector>

void example1 () {
  /*        -------\
   0 -\    /        -> 9
       -> 3 -> 4-\ /
   1 -/    \      ---> 7
            -> 6   \       -> 8
   2 ------/    \   -> 5 -/
                 \       /
                  -------       */

  std::vector<std::vector<int>> adjacency_list {
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
  common::run_example(adjacency_list);
}