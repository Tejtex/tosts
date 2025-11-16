#include <string>
#include <vector>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// run is a cross-platform function for creating a process, getting it's stdout, and limiting the time and memory. memory should be in Bytes and timout in miliseconds
#ifdef _WIN32
#include <windows.h>
std::pair<std::string, int> run(const std::vector<std::string> &command, std::string &test, int timeout, int memory_limit)
{
    std::string ncommand;
    for (std::string arg : command)
    {
        ncommand += arg;
    }
    HANDLE job = CreateJobObject(NULL, NULL);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
    info.BasicLimitInformation.LimitFlags =
        JOB_OBJECT_LIMIT_PROCESS_MEMORY |
        JOB_OBJECT_LIMIT_JOB_MEMORY |
        JOB_OBJECT_LIMIT_ACTIVE_PROCESS |
        JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    info.ProcessMemoryLimit = memory_limit;
    info.JobMemoryLimit = memory_limit;

    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &info, sizeof(info));

    HANDLE child_stdin_read;
    HANDLE child_stdout_write;

    HANDLE parent_stdin_write;
    HANDLE parent_stdout_read;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    CreatePipe(&child_stdin_read, &parent_stdin_write, &sa, 0);
    SetHandleInformation(parent_stdin_write, HANDLE_FLAG_INHERIT, 0);

    CreatePipe(&parent_stdout_read, &child_stdout_write, &sa, 0);
    SetHandleInformation(parent_stdout_read, HANDLE_FLAG_INHERIT, 0);

    DWORD written = 0;

    if (!WriteFile(parent_stdin_write, test.data(), (DWORD)test.size(), &written, NULL))
        return {"", 3}; // RUNTIME ERROR

    CloseHandle(parent_stdin_write);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = child_stdin_read;
    si.hStdOutput = child_stdout_write;
    PROCESS_INFORMATION pi{};

    if (!CreateProcessA(NULL, (CHAR *)ncommand.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
        return {"", 3}; // RUNTIME ERROR

    AssignProcessToJobObject(job, pi.hProcess);

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeout);
    if (waitResult == WAIT_TIMEOUT)
    {
        // Kill process if timeout
        TerminateJobObject(job, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(job);
        return {"", 1}; // TIMEOUT
    }

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    char buf[4096];
    DWORD readBytes;
    std::string output;

    while (true)
    {
        BOOL okRead = ReadFile(
            parent_stdout_read,
            buf,
            sizeof(buf),
            &readBytes,
            NULL);

        if (!okRead || readBytes == 0)
            break; // EOF or child ended

        output.append(buf, readBytes);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(job);

    if (exitCode == STATUS_NO_MEMORY)
        return {"", 2}; // MLE

    return {output, 0};
}
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <vector>
#include <cstring>

std::pair<std::string, int> run(const std::vector<std::string> &command,
                                std::string &input,
                                int timeout, int memory_limit_kb)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1)
        return {"", 3}; // RUNTIME ERROR

    pid_t pid = fork();
    if (pid < 0)
        return {"", 3}; // RUNTIME ERROR

    if (pid == 0)
    {
        // Child process
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stdout_pipe[1], STDERR_FILENO);

        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        // Set memory limit
        rlimit mem_limit{};
        mem_limit.rlim_cur = memory_limit_kb * 1024;
        mem_limit.rlim_max = memory_limit_kb * 1024;
        setrlimit(RLIMIT_AS, &mem_limit);

        // Prepare args
        std::vector<char *> args;
        for (const auto &arg : command)
            args.push_back(const_cast<char *>(arg.c_str()));
        args.push_back(nullptr);

        execvp(args[0], args.data());
        _exit(127); // exec failed
    }

    // Parent process
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    // Write input
    if (!input.empty())
    {
        ssize_t written = write(stdin_pipe[1], input.data(), input.size());
        if (written < 0)
        {
            close(stdin_pipe[1]);
            close(stdout_pipe[0]);
            kill(pid, SIGKILL);
            return {"", 3}; // RUNTIME ERROR
        }
    }
    close(stdin_pipe[1]);

    // Wait with timeout
    int status = 0;
    int waited = 0;
    const int interval_ms = 10;
    while (waited < timeout)
    {
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret == pid)
            break;
        usleep(interval_ms * 1000);
        waited += interval_ms;
    }

    if (waited >= timeout)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        close(stdout_pipe[0]);
        return {"", 1}; // TIMEOUT
    }

    // Read output
    std::string output;
    char buf[4096];
    ssize_t n;
    while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0)
        output.append(buf, n);

    close(stdout_pipe[0]);

    if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        if (sig == SIGKILL)
            return {"", 2}; // MLE
    }
    if (WEXITSTATUS(status) != 0)
        return {output, 3}; // RUNTIME ERROR

    return {output, 0};
}
#endif