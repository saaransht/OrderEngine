#include <iostream>
#include <cassert>
#include <memory>
#include "../src/order_book.hpp"
#include "../src/parser.hpp"

using namespace OrderEngine;

void testBasicOrderMatching() {
    OrderBook order_book;
    bool trade_executed = false;
    
    order_book.setTradeCallback([&trade_executed](const Trade& trade) {
        trade_executed = true;
        assert(trade.price == 100.0);
        assert(trade.quantity == 5);
    });
    
    // Add buy order
    auto buy_order = std::make_unique<Order>(1, OrderSide::BUY, 100.0, 10);
    order_book.addOrder(std::move(buy_order));
    
    // Add matching sell order
    auto sell_order = std::make_unique<Order>(2, OrderSide::SELL, 100.0, 5);
    order_book.addOrder(std::move(sell_order));
    
    assert(trade_executed);
    assert(order_book.getBuyOrdersCount() == 1);  // Partial fill
    assert(order_book.getSellOrdersCount() == 0);
    
    std::cout << "testBasicOrderMatching: PASSED\n";
}

void testOrderParser() {
    OrderParser parser;
    
    std::string json = R"({"side":"buy","price":100.50,"quantity":10})";
    auto order = parser.parseOrder(json);
    
    assert(order.has_value());
    assert((*order)->side == OrderSide::BUY);
    assert((*order)->price == 100.50);
    assert((*order)->quantity == 10);
    
    std::cout << "testOrderParser: PASSED\n";
}

void testPriceTimePriority() {
    OrderBook order_book;
    int trade_count = 0;
    
    order_book.setTradeCallback([&trade_count](const Trade& trade) {
        trade_count++;
        // First trade should be at price 99.0 (best sell price)
        if (trade_count == 1) {
            assert(trade.price == 99.0);
        }
    });
    
    // Add buy order at 100
    auto buy_order = std::make_unique<Order>(1, OrderSide::BUY, 100.0, 10);
    order_book.addOrder(std::move(buy_order));
    
    // Add sell orders at different prices
    auto sell_order1 = std::make_unique<Order>(2, OrderSide::SELL, 101.0, 5);
    auto sell_order2 = std::make_unique<Order>(3, OrderSide::SELL, 99.0, 5);
    
    order_book.addOrder(std::move(sell_order1));
    order_book.addOrder(std::move(sell_order2));  // This should match first
    
    assert(trade_count == 1);
    
    std::cout << "testPriceTimePriority: PASSED\n";
}

int main() {
    std::cout << "Running Order Book Engine Tests...\n\n";
    
    testBasicOrderMatching();
    testOrderParser();
    testPriceTimePriority();
    
    std::cout << "\nAll tests passed!\n";
    return 0;
}
