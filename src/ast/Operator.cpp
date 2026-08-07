#include "Operator.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

OperatorKind binaryOperatorKindFromLexeme(const std::string& lexeme) {
    if (lexeme == "+=") return OperatorKind::AddAssign;
    if (lexeme == "-=") return OperatorKind::SubAssign;
    if (lexeme == "*=") return OperatorKind::MulAssign;
    if (lexeme == "/=") return OperatorKind::DivAssign;
    if (lexeme == "%=") return OperatorKind::ModAssign;
    if (lexeme == "&=") return OperatorKind::AndAssign;
    if (lexeme == "|=") return OperatorKind::OrAssign;
    if (lexeme == "^=") return OperatorKind::XorAssign;
    if (lexeme == "<<=") return OperatorKind::ShlAssign;
    if (lexeme == ">>=") return OperatorKind::ShrAssign;
    if (lexeme == "<<") return OperatorKind::Shl;
    if (lexeme == ">>") return OperatorKind::Shr;
    if (lexeme == "<=") return OperatorKind::Le;
    if (lexeme == ">=") return OperatorKind::Ge;
    if (lexeme == "==") return OperatorKind::Eq;
    if (lexeme == "!=") return OperatorKind::Ne;
    if (lexeme == "&&") return OperatorKind::LogicalAnd;
    if (lexeme == "||") return OperatorKind::LogicalOr;
    if (lexeme == "++") return OperatorKind::Inc;
    if (lexeme == "--") return OperatorKind::Dec;
    if (lexeme == "()") return OperatorKind::Call;
    if (lexeme == "[]") return OperatorKind::Index;
    if (lexeme == "+") return OperatorKind::Add;
    if (lexeme == "-") return OperatorKind::Sub;
    if (lexeme == "*") return OperatorKind::Mul;
    if (lexeme == "/") return OperatorKind::Div;
    if (lexeme == "%") return OperatorKind::Mod;
    if (lexeme == "<") return OperatorKind::Lt;
    if (lexeme == ">") return OperatorKind::Gt;
    if (lexeme == "&") return OperatorKind::BitAnd;
    if (lexeme == "|") return OperatorKind::BitOr;
    if (lexeme == "^") return OperatorKind::BitXor;
    if (lexeme == "!") return OperatorKind::LogicalNot;
    if (lexeme == "~") return OperatorKind::BitNot;
    if (lexeme == "=") return OperatorKind::Assign;
    if (lexeme == ",") return OperatorKind::Comma;
    // Type-cast placeholder carries a type name (e.g. "int").
    return OperatorKind::Cast;
}

OperatorKind unaryOperatorKindFromLexeme(const std::string& lexeme) {
    if (lexeme == "++") return OperatorKind::Inc;
    if (lexeme == "--") return OperatorKind::Dec;
    if (lexeme == "sizeof") return OperatorKind::Sizeof;
    if (lexeme == "*") return OperatorKind::Deref;
    if (lexeme == "&") return OperatorKind::AddressOf;
    if (lexeme == "+") return OperatorKind::Add;
    if (lexeme == "-") return OperatorKind::Sub;
    if (lexeme == "!") return OperatorKind::LogicalNot;
    if (lexeme == "~") return OperatorKind::BitNot;
    return OperatorKind::Unknown;
}

OperatorKind compoundAssignBase(OperatorKind kind) {
    switch (kind) {
    case OperatorKind::AddAssign: return OperatorKind::Add;
    case OperatorKind::SubAssign: return OperatorKind::Sub;
    case OperatorKind::MulAssign: return OperatorKind::Mul;
    case OperatorKind::DivAssign: return OperatorKind::Div;
    case OperatorKind::ModAssign: return OperatorKind::Mod;
    case OperatorKind::AndAssign: return OperatorKind::BitAnd;
    case OperatorKind::OrAssign: return OperatorKind::BitOr;
    case OperatorKind::XorAssign: return OperatorKind::BitXor;
    case OperatorKind::ShlAssign: return OperatorKind::Shl;
    case OperatorKind::ShrAssign: return OperatorKind::Shr;
    default: return OperatorKind::Unknown;
    }
}

bool isCompoundAssign(OperatorKind kind) {
    return compoundAssignBase(kind) != OperatorKind::Unknown;
}

Operator::Operator(std::string lexeme, OperatorKind kind) :
        lexeme { std::move(lexeme) },
        kind { kind } {
}

Operator::Operator(std::string lexeme) :
        // Compute kind before moving: argument evaluation order is unspecified.
        lexeme { },
        kind { binaryOperatorKindFromLexeme(lexeme) } {
    this->lexeme = std::move(lexeme);
}

std::unique_ptr<Operator> Operator::makeUnary(std::string lexeme) {
    const OperatorKind k = unaryOperatorKindFromLexeme(lexeme);
    return std::unique_ptr<Operator>(new Operator(std::move(lexeme), k));
}

void Operator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::string Operator::getLexeme() const {
    return lexeme;
}

} // namespace ast
