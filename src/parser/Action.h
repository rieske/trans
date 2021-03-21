#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>
#include <stack>
#include <string>

#include "ParsingTable.h"
#include "TokenStream.h"

#include "SyntaxTreeBuilder.h"

namespace parser {

class Action {
public:
    virtual ~Action();

    virtual bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const = 0;

    virtual std::string serialize() const = 0;

    static std::unique_ptr<Action> deserialize(std::string serializedAction, const ParsingTable& parsingTable, const Grammar& grammar);
};

class AcceptAction: public Action {
public:
    virtual ~AcceptAction();

    bool parse(std::stack<parse_state>&, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>&) const override;

    std::string serialize() const override;
};

class ShiftAction: public Action {
public:
    ShiftAction(parse_state state);
    virtual ~ShiftAction();

    bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

    std::string serialize() const override;

private:
    parse_state state;
};

class ReduceAction: public Action {
public:
    ReduceAction(const Production& production, const ParsingTable* parsingTable);
    virtual ~ReduceAction();

    bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

    std::string serialize() const override;

private:
    Production production;
    const ParsingTable* parsingTable;
};

class ErrorAction: public Action {
public:
    ErrorAction(parse_state state, int expectedSymbol, const Grammar* grammar);
    virtual ~ErrorAction();

    bool parse(std::stack<parse_state>& parsingStack, TokenStream& tokenStream, std::unique_ptr<SyntaxTreeBuilder>& syntaxTreeBuilder) const override;

    std::string serialize() const override;

private:
    const parse_state state;
    int expectedSymbol;
    const Grammar* grammar;
};

} // namespace parser

#endif // _ACTION_H_
