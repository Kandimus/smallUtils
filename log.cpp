#include <ctime>
#include <fstream>
#include <iostream>
#include <stdarg.h>
#include <unordered_map>
#include <vector>

#include "Log.h"

namespace su
{


class LogFileInfo
{
public:
    LogFileInfo(size_t hash) : m_count(0), m_hash(hash) {}
    ~LogFileInfo() = default;

public:
    std::mutex m_mutex;
    std::atomic_int m_count = 0;
    size_t m_hash = 0;
};

namespace
{

std::mutex mutexFileList;
std::unordered_map<size_t, LogFileInfo*> fileList;
const uint32_t MAX_TEXT_BUFF = 2048;

};

#ifndef SU_LOGS_NOSINGLETON

Log::Log()
{
    setUnsafeFilename(m_filename);
}

Log& Log::instance()
{
    static Log Singleton;
    return Singleton;
}

#else

Log::Log(const std::string& name, const std::string& filename, const std::string& path, Level level)
{
    m_name = name;
    m_level = level;
    setUnsafeDir(path);
    setUnsafeFilename(filename);
}

#endif

Log::~Log()
{
    std::lock_guard<std::mutex> guard(mutexFileList);

    clearUnsafeFileInfo();
}

void Log::put(Level level, const char* source, uint32_t lineno, const std::string& text)
{
    static char mark[Level::LevelLog__END] = { 'E', 'W', 'I', 'N', 'D' };

    char postfix[32] = { 0 };
    char datetimeMark[64] = { 0 };
    std::time_t t = std::time(nullptr);
    std::tm dt;

    localtime_s(&dt, &t);

    if (m_isTimeStamp)
    {
        sprintf_s(postfix, "_%04i.%02i.%02i", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday);
    }

    std::string filename = m_dir + m_filename + postfix + ".log";

    sprintf_s(datetimeMark, "%02i.%02i.%04i %02i:%02i:%02i [%s:%c",
        dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900,
        dt.tm_hour, dt.tm_min, dt.tm_sec,
        m_name.c_str(),
        mark[static_cast<int>(level)]);

    std::string fulltext = datetimeMark;

    if (source)
    {
        fulltext += ":" + std::string(source) + ":" + std::to_string(lineno);
    }

    fulltext += "] " + text + "\n";

    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_toFile)
    {
        std::lock_guard<std::mutex> guard(m_fileInfo->m_mutex);

        std::ofstream file(filename, std::ios_base::app);
        if (file.is_open())
        {
            file << fulltext;
            file.close();
        }
    }

    if (level > m_level)
    {
        return;
    }

    if (m_toTerminal)
    {
        std::cout << fulltext;
    }

    m_list.push_back(fulltext);
}

void Log::putFormat(Level level, const char* source, uint32_t lineno, const char* format, ...)
{
    char* buff = new char[MAX_TEXT_BUFF];

    va_list(args);
    va_start(args, format);
    int result = vsnprintf(buff, MAX_TEXT_BUFF, format, args);
    va_end(args);

    put(level, source, lineno, buff);

    delete[] buff;
}

void Log::setUnsafeDir(const std::string& dir)
{
#ifdef _WIN32
    char sep = '\\';
#else
    char sep = '/'
#endif

    m_dir = dir;

    if (m_dir.back() != sep)
    {
        m_dir += sep;
    }

}

void Log::setDir(const std::string& dir)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    setUnsafeDir(dir);
    setUnsafeFilename(m_filename);
}

const std::string& Log::getDir() const
{
    return m_dir;
}

void Log::setUnsafeFilename(const std::string& filename)
{
    size_t hash = std::hash<std::string>{}(getDir() + filename);

    if (m_fileInfo && m_fileInfo->m_hash == hash)
    {
        return;
    }

    std::lock_guard<std::mutex> gfl(mutexFileList);

    // create a new file info
    if (!fileList.contains(hash))
    {
        fileList[hash] = new LogFileInfo(hash);
    }

    // delete old
    clearUnsafeFileInfo();

    m_filename = filename;
    m_fileInfo = fileList[hash];
    ++m_fileInfo->m_count;
}

void Log::clearUnsafeFileInfo()
{
    if (m_fileInfo)
    {
        if (--m_fileInfo->m_count <= 0)
        {
            fileList.erase(m_fileInfo->m_hash);
            delete m_fileInfo;
        }
        m_fileInfo = nullptr;
    }
}

void Log::setFilename(const std::string& filename)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    setUnsafeFilename(filename);
}

const std::string& Log::getFilename(void) const
{
    return m_filename;
}

void Log::setLevel(Level level)
{
    m_level = level;
}

Log::Level Log::getLevel(void) const
{
    return m_level;
}

void Log::setTerminal(bool toTerminal)
{
    m_toTerminal = toTerminal;
}

bool Log::toTerminal(void) const
{
    return m_toTerminal;
}

bool Log::toFile() const
{
    return m_toFile;
}

void Log::setFile(bool toFile)
{
    m_toFile = toFile;
}

void Log::setTimeStamp(bool val)
{
    m_isTimeStamp = val;
}

bool Log::timeStamp() const
{
    return m_isTimeStamp;
}

std::list<std::string> Log::getNews()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    return std::move(m_list);
}

} // namespace su

