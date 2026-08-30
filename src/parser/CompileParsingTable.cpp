#include "CompileParsingTable.h"

#include "Action.h"
#include "Grammar.h"
#include "ParsingTable.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace parser {
namespace {

constexpr uint8_t kEmpty = 0;
constexpr uint8_t kShift = 1;
constexpr uint8_t kReduce = 2;
constexpr uint8_t kAccept = 3;

template<typename T>
void writeCommaList(std::ostream& out, const std::vector<T>& values) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << +values[i];
    }
}

template<typename T>
void writeArray(std::ostream& out, const char* typeName, const char* name, const std::vector<T>& values) {
    if (values.empty()) {
        out << "const " << typeName << "* const " << name << " = nullptr;\n";
        return;
    }
    out << "const " << typeName << " " << name << "[" << values.size() << "] = {\n";
    writeCommaList(out, values);
    out << "\n};\n";
}

void requireFitsU16(std::size_t value, const char* what) {
    if (value > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error { std::string(what) + " does not fit compiled table payload" };
    }
}

void requireFitsI16(std::size_t value, const char* what) {
    if (value > static_cast<std::size_t>(std::numeric_limits<int16_t>::max())) {
        throw std::runtime_error { std::string(what) + " does not fit compiled table cell" };
    }
}

uint8_t compiledKind(const Action& action) {
    switch (action.kind()) {
    case Action::Kind::Shift:
        return kShift;
    case Action::Kind::Reduce:
        return kReduce;
    case Action::Kind::Accept:
        return kAccept;
    case Action::Kind::Error:
        throw std::logic_error { "compileParsingTable: error is not an explicit cell" };
    }
    throw std::logic_error { "compileParsingTable: unhandled Action::Kind" };
}

uint16_t compiledPayload(const Action& action) {
    switch (action.kind()) {
    case Action::Kind::Shift:
        requireFitsU16(action.shiftState(), "shift state");
        return static_cast<uint16_t>(action.shiftState());
    case Action::Kind::Reduce:
        requireFitsU16(static_cast<std::size_t>(action.productionId()), "production id");
        return static_cast<uint16_t>(action.productionId());
    case Action::Kind::Accept:
        return 0;
    case Action::Kind::Error:
        throw std::logic_error { "compileParsingTable: error is not an explicit cell" };
    }
    throw std::logic_error { "compileParsingTable: unhandled Action::Kind" };
}

int minMaxOrZero(int value, bool empty) {
    return empty ? 0 : value;
}

template<typename T>
void emitField(std::ostream& out, const char* name, const T& value) {
    out << "    " << name << " = " << value << ";\n";
}

void emitAssign(std::ostream& out, const char* member, const char* array, std::size_t count) {
    out << "    " << member << ".assign(" << array << ", " << array << " + " << count << ");\n";
}

} // namespace

struct ParsingTableAccess {
    static ParsingTable compile(
            const LookaheadActionTable& actions,
            const std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash>& gotos,
            const Grammar& grammar);
    static void write(const ParsingTable& table, const std::string& sourcePath);
};

ParsingTable ParsingTableAccess::compile(
        const LookaheadActionTable& actions,
        const std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash>& gotos,
        const Grammar& grammar) {
    const std::size_t states = actions.size();
    if (states == 0) {
        throw std::runtime_error { "compiled parsing table has no states" };
    }

    const auto explicitActions = actions.explicitActions();
    int minTerminal = 0;
    int maxTerminal = 0;
    bool haveTerminal = false;
    auto considerTerminal = [&](int id) {
        if (!haveTerminal) {
            minTerminal = id;
            maxTerminal = id;
            haveTerminal = true;
            return;
        }
        minTerminal = std::min(minTerminal, id);
        maxTerminal = std::max(maxTerminal, id);
    };
    for (const auto& action : explicitActions) {
        considerTerminal(action.lookahead);
    }
    if (!haveTerminal) {
        for (const int id : grammar.getTerminalIDs()) {
            considerTerminal(id);
        }
    }
    const int terminalColumns = haveTerminal ? maxTerminal - minTerminal + 1 : 0;

    int minNonterminal = 0;
    int maxNonterminal = 0;
    bool haveNonterminal = false;
    for (const auto& entry : gotos) {
        const int id = entry.first.second;
        if (!haveNonterminal) {
            minNonterminal = id;
            maxNonterminal = id;
            haveNonterminal = true;
        } else {
            minNonterminal = std::min(minNonterminal, id);
            maxNonterminal = std::max(maxNonterminal, id);
        }
    }
    const int nonterminalColumns = haveNonterminal ? maxNonterminal - minNonterminal + 1 : 0;

    ParsingTable table;
    table.grammar_ = &grammar;
    table.stateCount_ = states;
    table.ruleCount_ = grammar.ruleCount();
    table.minTerminal_ = minMaxOrZero(minTerminal, !haveTerminal);
    table.maxTerminal_ = minMaxOrZero(maxTerminal, !haveTerminal);
    table.terminalColumns_ = terminalColumns;
    table.minNonterminal_ = minMaxOrZero(minNonterminal, !haveNonterminal);
    table.maxNonterminal_ = minMaxOrZero(maxNonterminal, !haveNonterminal);
    table.nonterminalColumns_ = nonterminalColumns;
    table.actionKind_.assign(states * static_cast<std::size_t>(std::max(terminalColumns, 0)), kEmpty);
    table.actionPayload_.assign(table.actionKind_.size(), 0);
    table.gotos_.assign(states * static_cast<std::size_t>(std::max(nonterminalColumns, 0)), -1);
    table.terminalIds_ = grammar.getTerminalIDs();

    for (const auto& action : explicitActions) {
        if (action.state >= states) {
            throw std::runtime_error { "action state out of range" };
        }
        const std::size_t index = action.state * static_cast<std::size_t>(terminalColumns)
                + static_cast<std::size_t>(action.lookahead - minTerminal);
        table.actionKind_[index] = compiledKind(action.action);
        table.actionPayload_[index] = compiledPayload(action.action);
    }

    for (const auto& entry : gotos) {
        if (entry.first.first >= states) {
            throw std::runtime_error { "goto state out of range" };
        }
        requireFitsI16(entry.second, "goto state");
        const std::size_t index = entry.first.first * static_cast<std::size_t>(nonterminalColumns)
                + static_cast<std::size_t>(entry.first.second - minNonterminal);
        table.gotos_[index] = static_cast<int16_t>(entry.second);
    }

    table.errorOffset_.assign(states + 1, 0);
    const auto errorRows = actions.errorRows();
    std::vector<const LookaheadActionTable::ErrorRow*> errorByState(states, nullptr);
    for (const auto& error : errorRows) {
        if (error.state >= states) {
            throw std::runtime_error { "error state out of range" };
        }
        errorByState[error.state] = &error;
    }
    for (std::size_t state = 0; state < states; ++state) {
        if (errorByState[state] != nullptr) {
            table.errorCandidates_.insert(table.errorCandidates_.end(),
                    errorByState[state]->candidates.begin(),
                    errorByState[state]->candidates.end());
        }
        table.errorOffset_[state + 1] = static_cast<uint32_t>(table.errorCandidates_.size());
    }

    table.validate();
    return table;
}

void ParsingTableAccess::write(const ParsingTable& table, const std::string& sourcePath) {
    if (table.stateCount_ == 0) {
        throw std::runtime_error { "compiled parsing table has no states" };
    }

    std::ofstream source { sourcePath };
    if (!source) {
        throw std::runtime_error { "unable to write compiled parsing table source: " + sourcePath };
    }

    source << R"(#include "parser/ParsingTable.h"

#include <cstdint>

namespace {

)";
    writeArray(source, "uint8_t", "actionKind", table.actionKind_);
    writeArray(source, "uint16_t", "actionPayload", table.actionPayload_);
    writeArray(source, "int16_t", "gotos", table.gotos_);
    writeArray(source, "uint32_t", "errorOffset", table.errorOffset_);
    writeArray(source, "int", "errorCandidates", table.errorCandidates_);
    writeArray(source, "int", "terminalIds", table.terminalIds_);
    source << R"(
} // namespace

namespace parser {

void ParsingTable::loadProduct() {
)";
    emitField(source, "stateCount_", table.stateCount_);
    emitField(source, "ruleCount_", table.ruleCount_);
    emitField(source, "minTerminal_", table.minTerminal_);
    emitField(source, "maxTerminal_", table.maxTerminal_);
    emitField(source, "terminalColumns_", table.terminalColumns_);
    emitField(source, "minNonterminal_", table.minNonterminal_);
    emitField(source, "maxNonterminal_", table.maxNonterminal_);
    emitField(source, "nonterminalColumns_", table.nonterminalColumns_);
    emitAssign(source, "actionKind_", "actionKind", table.actionKind_.size());
    emitAssign(source, "actionPayload_", "actionPayload", table.actionPayload_.size());
    emitAssign(source, "gotos_", "gotos", table.gotos_.size());
    emitAssign(source, "errorOffset_", "errorOffset", table.errorOffset_.size());
    emitAssign(source, "errorCandidates_", "errorCandidates", table.errorCandidates_.size());
    emitAssign(source, "terminalIds_", "terminalIds", table.terminalIds_.size());
    source << R"(}

ParsingTable::ParsingTable(const Grammar* grammar) :
        grammar_ { grammar } {
    loadProduct();
    validate();
}

} // namespace parser
)";
}

ParsingTable compileParsingTable(
        const LookaheadActionTable& actions,
        const std::unordered_map<StateSymbolKey, parse_state, StateSymbolHash>& gotos,
        const Grammar& grammar) {
    return ParsingTableAccess::compile(actions, gotos, grammar);
}

void writeParsingTableSource(const ParsingTable& table, const std::string& path) {
    ParsingTableAccess::write(table, path);
}

} // namespace parser
