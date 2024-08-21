
#include "win/process.h"

#include "log.h"
#include "stringex.h"
#include "utf8.h"

namespace su
{
namespace Process
{

AppObject::AppObject(const char* nativePath, const char* commandLine, const char* workingDirectory, LaunchMode launchMode, su::Log* pLog)
{
    // Validate assumptions
    //SOEUTIL_CHECKSIZE(SoeUtil::Internal::ProcessInformation, PROCESS_INFORMATION);
    //SOEUTIL_CHECKSTATIC(offsetof(PROCESS_INFORMATION, hProcess) == offsetof(SoeUtil::Internal::ProcessInformation, hProcess), InvalidAssumption1);
    //SOEUTIL_CHECKSTATIC(offsetof(PROCESS_INFORMATION, hThread) == offsetof(SoeUtil::Internal::ProcessInformation, hThread), InvalidAssumption2);
    //SOEUTIL_CHECKSTATIC(offsetof(PROCESS_INFORMATION, dwProcessId) == offsetof(SoeUtil::Internal::ProcessInformation, dwProcessId), InvalidAssumption3);
    //SOEUTIL_CHECKSTATIC(offsetof(PROCESS_INFORMATION, dwThreadId) == offsetof(SoeUtil::Internal::ProcessInformation, dwThreadId), InvalidAssumption4);

    memset(&m_processInfo, 0, sizeof(m_processInfo));
    m_launchMode = launchMode;
    m_success = false;
    m_nativePath = nativePath;
    m_commandLine = commandLine;
    m_log = pLog;
    if (workingDirectory != NULL)
    {
        m_workingDirectory = workingDirectory;
    }
}

AppObject::~AppObject()
{
    if (m_success)
    {
        CloseHandle((HANDLE)m_processInfo.hThread);
        CloseHandle((HANDLE)m_processInfo.hProcess);
    }
}


bool AppObject::execute()
{
    STARTUPINFOW si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.lpTitle = NULL;
    
    DWORD flags = 0;
    switch (m_launchMode)
    {
        case LaunchMode::NewConsole: flags = CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP; break;
        case LaunchMode::InheritConsole: flags = CREATE_NEW_PROCESS_GROUP; break;
        case LaunchMode::NoConsole: flags = CREATE_NO_WINDOW; break;
    }
    
    std::string finalCommandLine = su::String_format2("\"%s\" %s", m_nativePath.c_str(), m_commandLine.c_str());
    std::wstring widePath;
    std::wstring wideCommandLine;
    std::wstring wideDir;
    
    su::convertUtf8ToWide(m_nativePath.c_str(), widePath);
    su::convertUtf8ToWide(finalCommandLine.c_str(), wideCommandLine);
    
    LPCSTR pDir = (m_workingDirectory.size() > 0) ? m_workingDirectory.c_str() : nullptr;
    LPCWSTR pWideDir = nullptr;
    if (pDir)
    {
        su::convertUtf8ToWide(pDir, wideDir);
        pWideDir = wideDir.c_str();

        SetCurrentDirectoryW(pWideDir);
    }

    if (CreateProcessW(widePath.c_str(), (LPWSTR)wideCommandLine.c_str(), NULL, NULL, FALSE, flags, NULL, pWideDir, &si, (LPPROCESS_INFORMATION)&m_processInfo) != 0)
    {
        m_success = true;
    }
    else
    {
        DWORD err = ::GetLastError();
        if (m_log)
        {
            LOGPE(m_log, "Process::Object::Execute() - %s, path '%s', commandLine: '%s', workingDir: '%s', failure: (Windows GetLastError=%u)",
                m_loggingContext.empty() ? "null" : m_loggingContext.c_str(),
                m_nativePath.c_str(), finalCommandLine.c_str(), m_workingDirectory.c_str(), err);
        }
    }

    return m_success;
}

bool AppObject::isRunning()
{
    return getProcessExitCode(NULL) == ExitCodeResult::StillRunning;
}

void AppObject::terminate()
{
    if (isRunning())
    {
        TerminateProcess((HANDLE)m_processInfo.hProcess, 1);
    }
}

size_t AppObject::getProcessId()
{
    if (m_success)
    {
        return static_cast<size_t>(m_processInfo.dwProcessId);
    }
    else
    {
        return 0;
    }
}

ExitCodeResult AppObject::getProcessExitCode(int* exitCodeDest)
{
    if (m_success)
    {
        DWORD ec;
        if (GetExitCodeProcess((HANDLE)m_processInfo.hProcess, &ec))
        {
            if (ec == STILL_ACTIVE)
            {
                return ExitCodeResult::StillRunning;
            }
            else if (exitCodeDest)
            {
                *exitCodeDest = (int)ec;
            }

            return ExitCodeResult::Exited;
        }
    }

    return ExitCodeResult::NotStarted;
}

} //namespace Process
} //namespace su
