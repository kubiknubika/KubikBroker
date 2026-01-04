#include "LockFreeQueue.hpp"
#include "Order.hpp"
#include <thread>
#include <iostream>
#include <atomic>
#include <chrono>
#include <random>
#include <cmath>

void engine_loop(LockFreeQueue<Order>& queue, std::atomic<bool>& running);

int main() {
    LockFreeQueue<Order> queue(4 * 1024 * 1024); 
    std::atomic<bool> running{true};

    std::thread consumer([&]() {
        engine_loop(queue, running);
    });

    std::cout << "🚀 HFT Market: Stable Edition..." << std::endl;

    std::mt19937 rng(std::random_device{}());
    
    double fair_price = 500.0; // Базовая цена
    double target_price = 500.0; // Куда рынок "хочет" прийти
    double momentum = 0.0; 
    
    std::normal_distribution<> qty_dist(5.0, 3.0); 

    uint64_t id = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        id++;

        // --- 1. ЛОГИКА РЫНКА (Mean Reversion) ---
        
        // Рынок иногда меняет цель (новости)
        if (id % 5000 == 0) {
            std::normal_distribution<> new_target_dist(0.0, 20.0);
            target_price = 500.0 + new_target_dist(rng); // Цель гуляет вокруг 500
        }

        // Сила притяжения к цели
        double pull = (target_price - fair_price) * 0.001;
        
        // Случайный шум
        std::normal_distribution<> shock_dist(0.0, 0.5);
        double shock = shock_dist(rng);

        // Обновляем моментум (с затуханием)
        momentum = (momentum * 0.95) + shock + pull;

        // Двигаем цену
        fair_price += momentum;
        if (fair_price < 100) fair_price = 100;

        // --- 2. ГЕНЕРАЦИЯ ЗАЯВКИ ---
        double buy_prob = 0.5 + (momentum * 0.05); 
        if (buy_prob > 0.8) buy_prob = 0.8;
        if (buy_prob < 0.2) buy_prob = 0.2;

        Side side = (std::bernoulli_distribution(buy_prob)(rng)) ? Side::Buy : Side::Sell;
        
        std::normal_distribution<> noise_dist(0, 5.0); 
        double noise = std::abs(noise_dist(rng));

        uint64_t price;
        if (side == Side::Buy) {
            price = (uint64_t)(fair_price - 1.0 - noise);
        } else {
            price = (uint64_t)(fair_price + 1.0 + noise);
        }
        if (price < 1) price = 1;

        uint64_t qty = (uint64_t)std::round(std::abs(qty_dist(rng)));
        if (qty < 1) qty = 1;

        while (!queue.push(Order{ id, price, qty, side })) {
            std::this_thread::yield();
        }

        std::this_thread::sleep_for(std::chrono::microseconds(50));

        if (id % 10000 == 0) {
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = current_time - start_time;
            if (diff.count() > 5.0) {
                 std::cout << "[Market] Price: " << (int)fair_price << " | Target: " << (int)target_price << std::endl;
                 start_time = current_time;
            }
        }
    }

    running = false;
    consumer.join();
    return 0;
}