#include "ParsingTableFile.h"

#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace parser {
namespace {

const char* const kKind = "sparse";
constexpr std::size_t kVersion = 1;
const char* const kDelimiter = "%%";

std::string trimLeadingSpace(std::string text) {
    if (!text.empty() && text.front() == ' ') {
        text.erase(0, 1);
    }
    return text;
}

} // namespace

ParsingTableWriter::ParsingTableWriter(std::ostream& out) :
        out_ { out } {
}

void ParsingTableWriter::writeDelimiter() {
    out_ << kDelimiter << '\n';
}

void ParsingTableWriter::writeHeader(std::size_t stateCount) {
    out_ << kKind << ' ' << kVersion << '\n';
    out_ << stateCount << '\n';
    writeDelimiter();
}

void ParsingTableWriter::writeActions(const std::vector<ParsingTableActionRecord>& actions) {
    for (const auto& action : actions) {
        out_ << action.state << ' ' << action.terminal << ' ' << action.serialized << '\n';
    }
    writeDelimiter();
}

void ParsingTableWriter::writeErrors(const std::vector<ParsingTableErrorRecord>& errors) {
    for (const auto& error : errors) {
        out_ << error.state;
        for (const int candidate : error.candidates) {
            out_ << ' ' << candidate;
        }
        out_ << '\n';
    }
    writeDelimiter();
}

void ParsingTableWriter::writeGotos(const std::vector<ParsingTableGotoRecord>& gotos) {
    for (const auto& edge : gotos) {
        out_ << edge.fromState << ' ' << edge.nonterminal << ' ' << edge.toState << '\n';
    }
}

ParsingTableReader::ParsingTableReader(std::istream& in) :
        in_ { in } {
}

void ParsingTableReader::expectDelimiter() {
    const auto line = readNonEmptyLine();
    if (!line || *line != kDelimiter) {
        throw std::runtime_error {
                std::string("error in parsing table configuration file: ") + kDelimiter + " delimiter expected" };
    }
}

std::optional<std::string> ParsingTableReader::readNonEmptyLine() {
    std::string line;
    while (std::getline(in_, line)) {
        if (!line.empty()) {
            return line;
        }
    }
    return std::nullopt;
}

std::size_t ParsingTableReader::readHeader() {
    std::string kind;
    std::size_t version = 0;
    std::size_t stateCount = 0;
    in_ >> kind >> version >> stateCount;
    if (!in_ || kind != kKind) {
        throw std::runtime_error(
                "unsupported parsing table format (expected 'sparse <version> <states>'); "
                "regenerate with ./regenerate-parsing-table.sh");
    }
    if (version != kVersion) {
        throw std::runtime_error("unsupported parsing table version: " + std::to_string(version));
    }
    std::string restOfLine;
    std::getline(in_, restOfLine);
    expectDelimiter();
    return stateCount;
}

std::vector<ParsingTableActionRecord> ParsingTableReader::readActions() {
    std::vector<ParsingTableActionRecord> actions;
    for (;;) {
        const auto line = readNonEmptyLine();
        if (!line || *line == kDelimiter) {
            break;
        }
        std::istringstream lineStream { *line };
        ParsingTableActionRecord record;
        if (!(lineStream >> record.state >> record.terminal)) {
            throw std::runtime_error { "error reading parsing table action: " + *line };
        }
        record.serialized = trimLeadingSpace(
                std::string { std::istreambuf_iterator<char>(lineStream), std::istreambuf_iterator<char>() });
        if (record.serialized.empty()) {
            throw std::runtime_error { "error reading parsing table action: " + *line };
        }
        actions.push_back(std::move(record));
    }
    return actions;
}

std::vector<ParsingTableErrorRecord> ParsingTableReader::readErrors() {
    std::vector<ParsingTableErrorRecord> errors;
    for (;;) {
        const auto line = readNonEmptyLine();
        if (!line || *line == kDelimiter) {
            break;
        }
        std::istringstream lineStream { *line };
        ParsingTableErrorRecord record;
        if (!(lineStream >> record.state)) {
            throw std::runtime_error { "error reading parsing table error row: " + *line };
        }
        int candidate = 0;
        while (lineStream >> candidate) {
            record.candidates.push_back(candidate);
        }
        if (record.candidates.empty()) {
            throw std::runtime_error { "error reading parsing table error row: " + *line };
        }
        errors.push_back(std::move(record));
    }
    return errors;
}

std::vector<ParsingTableGotoRecord> ParsingTableReader::readGotos() {
    std::vector<ParsingTableGotoRecord> gotos;
    for (;;) {
        const auto line = readNonEmptyLine();
        if (!line) {
            break;
        }
        if (*line == kDelimiter) {
            throw std::runtime_error { "unexpected delimiter in parsing table goto section" };
        }
        std::istringstream lineStream { *line };
        ParsingTableGotoRecord record;
        if (!(lineStream >> record.fromState >> record.nonterminal >> record.toState)) {
            throw std::runtime_error { "error reading parsing table goto: " + *line };
        }
        gotos.push_back(record);
    }
    return gotos;
}

} // namespace parser
