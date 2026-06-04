#pragma once
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

/**
 * @brief Простой таймер для измерения времени выполнения.
 */
class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime;

public:
    /**
     * @brief Запускает таймер.
     */
    Timer() {
        reset();
    }

    /**
     * @brief Сбрасывает таймер.
     */
    void reset() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Возвращает прошедшее время в миллисекундах.
     *
     * @return Время в миллисекундах.
     */
    double elapsedMilliseconds() const {
        auto endTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed = endTime - startTime;

        return elapsed.count();
    }
};

#endif