#include "util/Process.h"

#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace util {
namespace {

constexpr size_t kIoChunk = 4096;

void closeFd(int& fd) {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

void closePipe(int pipefds[2]) {
    closeFd(pipefds[0]);
    closeFd(pipefds[1]);
}

[[noreturn]] void throwErrno(const char* what) {
    throw std::runtime_error(std::string(what) + ": " + std::strerror(errno));
}

std::string joinArgv(const std::vector<std::string>& argv) {
    std::string joined;
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i != 0) {
            joined.push_back(' ');
        }
        joined += argv[i];
    }
    return joined;
}

// Multiplex stdin write + stdout/stderr reads until all pipe ends close.
// Avoids the classic deadlock from draining one stream before the other.
void pumpIo(int& stdinWriteFd, const std::string& stdinData, size_t& stdinOffset,
        int& stdoutReadFd, std::string* stdoutOut,
        int& stderrReadFd, std::string& stderrOut) {
    std::array<char, kIoChunk> buf {};

    while (stdinWriteFd >= 0 || stdoutReadFd >= 0 || stderrReadFd >= 0) {
        std::array<pollfd, 3> pfds {};
        nfds_t nfds = 0;

        int stdinIdx = -1;
        int stdoutIdx = -1;
        int stderrIdx = -1;

        if (stdinWriteFd >= 0) {
            stdinIdx = static_cast<int>(nfds);
            pfds[nfds++] = pollfd { stdinWriteFd, POLLOUT, 0 };
        }
        if (stdoutReadFd >= 0) {
            stdoutIdx = static_cast<int>(nfds);
            pfds[nfds++] = pollfd { stdoutReadFd, POLLIN, 0 };
        }
        if (stderrReadFd >= 0) {
            stderrIdx = static_cast<int>(nfds);
            pfds[nfds++] = pollfd { stderrReadFd, POLLIN, 0 };
        }

        int ready = ::poll(pfds.data(), nfds, -1);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            throwErrno("poll");
        }

        auto handleRead = [&](int& fd, short revents, std::string* dest) {
            if (fd < 0) {
                return;
            }
            if (!(revents & (POLLIN | POLLHUP | POLLERR))) {
                return;
            }
            if (revents & POLLIN) {
                ssize_t n = ::read(fd, buf.data(), buf.size());
                if (n > 0) {
                    if (dest != nullptr) {
                        dest->append(buf.data(), static_cast<size_t>(n));
                    }
                    return;
                }
                if (n < 0 && errno == EINTR) {
                    return;
                }
            }
            closeFd(fd);
        };

        if (stdinIdx >= 0) {
            short revents = pfds[static_cast<size_t>(stdinIdx)].revents;
            if (revents & (POLLOUT | POLLERR | POLLHUP)) {
                if (revents & POLLOUT) {
                    size_t remaining = stdinData.size() - stdinOffset;
                    ssize_t n = ::write(stdinWriteFd, stdinData.data() + stdinOffset, remaining);
                    if (n > 0) {
                        stdinOffset += static_cast<size_t>(n);
                        if (stdinOffset >= stdinData.size()) {
                            closeFd(stdinWriteFd);
                        }
                    } else if (n < 0 && errno == EINTR) {
                        // retry
                    } else {
                        // EPIPE or other write error: child closed early.
                        closeFd(stdinWriteFd);
                    }
                } else {
                    closeFd(stdinWriteFd);
                }
            }
        }

        if (stdoutIdx >= 0) {
            handleRead(stdoutReadFd, pfds[static_cast<size_t>(stdoutIdx)].revents, stdoutOut);
        }
        if (stderrIdx >= 0) {
            handleRead(stderrReadFd, pfds[static_cast<size_t>(stderrIdx)].revents, &stderrOut);
        }
    }
}

} // namespace

ProcessResult runProcess(const std::vector<std::string>& argv,
        const std::string& stdinData,
        const std::string& stdoutPath) {
    if (argv.empty()) {
        throw std::invalid_argument("runProcess: empty argv");
    }

    int stdinPipe[2] = { -1, -1 };
    int stdoutPipe[2] = { -1, -1 };
    int stderrPipe[2] = { -1, -1 };

    const bool captureStdout = stdoutPath.empty();
    const bool feedStdin = !stdinData.empty();

    auto cleanupPipes = [&]() {
        closePipe(stdinPipe);
        closePipe(stdoutPipe);
        closePipe(stderrPipe);
    };

    if (feedStdin && ::pipe(stdinPipe) != 0) {
        throwErrno("pipe(stdin)");
    }
    if (captureStdout && ::pipe(stdoutPipe) != 0) {
        cleanupPipes();
        throwErrno("pipe(stdout)");
    }
    if (::pipe(stderrPipe) != 0) {
        cleanupPipes();
        throwErrno("pipe(stderr)");
    }

    pid_t pid = ::fork();
    if (pid < 0) {
        cleanupPipes();
        throwErrno("fork");
    }

    if (pid == 0) {
        if (feedStdin) {
            ::dup2(stdinPipe[0], STDIN_FILENO);
        } else {
            int devnull = ::open("/dev/null", O_RDONLY);
            if (devnull >= 0) {
                ::dup2(devnull, STDIN_FILENO);
                ::close(devnull);
            }
        }

        if (captureStdout) {
            ::dup2(stdoutPipe[1], STDOUT_FILENO);
        } else {
            int outFd = ::open(stdoutPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outFd < 0) {
                _exit(126);
            }
            ::dup2(outFd, STDOUT_FILENO);
            ::close(outFd);
        }

        ::dup2(stderrPipe[1], STDERR_FILENO);

        closePipe(stdinPipe);
        closePipe(stdoutPipe);
        closePipe(stderrPipe);

        std::vector<char*> cargv;
        cargv.reserve(argv.size() + 1);
        for (const auto& arg : argv) {
            cargv.push_back(const_cast<char*>(arg.c_str()));
        }
        cargv.push_back(nullptr);

        ::execvp(cargv[0], cargv.data());
        _exit(127);
    }

    // Parent: close child-side ends; keep parent-side ends in the pipe arrays.
    closeFd(stdinPipe[0]);
    closeFd(stdoutPipe[1]);
    closeFd(stderrPipe[1]);

    if (!feedStdin) {
        closeFd(stdinPipe[1]);
    }
    if (!captureStdout) {
        closeFd(stdoutPipe[0]);
    }

    ProcessResult result;
    size_t stdinOffset = 0;

    try {
        pumpIo(stdinPipe[1], stdinData, stdinOffset,
                stdoutPipe[0], captureStdout ? &result.stdoutOutput : nullptr,
                stderrPipe[0], result.stderrOutput);
    } catch (...) {
        closePipe(stdinPipe);
        closePipe(stdoutPipe);
        closePipe(stderrPipe);
        int status = 0;
        ::waitpid(pid, &status, 0);
        throw;
    }

    // pumpIo closes all parent ends it was given.
    int status = 0;
    if (::waitpid(pid, &status, 0) < 0) {
        throwErrno("waitpid");
    }
    if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        result.exitCode = 128 + WTERMSIG(status);
    } else {
        result.exitCode = -1;
    }
    return result;
}

void runProcessOrThrow(const std::vector<std::string>& argv,
        const std::string& stdinData,
        const std::string& stdoutPath) {
    ProcessResult result = runProcess(argv, stdinData, stdoutPath);
    if (result.exitCode == 0) {
        return;
    }
    std::string message = "command failed (" + std::to_string(result.exitCode) + "): " + joinArgv(argv);
    if (!result.stderrOutput.empty()) {
        message += "\n" + result.stderrOutput;
    } else if (!result.stdoutOutput.empty()) {
        message += "\n" + result.stdoutOutput;
    }
    throw std::runtime_error { message };
}

} // namespace util
