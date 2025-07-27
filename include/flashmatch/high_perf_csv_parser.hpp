#pragma once

#include "../flashmatch/order_book.hpp"
#include <array>
#include <charconv> // for std::from_chars
#include <cstdint>
#include <string_view>

// struct Order {
//   uint64_t order_id;
//   std::string_view side;
//   double price;
//   uint32_t quantity;
//   std::string_view type;
// };

// Parse a single CSV line into an Order struct
inline bool parse_order_line(std::string_view line, Order &out) {
  size_t start = 0, end = 0;
  std::array<std::string_view, 5> tokens;

  for (int i = 0; i < 4; ++i) {
    end = line.find(',', start);
    if (end == std::string_view::npos)
      return false;
    tokens[i] = line.substr(start, end - start);
    start = end + 1;
  }
  tokens[4] = line.substr(start); // last token

  // Parse with no allocations
  std::from_chars(tokens[0].data(), tokens[0].data() + tokens[0].size(),
                  out.order_id);
  out.side = tokens[1];
  std::from_chars(tokens[2].data(), tokens[2].data() + tokens[2].size(),
                  out.price);
  std::from_chars(tokens[3].data(), tokens[3].data() + tokens[3].size(),
                  out.quantity);
  out.type = tokens[4];

  return true;
}
