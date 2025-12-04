#pragma once
#include <cstddef>

namespace mfast {

inline std::size_t align(std::size_t n, std::size_t x) {
  const std::size_t y = x - 1;
  return (n + y) & ~y;
}

}
