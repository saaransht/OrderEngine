#include <iostream>
#include <string>
#include <chrono>
#include "order_book.hpp"
#include "parser.hpp"
#include "logger.hpp"

using namespace OrderEngine;

int main() {
    std::cout << "Ultra-Low Latency Order Book Engine Starting...\n";
    
    // Initialize components
    OrderBook order_book;
    OrderParser parser;
    TradeLogger logger("trades.csv");
    
    // Set up trade callback
    order_book.setTradeCallback([&logger](const Trade& trade) {
        logger.logTrade(trade);
        std::cout << "TRADE: Buy Order " << trade.buy_order_id 
                  << " matched with Sell Order " << trade.sell_order_id
                  << " at price " << trade.price 
                  << " for quantity " << trade.quantity << "\n";
    });
    
    logger.start();
    
    std::cout << "Engine ready. Enter JSON orders (or 'quit' to exit):\n";
    std::cout << "Example: {\"side\":\"buy\",\"price\":100.50,\"quantity\":10}\n";
    
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "quit" || input == "exit") {
            break;
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        auto order = parser.parseOrder(input);
        if (order) {
            order_book.addOrder(std::move(*order));
        } else {
            std::cout << "Error: Invalid order format\n";
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        
        std::cout << "Processing time: " << duration.count() << "µs | "
                  << "Buy orders: " << order_book.getBuyOrdersCount() << " | "
                  << "Sell orders: " << order_book.getSellOrdersCount() << "\n";
    }
    
    logger.stop();
    std::cout << "Order Book Engine stopped.\n";
    return 0;
}
