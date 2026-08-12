#ifndef PARSER_PARSING_TABLE_FILE_H_
#define PARSER_PARSING_TABLE_FILE_H_

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include "Action.h"

namespace parser {

struct ParsingTableActionRecord {
    parse_state state;
    int terminal;
    std::string serialized;
};

struct ParsingTableErrorRecord {
    parse_state state;
    std::vector<int> candidates;
};

struct ParsingTableGotoRecord {
    parse_state fromState;
    int nonterminal;
    parse_state toState;
};

class ParsingTableWriter {
public:
    explicit ParsingTableWriter(std::ostream& out);

    void writeHeader(std::size_t stateCount);
    void writeActions(const std::vector<ParsingTableActionRecord>& actions);
    void writeErrors(const std::vector<ParsingTableErrorRecord>& errors);
    void writeGotos(const std::vector<ParsingTableGotoRecord>& gotos);

private:
    std::ostream& out_;
    void writeDelimiter();
};

class ParsingTableReader {
public:
    explicit ParsingTableReader(std::istream& in);

    std::size_t readHeader();
    std::vector<ParsingTableActionRecord> readActions();
    std::vector<ParsingTableErrorRecord> readErrors();
    std::vector<ParsingTableGotoRecord> readGotos();

private:
    std::istream& in_;

    void expectDelimiter();
    std::optional<std::string> readNonEmptyLine();
};

} // namespace parser

#endif
