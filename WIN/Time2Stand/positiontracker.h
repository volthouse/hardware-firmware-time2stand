#ifndef POSITIONTRACKER_H
#define POSITIONTRACKER_H

#include <atomic>
#include <chrono>
#include <chrono>
#include <iostream>
#include <thread>

#include "observer.h"

enum class PositionState {
    Sit,
    Stand
};

struct PositionData {
    std::chrono::seconds sit;
    std::chrono::seconds stand;
    std::chrono::seconds standTotal;
    PositionState position;
};

class PositionTracker
{
public:
    static PositionTracker& Instance() {
        static PositionTracker instance;
        return instance;
    }

    void Start(std::chrono::seconds standTotal = std::chrono::seconds(0));
    void Stop();
    void Reset();

    const struct PositionData Position();

    Observer<>& UpdatePosition();

private:
    PositionTracker();
    ~PositionTracker();

    void update();
    void updateLoop();

private:
    std::chrono::seconds m_sit;
    std::chrono::seconds m_stand;
    std::chrono::seconds m_standTotal;
    PositionState m_position;
    std::chrono::steady_clock::time_point m_lastMeasureTimePoint;
    std::thread m_thread;
    std::atomic<bool> m_stopThread;
    std::mutex m_mutex;
    Observer<> m_updatePostionObserver;

};

#endif // POSITIONTRACKER_H
