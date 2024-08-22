
#include "thread_class.h"

namespace su
{

ThreadClass::ThreadClass()
    : m_delay(100), m_status(Status::Undef), m_command(Command::None)
{
}

ThreadClass::~ThreadClass()
{
    close();
}

std::thread* ThreadClass::run(size_t delay)
{
    close();

    m_delay.store(delay);

    m_thread = new std::thread(ThreadClass::ThreadFunc, this); //TODO
    m_status.store(Status::Running);
    return m_thread;
}

void ThreadClass::close()
{
    if (!m_thread)
    {
        m_status.store(Status::Closed);
        return;
    }

    Status status = m_status.load();
    if (status == Status::Paused || status == Status::Running)
    {
        m_command.store(Command::Finish);
        m_thread->join();
    }

    delete m_thread;
    m_thread = nullptr;

    m_status.store(Status::Closed);
}

void ThreadClass::pause()
{
    if (m_status.load() == Status::Running)
    {
        m_command.store(Command::Pause);
    }
}

void ThreadClass::proccesing()
{
    while (1)
    {
        Command command = m_command.load();
        size_t delay = m_delay.load();
        bool isPaused = false;

        m_command.store(Command::None);

        switch (command)
        {
            case Command::Finish: m_status.store(Status::Finished); doFinished(); return;
            case Command::Pause:  m_status.store(Status::Paused); isPaused = true; break;
            default: break;
        }

        if (isPaused)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay < 16 ? 16 : delay));
            continue;
        }

        m_status.store(Status::Running);

        if (delay)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }

        doWork();
    }
}

void ThreadClass::ThreadFunc(ThreadClass* tc)
{
    if (!tc)
    {
        return;
    }

    tc->proccesing();
}

} // namespace su
