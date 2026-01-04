#pragma once
#include <vector>
#include <atomic>
#include <optional>

template<typename T>
class LockFreeQueue {
private:
    std::vector<T> buffer;
    size_t capacity;
    
    // alignas(64) предотвращает False Sharing (когда ядра процессора дерутся за одну кэш-линию)
    alignas(64) std::atomic<size_t> head {0}; // Пишет Producer
    alignas(64) std::atomic<size_t> tail {0}; // Читает Consumer

public:
    explicit LockFreeQueue(size_t size) : buffer(size), capacity(size) {}

    bool push(const T& item) {
        size_t currentHead = head.load(std::memory_order_relaxed);
        size_t nextHead = (currentHead + 1) % capacity;

        if (nextHead == tail.load(std::memory_order_acquire)) {
            return false; // Очередь полна
        }

        buffer[currentHead] = item;
        head.store(nextHead, std::memory_order_release); // "Публикуем" изменение
        return true;
    }

    bool pop(T& item) {
        size_t currentTail = tail.load(std::memory_order_relaxed);

        if (currentTail == head.load(std::memory_order_acquire)) {
            return false; // Очередь пуста
        }

        item = buffer[currentTail];
        tail.store((currentTail + 1) % capacity, std::memory_order_release);
        return true;
    }
};