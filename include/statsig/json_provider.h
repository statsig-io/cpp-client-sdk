#pragma once

#include <string>

namespace statsig {

class JsonProvider {
 public:
  virtual void Attach(
      std::string &sdk_key
  ) = 0;

};

}

