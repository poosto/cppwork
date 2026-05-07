#include "../fifo.h"
#include "catch_amalgamated.hpp"
#include <atomic>
#include <ranges>
#include <thread>

struct HeapObj {
  int *x_;
  constexpr HeapObj(int x) : x_{new int{x}} {}
  constexpr HeapObj(const HeapObj &other) : x_{new int{*other.x_}} {}
  constexpr ~HeapObj() { delete x_; };
  constexpr bool operator==(const HeapObj &other) const {
    return *x_ == *other.x_;
  }
};

TEST_CASE("empty construction runtime", "[fifo]") {
  FIFO<HeapObj, 5> fifo{};
  REQUIRE(fifo.size() == 0);
}

TEST_CASE("empty construction comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3> fifo{};
    return fifo.size() == 0;
  }());
}

TEST_CASE("initializer list construction runtime", "[fifo]") {
  FIFO<HeapObj, 3> fifo{67, 68, 69};
  REQUIRE(fifo.size() == 3);
}

TEST_CASE("initializer list construction comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3> fifo{67, 68, 69};
    return fifo.size() == 3;
  }());
}

TEST_CASE("copy construction runtime", "[fifo]") {
  FIFO<HeapObj, 3> fifo{0xA, 0xB, 0xC};
  FIFO<HeapObj, 3> copied{fifo};

  REQUIRE(fifo.size() == 3);
  REQUIRE(copied.size() == 3);

  for (auto it1 = fifo.begin(), it2 = copied.begin(); it1 != fifo.end();
       ++it1, ++it2) {
    REQUIRE(*it1 == *it2);
  }
}

TEST_CASE("copy construction comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3> fifo{0xA, 0xB, 0xC};
    FIFO<HeapObj, 3> copied{fifo};

    for (auto it1 = fifo.begin(), it2 = copied.begin(); it1 != fifo.end();
         ++it1, ++it2) {
      if (*it1 != *it2) {
        return false;
      }
    }

    return fifo.size() == 3 && copied.size() == 3;
  }());
}

TEST_CASE("copy assignment runtime", "[fifo]") {
  FIFO<HeapObj, 3> fifo{0xA, 0xB, 0xC};
  FIFO<HeapObj, 3> copied{};
  copied = fifo;

  REQUIRE(fifo.size() == 3);
  REQUIRE(copied.size() == 3);

  for (auto it1 = fifo.begin(), it2 = copied.begin(); it1 != fifo.end();
       ++it1, ++it2) {
    REQUIRE(*it1 == *it2);
  }
}

TEST_CASE("copy assignment comptime", "[fifo][consteval]") {
  static_assert([]() consteval {
    FIFO<HeapObj, 3> fifo{0xA, 0xB, 0xC};
    FIFO<HeapObj, 3> copied{};
    copied = fifo;

    for (auto it1 = fifo.begin(), it2 = copied.begin(); it1 != fifo.end();
         ++it1, ++it2) {
      if (*it1 != *it2) {
        return false;
      }
    }

    return fifo.size() == 3 && copied.size() == 3;
  }());
}

TEST_CASE("FIFO push/pop", "[fifo]") {
  FIFO<int, 5> fifo;

  std::array<int, 3> values{1, 2, 3};
  for (auto val : values) {
    fifo.push(val);
  }

  int i = 0;
  for (auto val : fifo) {
    REQUIRE(val == values[i++]);
  }
  REQUIRE(i == 3);

  fifo.pop();

  i = 0;
  for (auto val : fifo) {
    REQUIRE(val == values[i++]);
  }
  REQUIRE(i == 2);

  fifo.pop();
  fifo.pop();
  REQUIRE(fifo.size() == 0);

  std::array<int, 5> values2{1, 2, 3, 6, 8};
  for (auto val : values2) {
    fifo.push(val);
  }

  i = 0;
  for (auto val : fifo) {
    REQUIRE(val == values2[i++]);
  }
  REQUIRE(i == 5);

  fifo.pop();
  fifo.pop();
  fifo.push(0);
  fifo.push(0);

  std::array<int, 5> expected{1, 2, 3, 0, 0};
  i = 0;
  for (auto val : fifo) {
    REQUIRE(val == expected[i++]);
  }
  REQUIRE(i == 5);
}

TEST_CASE("FIFO comptime push/pop", "[fifo]") {
  static_assert([]() consteval {
    FIFO<int, 5> fifo;

    fifo.push(1);
    fifo.push(2);
    fifo.push(3);
    fifo.pop();
    fifo.pop();
    fifo.push(4);
    fifo.push(5);
    fifo.push(6);
    fifo.push(7);
    fifo.pop();
    fifo.push(8);

    const std::array<int, 5> expected{1, 4, 5, 6, 8};

    for (const auto &[a, b] : std::views::zip(fifo, expected)) {
      if (a != b) {
        return false;
      }
    }

    return true;
  }());
}

// TEST_CASE("FIFO concurrent push/pop", "[fifo][threading]") {
//   FIFO<int, 1024> queue;
//   constexpr int N = 100000;
//   std::atomic<int> sum_produced{0};
//   std::atomic<int> sum_consumed{0};
//
//   std::thread producer([&]() {
//     for (int i = 0; i < N; ++i) {
//       while (queue.size() == queue.capacity())
//         ; // spin until not full
//       queue.push(i);
//       sum_produced += i;
//     }
//   });
//
//   std::thread consumer([&]() {
//     int received = 0;
//     while (received < N) {
//       if (queue.size() > 0) {
//         sum_consumed += queue.top();
//         queue.pop();
//         ++received;
//       }
//     }
//   });
//
//   producer.join();
//   consumer.join();
//
//   REQUIRE(sum_produced == sum_consumed);
// }
