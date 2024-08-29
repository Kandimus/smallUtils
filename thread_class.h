
#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

namespace su
{

class ThreadClass
{
public:
    enum class Status
    {
        Undef = 0,
        Finished,
        Running,
        Paused,
        Closed,
    };

    enum class Command
    {
        None = 0,
        Pause,
        Finish,
        Restore,
    };

    ThreadClass();
    virtual ~ThreadClass();

    Status status() const { return m_status.load(); };
    std::thread* thread() const { return m_thread; }
    bool isPaused() const { return m_status.load() == Status::Paused; }
    bool isWork() const { auto status = m_status.load(); return status == Status::Running || status == Status::Paused; }

    virtual void join() const { if (m_thread) m_thread->join(); }
    virtual void finish();
    virtual void restore();
    virtual void close();
    virtual void pause();

    virtual std::thread* run(size_t delay);

    virtual void doWork() = 0;
    virtual void doFinished() = 0;

protected:
    Command popCommand();
    void proccesing();

private:
    std::thread* m_thread = nullptr;
    std::atomic<size_t> m_delay;
    std::atomic<Status> m_status;
    std::atomic<Command> m_command;

    static void ThreadFunc(ThreadClass*);
};

} // namespace su
