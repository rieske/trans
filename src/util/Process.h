#ifndef UTIL_PROCESS_H_
#define UTIL_PROCESS_H_

#include <string>
#include <vector>

namespace util {

struct ProcessResult {
    int exitCode { -1 };
    std::string stdoutOutput;
    std::string stderrOutput;
};

// Run a program with an explicit argv (no shell). argv[0] is resolved via PATH.
//
// Modes intentionally cover both call sites:
// - Compiler: no stdin, capture stderr (and unused stdout) for nasm/gcc errors.
// - Functional tests: optional stdin bytes; optional stdoutPath redirects child
//   stdout to a file instead of capturing it.
// Stdout and stderr are drained concurrently (poll) so dual-stream children cannot deadlock.
ProcessResult runProcess(const std::vector<std::string>& argv,
        const std::string& stdinData = {},
        const std::string& stdoutPath = {});

// Like runProcess, but throws std::runtime_error if exitCode != 0.
// The exception message includes the command and stderr (or stdout if stderr is empty).
void runProcessOrThrow(const std::vector<std::string>& argv,
        const std::string& stdinData = {},
        const std::string& stdoutPath = {});

} // namespace util

#endif
