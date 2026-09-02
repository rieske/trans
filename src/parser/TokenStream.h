#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Token.h"

#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace scanner {
class LexicalSession;
}

namespace parser {
class Grammar;

enum class LexIdContext {
    AsType,
    AsIdentifier,
};

class TokenStream {
public:
    TokenStream(std::function<scanner::Token()> scan, scanner::LexicalSession& session,
            const Grammar& grammar);

    const scanner::Token& getCurrentToken() const;
    const scanner::Token& nextToken();
    // Next scan result, not classified. Only current is a grammar terminal.
    const scanner::Token& peek();
    // Advance without session or id-context effects. Returns the current token.
    scanner::Token takeRaw();

    void setIdContext(LexIdContext context);

private:
    void indexRoles();
    void advanceIdContext(const scanner::Token& token);
    scanner::Token classifyAndStamp(const scanner::Token& token) const;
    void refreshCurrent() const;
    void installNext();

    std::function<scanner::Token()> scan;
    scanner::LexicalSession& session_;
    const Grammar& grammar_;
    LexIdContext idContext_ { LexIdContext::AsType };
    std::vector<std::optional<LexIdContext>> roleAfterId_;

    mutable scanner::Token current_;
    std::optional<scanner::Token> lookahead_;
    mutable unsigned classifiedRevision_ { 0 };
    int idId_ { -1 };
    int typedefNameId_ { -1 };

    struct SpecifierLookahead {
        enum class Op { None, OpenBlock, OpenRecord, OpenEnumBody, Close, EndDeclarators };
        Op consume(std::string_view id);
    private:
        enum class State { None, AfterEnum, AfterEnumTag, AfterRecord, AfterRecordTag };
        State state_ { State::None };
    };
    SpecifierLookahead specifier_;
};

} // namespace parser

#endif // TOKENSTREAM_H_
