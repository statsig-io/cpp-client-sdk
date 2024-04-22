#pragma once

#include <string>

namespace statsig {

typedef std::string String;

inline std::string FromCompat(const std::string &input) {
  return input;
}

inline String ToCompat(const std::string &input) {
  return input;
}

}
