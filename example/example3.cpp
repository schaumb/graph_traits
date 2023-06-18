#include "common.h"

#include <utility>
#include <list>

void example3 () {
  /*        -------\
   0 -\    /        -> 9
       -> 3 -> 4-\ /
   1 -/    \      ---> 7
            -> 6   \       -> 8
   2 ------/    \   -> 5 -/
                 \       /
                  -------       */
  std::list<std::pair<std::string, std::string>> edge_list {
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
        {"6", "8"}
  };

  std::cout << "example 3: ";
  common::run_example(edge_list);
}