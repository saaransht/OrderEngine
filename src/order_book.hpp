#pragma once

#include <map>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>
#include "memory_pool.hpp"

namespace OrderEngine {

enum class OrderSide { BUY, SELL };

struct Order {
    uint64_t id;
    OrderSide side;
    double price;
    uint32_t quantity;
    std::chrono::high_resolution_clock::time_point timestamp;
    
    // Default constructor for memory pool
    Order() : id(0), side(OrderSide::BUY), price(0.0), quantity(0), 
              timestamp(std::chrono::high_resolution_clock::now()) {}
    
    Order(uint64_t id, OrderSide side, double price, uint32_t quantity)
        : id(id), side(side), price(price), quantity(quantity), 
          timestamp(std::chrono::high_resolution_clock::now()) {}
};

struct Trade {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    uint32_t quantity;
    std::chrono::high_resolution_clock::time_point timestamp;
};

class OrderBook {
public:
    using TradeCallback = std::function<void(const Trade&)>;
    
    OrderBook();
    ~OrderBook();
    
    void addOrder(std::unique_ptr<Order> order);
    void setTradeCallback(TradeCallback callback);
    
    size_t getBuyOrdersCount() const;
    size_t getSellOrdersCount() const;
    
private:
    // Price-time priority maps
    std::multimap<double, std::unique_ptr<Order>, std::greater<double>> buy_orders_;  // Highest price first
    std::multimap<double, std::unique_ptr<Order>> sell_orders_;  // Lowest price first
    
    TradeCallback trade_callback_;
    MemoryPool<Order> order_pool_;
    std::atomic<uint64_t> next_trade_id_{1};
    
    void matchOrders();
    void executeTrade(Order& buy_order, Order& sell_order, uint32_t quantity);
};

} // namespace OrderEngine
