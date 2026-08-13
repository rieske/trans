#ifndef CONFIGURATION_PARSER_H_
#define CONFIGURATION_PARSER_H_

#include "driver/Configuration.h"

#include <optional>
#include <string>
#include <vector>

struct ParseResult {
    int exitCode { 0 };
    std::string message;
    std::optional<Configuration> configuration;
    // Makefile -MF paths: driver writes empty stubs after a successful compile.
    std::vector<std::string> depFiles;
};

ParseResult parseCommandLine(int argc, char **argv);

#endif // CONFIGURATION_PARSER_H_
