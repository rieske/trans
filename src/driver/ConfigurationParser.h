#ifndef CONFIGURATION_PARSER_H_
#define CONFIGURATION_PARSER_H_

#include "driver/Configuration.h"

#include <optional>
#include <string>

struct ParseResult {
    int exitCode { 0 };
    std::string message;
    std::optional<Configuration> configuration;
};

ParseResult parseCommandLine(int argc, char **argv);

#endif // CONFIGURATION_PARSER_H_
