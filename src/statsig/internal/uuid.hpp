#pragma once

#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>

namespace statsig::internal {

class UUID {
 public:
  static std::string v4() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 36; ++i) {
      if (i == 8 || i == 13 || i == 18 || i == 23) {
        ss << "-";
      } else if (i == 14) {
        ss << "4";
      } else if (i == 19) {
        ss << dis(gen) % 4 + 8; // For generating 8, 9, A, or B
      } else {
        ss << std::hex << dis(gen);
      }
    }
    return ss.str();
  }
};

}