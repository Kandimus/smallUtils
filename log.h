#pragma once

#include <mutex>
#include <string>
#include <list>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define LOG(level, format, ...)              { su::Log::instance().putFormat((level), __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGE(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Error, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGW(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Warning, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGI(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Info, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGN(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Notice, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }
#define LOGD(format, ...)                    { su::Log::instance().putFormat(su::Log::Level::Debug, __FILENAME__, __LINE__, (format), ##__VA_ARGS__); }

namespace su
{

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

    // Singleton
    virtual ~Log() = default;
    Log(const Log&) = delete;
    Log(const Log&&) = delete;

    static Log& instance();

private:
    Log() = default;
    Log& operator=(Log&);

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

    std::list<std::string> getNews();

private:
    std::mutex m_mutex;
    std::string m_dir = "";
    std::string m_filename = "logs";
    Level m_level = Level::Info;
    bool m_toTerminal = true;
    bool m_toFile = true;

    std::list<std::string> m_list;
};

}
