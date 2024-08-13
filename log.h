#pragma once

#include <string.h>
#include <atomic>
#include <mutex>
#include <string>
#include <list>

#if defined(__GNUC__) || defined(linux)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#else

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#endif

#ifndef SU_LOGS_NOSINGLETON

#define LOG(level, format, ...)              { su::Log::instance().putFormat((level), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGE(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Error, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGW(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Warning, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGI(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Info, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGN(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Notice, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGD(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Debug, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }

#else

#define LOG(log, level, format, ...)              { log.putFormat((level), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGE(log, format, ...)                    { log.putFormat(su::Log::Level::Error, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGW(log, format, ...)                    { log.putFormat(su::Log::Level::Warning, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGI(log, format, ...)                    { log.putFormat(su::Log::Level::Info, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGN(log, format, ...)                    { log.putFormat(su::Log::Level::Notice, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGD(log, format, ...)                    { log.putFormat(su::Log::Level::Debug, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }

#endif

#define LOGP(log, level, format, ...)              { log->putFormat((level), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGPE(log, format, ...)                    { log->putFormat(su::Log::Level::Error, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGPW(log, format, ...)                    { log->putFormat(su::Log::Level::Warning, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGPI(log, format, ...)                    { log->putFormat(su::Log::Level::Info, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGPN(log, format, ...)                    { log->putFormat(su::Log::Level::Notice, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGPD(log, format, ...)                    { log->putFormat(su::Log::Level::Debug, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }


namespace su
{

class LogFileInfo;

class Log
{
public:
    enum Level
    {
        Error = 0,
        Warning,
        Info,
        Notice,
        Debug,
        LevelLog__END
    };

    virtual ~Log();
    Log(const Log&) = delete;
    Log(const Log&&) = delete;

#ifndef SU_LOGS_NOSINGLETON
    static Log& instance();
private:
    Log();
#else
    Log(const std::string& name, const std::string& filename, const std::string& path, Level level = Level::Info);
#endif

private:
    Log& operator=(Log&);

    void setUnsafeDir(const std::string& dir);
    void setUnsafeFilename(const std::string& filename);
    void clearUnsafeFileInfo();

public:
    void put(Level level, const char* source, uint32_t lineno, const std::string& text);
    void putFormat(Level level, const char* source, uint32_t lineno, const char* format, ...);
    
    void setDir(const std::string& dir);
    const std::string& getDir() const;
    
    void setFilename(const std::string& filename);
    const std::string& getFilename(void) const;
    
    void setLevel(Level level);
    Level getLevel(void) const;
    
    bool toTerminal(void) const;
    void setTerminal(bool toTerminal);

    bool toFile() const;
    void setFile(bool toFile);

    void setTimeStamp(bool val);
    bool timeStamp() const;

    std::list<std::string> getNews();

private:
    std::mutex m_mutex;
    LogFileInfo* m_fileInfo = nullptr;
    std::string m_dir = "";
    std::string m_filename = "output";
    std::string m_name = "";
    std::atomic<Level> m_level = Level::Info;
    std::atomic_bool m_toTerminal = true;
    std::atomic_bool m_toFile = true;
    std::atomic_bool m_isTimeStamp = true;

    std::list<std::string> m_list;
};

}
