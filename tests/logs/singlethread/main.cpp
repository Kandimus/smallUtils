#include <thread>
#include "log.h"

int main()
{
    su::Log::instance().setDir("./");
    su::Log::instance().setFilename("test_singlethread");
    su::Log::instance().setLevel(su::Log::Level::Debug);
    su::Log::instance().setTerminal(true);

    std::thread thread1([]()
    {
        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD("A_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(11));
        }
    });

    std::thread thread2([]()
    {
        for (size_t ii = 0; ii < 100; ++ii)
        {
            LOGD("B_%i", ii);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    thread1.join();
    thread2.join();

    su::Log::instance().setFilename("test_singlethread2");

    for (size_t ii = 0; ii < 100; ++ii)
    {
        LOGD("C_%i", ii);
        std::this_thread::sleep_for(std::chrono::milliseconds(11));
    }

    su::Log::instance().setTerminal(false);
    su::Log::instance().setDir("..");

    for (size_t ii = 0; ii < 100; ++ii)
    {
        LOGD("D_%i", ii);
        std::this_thread::sleep_for(std::chrono::milliseconds(11));
    }

    return 0;
}
