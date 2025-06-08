#include "order_book.hpp"
#include <algorithm>
#include <iostream>

namespace OrderEngine {

OrderBook::OrderBook() = default;
OrderBook::~OrderBook() {
    stop();
}

void OrderBook::start() {
    running_ = true;
    matching_thread_ = std::thread(&OrderBook::matchingThreadFunc, this);
}

void OrderBook::stop() {
    running_ = false;
    queue_cv_.notify_all();
    if (matching_thread_.joinable()) {
        matching_thread_.join();
    }
}

void OrderBook::submitOrder(std::unique_ptr<Order> order) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    order_queue_.push(std::move(order));
    queue_cv_.notify_one();
}

void OrderBook::setTradeCallback(TradeCallback callback) {
    trade_callback_ = std::move(callback);
}

void OrderBook::matchingThreadFunc() {
    while (running_ || !order_queue_.empty()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !order_queue_.empty() || !running_; });
        
        while (!order_queue_.empty()) {
            auto order = std::move(order_queue_.front());
            order_queue_.pop();
            lock.unlock();
            
            processOrder(std::move(order));
            
            lock.lock();
        }
    }
}

void OrderBook::processOrder(std::unique_ptr<Order> order) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    if (order->side == OrderSide::BUY) {
        buy_orders_.emplace(order->price, std::move(order));
    } else {
        sell_orders_.emplace(order->price, std::move(order));
    }
    
    matchOrders();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time).count();
    
    latency_stats_.recordLatency(latency_ns);
}

void OrderBook::matchOrders() {
    while (!buy_orders_.empty() && !sell_orders_.empty()) {
        auto buy_it = buy_orders_.begin();  // Highest price
        auto sell_it = sell_orders_.begin();  // Lowest price
        
        if (buy_it->first < sell_it->first) {
            break;  // No match possible
        }
        
        auto& buy_order = *buy_it->second;
        auto& sell_order = *sell_it->second;
        
        uint32_t trade_quantity = std::min(buy_order.quantity, sell_order.quantity);
        executeTrade(buy_order, sell_order, trade_quantity);
        
        buy_order.quantity -= trade_quantity;
        sell_order.quantity -= trade_quantity;
        
        if (buy_order.quantity == 0) {
            buy_orders_.erase(buy_it);
        }
        if (sell_order.quantity == 0) {
            sell_orders_.erase(sell_it);
        }
    }
}

void OrderBook::executeTrade(Order& buy_order, Order& sell_order, uint32_t quantity) {
    if (trade_callback_) {
        Trade trade{
            buy_order.id,
            sell_order.id,
            sell_order.price,  // Trade at sell order price (price-time priority)
            quantity,
            std::chrono::high_resolution_clock::now()
        };
        trade_callback_(trade);
    }
}

size_t OrderBook::getBuyOrdersCount() const {
    return buy_orders_.size();
}

size_t OrderBook::getSellOrdersCount() const {
    return sell_orders_.size();
}

} // namespace OrderEngine
