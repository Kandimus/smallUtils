#include <thread>
#include "log.h"

int main()
{
    std::thread thread1([]()
    {
        su::Log log("thread A", "test_multithread", "./", su::Log::Level::Debug);

        log.setTerminal(false);

        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD(log, "A_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(11));
        }

        log.setFilename("test_multithread2");

        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD(log, "A_2_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(11));
        }
    });

    std::thread thread2([]()
    {
        su::Log log("thread B", "test_multithread", "./", su::Log::Level::Debug);

        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD(log, "B_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        log.setDir("..");

        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD(log, "B_2_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(11));
        }
    });

    std::thread thread3([]()
    {
        su::Log log("", "test_multithread", "./", su::Log::Level::Debug);

        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD(log, "C_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(12));
        }

        log.setFilename("test_multithread2");

        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD(log, "C_2_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(11));
        }
    });

    thread1.join();
    thread2.join();
    thread3.join();

    return 0;
}
