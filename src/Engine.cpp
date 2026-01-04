#include "LockFreeQueue.hpp"
#include "Order.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <deque>

// --- TYPES ---
struct MarketStats {
    uint64_t last_price = 500;
    uint64_t total_volume = 0;
    double simulated_profit = 0.0;
};

struct HistoryPoint {
    uint64_t price;
    double profit;
};

// --- ENGINE ---
struct OrderBook {
    std::vector<Order> bids;
    std::vector<Order> asks;
    MarketStats stats;
    std::deque<HistoryPoint> history;

    static bool compareBids(const Order& a, const Order& b) { return a.price < b.price; }
    static bool compareAsks(const Order& a, const Order& b) { return a.price > b.price; }

    void add(Order o) {
        if (o.side == Side::Buy) {
            bids.push_back(o);
            std::push_heap(bids.begin(), bids.end(), compareBids);
        } else {
            asks.push_back(o);
            std::push_heap(asks.begin(), asks.end(), compareAsks);
        }
        match();
    }

    void match() {
        while (!bids.empty() && !asks.empty()) {
            Order& bid = bids.front();
            Order& ask = asks.front();

            if (bid.price < ask.price) break;

            uint64_t qty = std::min(bid.quantity, ask.quantity);
            
            // --- ЛОГИКА ПРИБЫЛИ ДЛЯ ШОУКЕЙСА ---
            stats.last_price = ask.price;
            stats.total_volume += qty;
            
            // 1. Базовая прибыль (Комиссия)
            double commission = (double)qty * 0.05;
            
            // 2. Случайный "шум" (Рыночный риск), чтобы линия дрожала
            // Зависит от последней цифры цены (псевдо-рандом)
            double noise = ((int)ask.price % 10 - 4) * 0.02; 

            stats.simulated_profit += (commission + noise);

            bid.quantity -= qty;
            ask.quantity -= qty;

            if (bid.quantity == 0) { std::pop_heap(bids.begin(), bids.end(), compareBids); bids.pop_back(); }
            if (ask.quantity == 0) { std::pop_heap(asks.begin(), asks.end(), compareAsks); asks.pop_back(); }
        }
        cleanup();
    }

    void cleanup() {
        if (bids.size() > 5000) { bids.resize(4000); std::make_heap(bids.begin(), bids.end(), compareBids); }
        if (asks.size() > 5000) { asks.resize(4000); std::make_heap(asks.begin(), asks.end(), compareAsks); }
    }

    void snapshot() {
        history.push_back(HistoryPoint{stats.last_price, stats.simulated_profit});
        if (history.size() > 2000) history.pop_front();

        std::vector<Order> sorted_bids = bids;
        std::vector<Order> sorted_asks = asks;
        
        std::sort(sorted_bids.begin(), sorted_bids.end(), [](auto& a, auto& b){ return a.price > b.price; });
        std::sort(sorted_asks.begin(), sorted_asks.end(), [](auto& a, auto& b){ return a.price < b.price; });

        if(sorted_bids.size() > 50) sorted_bids.resize(50);
        if(sorted_asks.size() > 50) sorted_asks.resize(50);

        std::ofstream out("book_snapshot.tmp");
        out << "{ ";
        out << "\"stats\": { \"price\": " << stats.last_price << ", \"profit\": " << stats.simulated_profit << " }, ";
        out << "\"history\": [";
        for (size_t i = 0; i < history.size(); ++i) {
            out << "[" << history[i].price << "," << history[i].profit << "]" << (i == history.size()-1 ? "" : ",");
        }
        out << "], ";
        out << "\"bids\": [";
        for (size_t i = 0; i < sorted_bids.size(); ++i) {
            out << "[" << sorted_bids[i].price << ", " << sorted_bids[i].quantity << "]" << (i == sorted_bids.size()-1 ? "" : ",");
        }
        out << "], \"asks\": [";
        for (size_t i = 0; i < sorted_asks.size(); ++i) {
            out << "[" << sorted_asks[i].price << ", " << sorted_asks[i].quantity << "]" << (i == sorted_asks.size()-1 ? "" : ",");
        }
        out << "] }";
        out.close();
        std::rename("book_snapshot.tmp", "book_snapshot.json");
    }
};

void engine_loop(LockFreeQueue<Order>& queue, std::atomic<bool>& running) {
    OrderBook book;
    Order order;
    auto last_snap = std::chrono::steady_clock::now();

    while (running.load(std::memory_order_relaxed) || queue.pop(order)) {
        while (queue.pop(order)) { book.add(order); }
        
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_snap).count() > 100) { 
            book.snapshot();
            last_snap = now;
        }
        std::this_thread::yield(); 
    }
}