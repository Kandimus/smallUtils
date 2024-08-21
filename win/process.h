#pragma once

#include <Windows.h>
#include <string>

namespace su
{
class Log;

namespace Process
{

enum class LaunchMode
{
    NewConsole,
    InheritConsole,
    NoConsole
};

enum class ExitCodeResult
{
    NoInit = -100,
    NotStarted = -2,
    StillRunning = -1,
    Exited = 0
};

class BaseObject
{
protected:
    PROCESS_INFORMATION m_processInfo;

    std::string m_nativePath;
    std::string m_commandLine;
    std::string m_workingDirectory;
    LaunchMode m_launchMode;
    bool m_success;
};

class AppObject : public BaseObject
{
public:
    // nativePath is the name of the executable file to run
    // commandLine is the parameters to pass to that executable.  Note: commandLine should not replicate the nativePath at the beginning (ie, argv[0]).
    // workingDirectory launches the process in the directory specified.  This may be NULL, and if launches the process using the current working directory of the requestor.
    AppObject(const char* nativePath, const char* commandLine, const char* workingDirectory = 0, LaunchMode launchMode = LaunchMode::NewConsole, su::Log* pLog = nullptr);
    ~AppObject();

    std::string getLoggingContext() const { return m_loggingContext; }
    void setLoggingContext(const std::string& str) { m_loggingContext = str; }

    std::string getCommandLine() const { return m_commandLine; }
    void setCommandLine(const std::string& str) { m_commandLine = str; }

    bool execute();                 // returns true if successfully executed, false otherwise
    bool isRunning();               // returns true if process is actually running, false otherwise
    void terminate();
    size_t getProcessId();          // returns 0 if execution failed
    ExitCodeResult getProcessExitCode(int* exitCodeDest);  // when the process has exited, cGetProcessExitCodeResultExited is returned and the exit code is stored in exitCodeDest.

private:
    su::Log* m_log = nullptr;
    std::string m_loggingContext = "";
};

} //namespace Process
} //namespace su
