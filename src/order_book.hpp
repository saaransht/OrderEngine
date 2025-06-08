#pragma once

#include <map>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
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

struct LatencyStats {
    std::atomic<uint64_t> total_orders{0};
    std::atomic<uint64_t> total_latency_ns{0};
    std::atomic<uint64_t> min_latency_ns{UINT64_MAX};
    std::atomic<uint64_t> max_latency_ns{0};
    
    void recordLatency(uint64_t latency_ns) {
        total_orders++;
        total_latency_ns += latency_ns;
        
        uint64_t current_min = min_latency_ns.load();
        while (latency_ns < current_min && 
               !min_latency_ns.compare_exchange_weak(current_min, latency_ns));
        
        uint64_t current_max = max_latency_ns.load();
        while (latency_ns > current_max && 
               !max_latency_ns.compare_exchange_weak(current_max, latency_ns));
    }
    
    double getAverageLatencyUs() const {
        uint64_t orders = total_orders.load();
        return orders > 0 ? (total_latency_ns.load() / 1000.0) / orders : 0.0;
    }
    
    double getMinLatencyUs() const {
        uint64_t min_ns = min_latency_ns.load();
        return min_ns != UINT64_MAX ? min_ns / 1000.0 : 0.0;
    }
    
    double getMaxLatencyUs() const {
        return max_latency_ns.load() / 1000.0;
    }
};

class OrderBook {
public:
    using TradeCallback = std::function<void(const Trade&)>;
    
    OrderBook();
    ~OrderBook();
    
    void start();
    void stop();
    void submitOrder(std::unique_ptr<Order> order);
    void setTradeCallback(TradeCallback callback);
    
    size_t getBuyOrdersCount() const;
    size_t getSellOrdersCount() const;
    const LatencyStats& getLatencyStats() const { return latency_stats_; }
    
private:
    // Price-time priority maps
    std::multimap<double, std::unique_ptr<Order>, std::greater<double>> buy_orders_;  // Highest price first
    std::multimap<double, std::unique_ptr<Order>> sell_orders_;  // Lowest price first
    
    // Thread-safe order queue
    std::queue<std::unique_ptr<Order>> order_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    // Threading components
    std::thread matching_thread_;
    std::atomic<bool> running_{false};
    
    TradeCallback trade_callback_;
    MemoryPool<Order> order_pool_;
    std::atomic<uint64_t> next_trade_id_{1};
    LatencyStats latency_stats_;
    
    // Thread functions
    void matchingThreadFunc();
    void processOrder(std::unique_ptr<Order> order);
    void matchOrders();
    void executeTrade(Order& buy_order, Order& sell_order, uint32_t quantity);
};

} // namespace OrderEngine
