# 📈 KubikBroker: HFT Matching Engine

**Ultra-Low Latency Limit Order Book (LOB) written in C++20.**

> **Core:** A lock-free matching engine capable of processing **>1,000,000 orders/sec** on a single core.
> **UI:** Real-time web dashboard visualizing Market Depth and PnL.

![C++](https://img.shields.io/badge/C++-20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Performance](https://img.shields.io/badge/Speed-1M%2B%20TPS-brightgreen?style=for-the-badge)
![Visualization](https://img.shields.io/badge/UI-Real--Time-orange?style=for-the-badge)

## ⚡ Architecture

1.  **Market Generator (Thread 1):** Simulates retail flow + institutional logic (FOMO/Panic algorithms based on price momentum).
2.  **Matching Engine (Thread 2):**
    *   **SPSC Ring Buffer:** Lock-free communication.
    *   **Binary Heaps:** `std::push_heap` for O(log N) order management.
    *   **Zero-Copy:** No heap allocations in the hot path.
3.  **Visualization (Web):** Atomic JSON snapshots -> HTML5 Canvas (Chart.js) with resampling logic.

## 🚀 Quick Start

1.  **Build:**
    mkdir build && cd build
    cmake .. && make

2.  **Run Server (Terminal 1):**
    python3 -m http.server 8080

3.  **Run Engine (Terminal 2):**
    ./KubikBroker

4.  **View:** http://localhost:8080/visualizer.html

---
**Developed by KubikNubika** — *Zero to Hero Challenge (Project #6)*