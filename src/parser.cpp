#include "parser.hpp"
#include <sstream>
#include <iostream>

namespace OrderEngine {

OrderParser::OrderParser() = default;
OrderParser::~OrderParser() = default;

std::optional<std::unique_ptr<Order>> OrderParser::parseOrder(const std::string& json_str) {
    try {
        // Simple JSON parsing for demo - in production use nlohmann/json
        std::string side_str, price_str, quantity_str;
        
        // Basic JSON parsing (simplified for demo)
        size_t side_pos = json_str.find("\"side\":");
        size_t price_pos = json_str.find("\"price\":");
        size_t quantity_pos = json_str.find("\"quantity\":");
        
        if (side_pos == std::string::npos || price_pos == std::string::npos || 
            quantity_pos == std::string::npos) {
            return std::nullopt;
        }
        
        // Extract values (simplified parsing)
        size_t side_start = json_str.find("\"", side_pos + 7) + 1;
        size_t side_end = json_str.find("\"", side_start);
        side_str = json_str.substr(side_start, side_end - side_start);
        
        size_t price_start = json_str.find(":", price_pos + 8) + 1;
        size_t price_end = json_str.find_first_of(",}", price_start);
        price_str = json_str.substr(price_start, price_end - price_start);
        
        size_t quantity_start = json_str.find(":", quantity_pos + 11) + 1;
        size_t quantity_end = json_str.find_first_of(",}", quantity_start);
        quantity_str = json_str.substr(quantity_start, quantity_end - quantity_start);
        
        OrderSide side = parseOrderSide(side_str);
        double price = std::stod(price_str);
        uint32_t quantity = std::stoul(quantity_str);
        
        return std::make_unique<Order>(next_order_id_++, side, price, quantity);
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing order: " << e.what() << std::endl;
        return std::nullopt;
    }
}

OrderSide OrderParser::parseOrderSide(const std::string& side_str) {
    if (side_str == "buy" || side_str == "BUY") {
        return OrderSide::BUY;
    } else if (side_str == "sell" || side_str == "SELL") {
        return OrderSide::SELL;
    }
    throw std::invalid_argument("Invalid order side: " + side_str);
}

} // namespace OrderEngine
