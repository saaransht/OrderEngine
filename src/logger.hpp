#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "order_book.hpp"

namespace OrderEngine {

class TradeLogger {
public:
    TradeLogger(const std::string& filename);
    ~TradeLogger();
    
    void logTrade(const Trade& trade);
    void start();
    void stop();
    
private:
    std::ofstream file_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::queue<Trade> trade_queue_;
    std::thread logging_thread_;
    std::atomic<bool> running_{false};
    
    void loggerThreadFunc();
    std::string formatTrade(const Trade& trade);
};

} // namespace OrderEngine
