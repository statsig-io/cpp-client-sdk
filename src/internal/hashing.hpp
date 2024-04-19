#pragma once

#include <string>

namespace statsig::hashing {

std::string DJB2(const std::string &value) {
  unsigned long hash = 0;
  for (char character : value) {
    hash = (hash << 5) - hash + static_cast<unsigned long>(character);
    // hash &= hash;
  }

  hash &= 0xffffffff;
  return std::to_string(hash);
}

}
