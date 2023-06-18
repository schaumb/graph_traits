#include "common.h"

#include <bitset>

void example2 () {
  /*        -------\
   0 -\    /        -> 9
       -> 3 -> 4-\ /
   1 -/    \      ---> 7
            -> 6   \       -> 8
   2 ------/    \   -> 5 -/
                 \       /
                  -------       */

  char d[] =//0123456789
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

  std::bitset<sizeof(d)/sizeof(d[0]) - 1> adjacency_matrix{d};

  std::cout << "example 2: ";
  common::run_example(adjacency_matrix);
}