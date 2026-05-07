#include "fifo.h"
#include <print>

int main() {
  constexpr FIFO<int, 2> fifo{1, 2};

  std::println("val={}", fifo.peek());

  return 0;
}
