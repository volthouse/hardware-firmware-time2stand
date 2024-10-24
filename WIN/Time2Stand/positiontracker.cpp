#include "positiontracker.h"
#include "appsettings.h"
#include "device.h"

PositionTracker::PositionTracker()
    : m_sit(0), m_stand(0), m_standTotal(0),
      m_lastMeasureTimePoint(),
      m_stopThread(false)
{
}

PositionTracker::~PositionTracker()
{
    Stop();
}

void PositionTracker::Start(std::chrono::seconds standTotal)
{
    m_standTotal = standTotal;
    m_lastMeasureTimePoint = std::chrono::steady_clock::now();
    m_thread = std::thread(&PositionTracker::updateLoop, this);
}

void PositionTracker::Reset()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sit = std::chrono::seconds(0);
    m_stand = std::chrono::seconds(0);
    m_standTotal = std::chrono::seconds(0);
    m_updatePostionObserver.Notify();
}

void PositionTracker::Stop()
{
    m_stopThread = true;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

const PositionData PositionTracker::Position()
{
    return PositionData {
        .sit = m_sit,
        .stand = m_stand,
        .standTotal = m_standTotal,
        .position = m_position
    };
}

Observer<> &PositionTracker::UpdatePosition()
{
    return m_updatePostionObserver;;
}

void PositionTracker::update()
{
    DeviceData deviceData;

    auto current_time = std::chrono::steady_clock::now();
    auto duration     = std::chrono::duration_cast<std::chrono::seconds>(
        current_time - m_lastMeasureTimePoint);

    auto err = Device::Instance().ReceiveData(deviceData);
    if(err) {
        return;
    }

    { /* Critical section begin: update position data */
        std::lock_guard<std::mutex> lock(m_mutex);

        if (deviceData.distance < 80.0) {
            m_sit += duration;
            m_stand = std::chrono::seconds(0);
            m_position = PositionState::Sit;
        } else {
            m_stand += duration;
            m_standTotal += duration;
            m_sit = std::chrono::seconds(0);
            m_position = PositionState::Stand;
        }
    } /* Critical section end */

    m_updatePostionObserver.Notify();

    AppSettings::Instance().setStandTotalDuration(m_standTotal.count());

    m_lastMeasureTimePoint = std::chrono::steady_clock::now();
}

void PositionTracker::updateLoop() {
    while (!m_stopThread) {
        update();
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
