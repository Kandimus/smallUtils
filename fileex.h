
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace su
{
namespace fs
{

const char LinuxPathSeparator = '/';
const char WinPathSeparator = '\\';

const char PathSeparator =
#ifdef _WIN32
    WinPathSeparator
#else
    LinuxPathSeparator
#endif
    ;

enum Result
{
    OK = 0,
    Fault = 0x010000,
    NotFound,
    EAccess,
    IsDir,
    EDir,
    Blocked,
    ReadOnly,
    CantDelete,
    CantOpen,
    CantWrite,
};

inline void addSeparator(std::string& path)
{
    if (path.back() != WinPathSeparator && path.back() != LinuxPathSeparator)
    {
        path += PathSeparator;
    }
}

extern Result createFolder(const std::string& path);
extern Result deleteFile(const std::string& filename);
extern Result load(const std::string& filename, std::vector<uint8_t>& data);
extern Result load(const std::string& filename, std::string& text);
extern Result save(const std::string& filename, const std::vector<uint8_t>& data);
extern Result save(const std::string& filename, const std::string& text);
//extern unsigned int append(const std::string& filename, const std::string& text);
//extern unsigned int append(const std::string& filename, const std::vector<uint8_t>& text);
//extern unsigned int garanteedSave(const std::string& filename, const std::string& text);

} //namespace fs
} // namespace su
