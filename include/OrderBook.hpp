#pragma once
#include "Order.hpp"
#include <map>
#include <list>
#include <vector>
#include <iostream>

class OrderBook {
private:
    // Стакан: Цена -> Список ордеров по этой цене
    // std::map автоматически сортирует ключи (цены)
    
    // Asks: Продавцы. Сортировка: Низкая цена -> Высокая (std::less - default)
    std::map<Price, std::list<Order>> asks;

    // Bids: Покупатели. Сортировка: Высокая цена -> Низкая (std::greater)
    std::map<Price, std::list<Order>, std::greater<Price>> bids;

public:
    // Добавить ордер и попытаться его исполнить
    void addOrder(Order order);

    // Матчинг: Самая сложная логика
    void match();

    // Вывод стакана в консоль
    void printBook() const;
};