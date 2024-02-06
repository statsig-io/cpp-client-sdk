#include <iostream>
#include "../include/statsig_client.h"

int main() {
  auto client = new statsig::StatsigClient("", nullptr, nullptr);

  std::cout << "Hello, World!" << std::endl;
  return 0;
}
