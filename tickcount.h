
#pragma once

#include <chrono>

namespace su
{

class TickCount
{
public:
    TickCount() { m_tick = getTickCount(); }
    virtual ~TickCount() = default;

    size_t count() { return getCount(false); }
    void  start(size_t msec) { restart(); m_setting = msec; }
    bool  isFinished() { return m_isStart && count() >= m_setting; }
    void  stop() { m_isStart = false; }
    bool  isStarted() const { return m_isStart; }
    void  restart() { m_isStart = true; m_tick = getTickCount(); }

    static size_t   getTickCount();
    static size_t unixTime();

protected:
    size_t m_tick = 0;
    size_t m_setting = 0;
    bool m_isStart = false;

    size_t getCount(bool reset);
};

}
