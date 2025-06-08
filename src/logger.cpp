#include "logger.hpp"
#include <iomanip>
#include <sstream>

namespace OrderEngine {

TradeLogger::TradeLogger(const std::string& filename) : file_(filename) {
    if (file_.is_open()) {
        file_ << "timestamp,buy_order_id,sell_order_id,price,quantity\n";
        file_.flush();
    }
}

TradeLogger::~TradeLogger() {
    stop();
}

void TradeLogger::start() {
    running_ = true;
    logging_thread_ = std::thread(&TradeLogger::loggerThreadFunc, this);
}

void TradeLogger::stop() {
    running_ = false;
    queue_cv_.notify_all();
    if (logging_thread_.joinable()) {
        logging_thread_.join();
    }
    if (file_.is_open()) {
        file_.close();
    }
}

void TradeLogger::logTrade(const Trade& trade) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    trade_queue_.push(trade);
    queue_cv_.notify_one();
}

void TradeLogger::loggerThreadFunc() {
    while (running_ || !trade_queue_.empty()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !trade_queue_.empty() || !running_; });
        
        while (!trade_queue_.empty()) {
            Trade trade = trade_queue_.front();
            trade_queue_.pop();
            lock.unlock();
            
            if (file_.is_open()) {
                file_ << formatTrade(trade) << "\n";
                file_.flush();
            }
            
            lock.lock();
        }
    }
}

std::string TradeLogger::formatTrade(const Trade& trade) {
    auto time_point = trade.timestamp;
    auto time_t = std::chrono::system_clock::to_time_t(
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            time_point - std::chrono::high_resolution_clock::now() + 
            std::chrono::system_clock::now()));
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "," << trade.buy_order_id;
    ss << "," << trade.sell_order_id;
    ss << "," << std::fixed << std::setprecision(2) << trade.price;
    ss << "," << trade.quantity;
    
    return ss.str();
}

} // namespace OrderEngine
