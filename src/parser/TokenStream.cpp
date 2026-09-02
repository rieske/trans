#include "TokenStream.h"

#include "parser/Grammar.h"
#include "scanner/LexicalSession.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace parser {

namespace {

// Closed transition membership: after consuming this token id, next name is
// AsType / AsIdentifier, or keep current role (nullopt). Extend via these
// tables only - do not add one-off previous-token specials in nextToken.

// Keywords, unaries (except '*'), assigns/relops, primaries that put the next
// name in expression position. Extend this table only - keep roleAfter closed.
constexpr std::string_view kExpressionCues[] = {
        "return", "else", "goto", "sizeof", "case",
        "++", "--", "+", "-", "&", "!", "~",
        "=", "+=", "-=", "*=", "/=", "%=",
        "<<=", ">>=", "&=", "^=", "|=",
        "==", "!=", "<", ">", "<=", ">=",
        "<<", ">>", "/", "%", "^", "|",
        "&&", "||", "?", "[",
        "id", "int_const", "char_const", "float_const", "string", "enumeration_const",
};

constexpr std::string_view kIdentifierRoles[] = {
        "struct", "union", "enum", ".", "->", "*", "typedef_name",
        "int", "char", "void", "short", "long", "signed", "unsigned",
        "float", "double",
};

// Restart type position. Intentionally omits ':' (label/case vs ternary).
// ',' prefers type-list / multi-declarator type reuse (`void f(int a, size_t b)`)
// over multi-declarator object lists that reuse a typedef spelling as a name
// without a shadow (`int a, T` in an inner scope). Shadows cover the common
// object case; a full type-vs-expression lattice is out of product scope.
constexpr std::string_view kTypeRestarts[] = {
        ";", "{", "}", ")", "]", "(", ",",
};

} // namespace

TokenStream::SpecifierLookahead::Op TokenStream::SpecifierLookahead::consume(std::string_view id) {
    if (id == "enum") {
        state_ = State::AfterEnum;
        return Op::None;
    }
    if (id == "struct" || id == "union") {
        state_ = State::AfterRecord;
        return Op::None;
    }
    if (id == "{") {
        Op op = Op::OpenBlock;
        if (state_ == State::AfterEnum || state_ == State::AfterEnumTag) {
            op = Op::OpenEnumBody;
        } else if (state_ == State::AfterRecord || state_ == State::AfterRecordTag) {
            op = Op::OpenRecord;
        }
        state_ = State::None;
        return op;
    }
    if (id == "}") {
        return Op::Close;
    }
    if (id == ";") {
        state_ = State::None;
        return Op::EndDeclarators;
    }
    if (state_ == State::AfterEnum && id == "id") {
        state_ = State::AfterEnumTag;
        return Op::None;
    }
    if (state_ == State::AfterRecord && id == "id") {
        state_ = State::AfterRecordTag;
        return Op::None;
    }
    state_ = State::None;
    return Op::None;
}

TokenStream::TokenStream(std::function<scanner::Token()> scan, scanner::LexicalSession& session,
        const Grammar& grammar) :
    scan { std::move(scan) },
    session_ { session },
    grammar_ { grammar },
    current_ { scanner::Token::END, scanner::Token::END, translation_unit::Context { "", 0 } }
{
    indexRoles();
    current_ = classifyAndStamp(this->scan());
    classifiedRevision_ = session.names.revision();
}

void TokenStream::indexRoles() {
    const int endId = grammar_.getEndSymbol();
    const std::size_t size = endId < 0 ? 0 : static_cast<std::size_t>(endId) + 1;
    roleAfterId_.assign(size, std::nullopt);
    auto setRole = [&](std::string_view name, LexIdContext role) {
        if (const auto id = grammar_.trySymbolId(name)) {
            if (*id >= 0 && static_cast<std::size_t>(*id) < roleAfterId_.size()) {
                roleAfterId_[static_cast<std::size_t>(*id)] = role;
            }
        }
    };
    for (std::string_view cue : kExpressionCues) {
        setRole(cue, LexIdContext::AsIdentifier);
    }
    for (std::string_view name : kIdentifierRoles) {
        setRole(name, LexIdContext::AsIdentifier);
    }
    for (std::string_view name : kTypeRestarts) {
        setRole(name, LexIdContext::AsType);
    }
    idId_ = grammar_.trySymbolId("id").value_or(-1);
    typedefNameId_ = grammar_.trySymbolId("typedef_name").value_or(-1);
}

// Transitions use the reclassified token id so shadows and type promotions
// feed roleAfter (not the raw FA id).
void TokenStream::advanceIdContext(const scanner::Token& token) {
    const int id = token.symbolId;
    if (id >= 0 && static_cast<std::size_t>(id) < roleAfterId_.size()) {
        if (const auto next = roleAfterId_[static_cast<std::size_t>(id)]) {
            idContext_ = *next;
        }
    }
}

void TokenStream::setIdContext(LexIdContext context) {
    idContext_ = context;
    refreshCurrent();
}

scanner::Token TokenStream::classifyAndStamp(const scanner::Token& token) const {
    scanner::Token out = token;
    if (token.id == "id" || token.id == "typedef_name") {
        if (session_.names.isIdentifierShadow(token.lexeme)
                || !session_.isTypedef(token.lexeme)
                || idContext_ == LexIdContext::AsIdentifier) {
            if (idId_ < 0) {
                throw std::logic_error { "TokenStream: not a grammar terminal: id" };
            }
            out.id = "id";
            out.symbolId = idId_;
        } else {
            if (typedefNameId_ < 0) {
                throw std::logic_error { "TokenStream: not a grammar terminal: typedef_name" };
            }
            out.id = "typedef_name";
            out.symbolId = typedefNameId_;
        }
        return out;
    }
    const auto symbolId = grammar_.trySymbolId(token.id);
    if (!symbolId) {
        throw std::logic_error { "TokenStream: not a grammar terminal: " + std::string { token.id } };
    }
    out.symbolId = *symbolId;
    return out;
}

void TokenStream::refreshCurrent() const {
    current_ = classifyAndStamp(current_);
    classifiedRevision_ = session_.names.revision();
}

void TokenStream::installNext() {
    if (lookahead_) {
        current_ = classifyAndStamp(*lookahead_);
        lookahead_.reset();
    } else {
        current_ = classifyAndStamp(scan());
    }
    classifiedRevision_ = session_.names.revision();
}

const scanner::Token& TokenStream::getCurrentToken() const {
    if (classifiedRevision_ != session_.names.revision()) {
        refreshCurrent();
    }
    return current_;
}

const scanner::Token& TokenStream::peek() {
    if (!lookahead_) {
        lookahead_.emplace(scan());
    }
    return *lookahead_;
}

scanner::Token TokenStream::takeRaw() {
    scanner::Token taken = getCurrentToken();
    installNext();
    return taken;
}

const scanner::Token& TokenStream::nextToken() {
    const scanner::Token consumed = getCurrentToken();
    advanceIdContext(consumed);
    switch (specifier_.consume(consumed.id)) {
    case SpecifierLookahead::Op::OpenBlock:
        session_.openBrace(scanner::BraceFrame::Block);
        break;
    case SpecifierLookahead::Op::OpenRecord:
        session_.openBrace(scanner::BraceFrame::Record);
        break;
    case SpecifierLookahead::Op::OpenEnumBody:
        session_.openBrace(scanner::BraceFrame::EnumBody);
        break;
    case SpecifierLookahead::Op::Close:
        session_.closeBrace();
        break;
    case SpecifierLookahead::Op::EndDeclarators:
        session_.endDeclarators();
        break;
    case SpecifierLookahead::Op::None:
        break;
    }
    installNext();
    return getCurrentToken();
}

} // namespace parser
