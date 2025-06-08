#pragma once

#include <string>
#include <memory>
#include <optional>
#include <atomic>
#include "order_book.hpp"

namespace OrderEngine {

class OrderParser {
public:
    OrderParser();
    ~OrderParser();
    
    std::optional<std::unique_ptr<Order>> parseOrder(const std::string& json_str);
    
private:
    std::atomic<uint64_t> next_order_id_{1};
    
    OrderSide parseOrderSide(const std::string& side_str);
};

} // namespace OrderEngine
