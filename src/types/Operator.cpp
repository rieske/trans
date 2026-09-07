#include "Operator.h"

namespace type {

std::optional<UnaryOp> unaryOpFromLexeme(std::string_view lexeme) {
    if (lexeme == "+") {
        return UnaryOp::Plus;
    }
    if (lexeme == "-") {
        return UnaryOp::Minus;
    }
    if (lexeme == "!") {
        return UnaryOp::LogicalNot;
    }
    if (lexeme == "~") {
        return UnaryOp::BitNot;
    }
    if (lexeme == "*") {
        return UnaryOp::Deref;
    }
    if (lexeme == "&") {
        return UnaryOp::Addr;
    }
    if (lexeme == "sizeof") {
        return UnaryOp::Sizeof;
    }
    return std::nullopt;
}

std::optional<IncDec> incDecFromLexeme(std::string_view lexeme) {
    if (lexeme == "++") {
        return IncDec::Inc;
    }
    if (lexeme == "--") {
        return IncDec::Dec;
    }
    return std::nullopt;
}

std::optional<ArithmeticOp> arithmeticOpFromLexeme(std::string_view lexeme) {
    if (lexeme == "+") {
        return ArithmeticOp::Add;
    }
    if (lexeme == "-") {
        return ArithmeticOp::Sub;
    }
    if (lexeme == "*") {
        return ArithmeticOp::Mul;
    }
    if (lexeme == "/") {
        return ArithmeticOp::Div;
    }
    if (lexeme == "%") {
        return ArithmeticOp::Mod;
    }
    return std::nullopt;
}

std::optional<ShiftOp> shiftOpFromLexeme(std::string_view lexeme) {
    if (lexeme == "<<") {
        return ShiftOp::Shl;
    }
    if (lexeme == ">>") {
        return ShiftOp::Shr;
    }
    return std::nullopt;
}

std::optional<ComparisonOp> comparisonOpFromLexeme(std::string_view lexeme) {
    if (lexeme == "<") {
        return ComparisonOp::Lt;
    }
    if (lexeme == ">") {
        return ComparisonOp::Gt;
    }
    if (lexeme == "<=") {
        return ComparisonOp::Le;
    }
    if (lexeme == ">=") {
        return ComparisonOp::Ge;
    }
    if (lexeme == "==") {
        return ComparisonOp::Eq;
    }
    if (lexeme == "!=") {
        return ComparisonOp::Ne;
    }
    return std::nullopt;
}

std::optional<BitwiseOp> bitwiseOpFromLexeme(std::string_view lexeme) {
    if (lexeme == "&") {
        return BitwiseOp::BitAnd;
    }
    if (lexeme == "|") {
        return BitwiseOp::BitOr;
    }
    if (lexeme == "^") {
        return BitwiseOp::BitXor;
    }
    return std::nullopt;
}

std::optional<AssignOp> assignOpFromLexeme(std::string_view lexeme) {
    if (lexeme == "=") {
        return AssignOp::Assign;
    }
    if (lexeme == "*=") {
        return AssignOp::MulAssign;
    }
    if (lexeme == "/=") {
        return AssignOp::DivAssign;
    }
    if (lexeme == "%=") {
        return AssignOp::ModAssign;
    }
    if (lexeme == "+=") {
        return AssignOp::AddAssign;
    }
    if (lexeme == "-=") {
        return AssignOp::SubAssign;
    }
    if (lexeme == "<<=") {
        return AssignOp::ShlAssign;
    }
    if (lexeme == ">>=") {
        return AssignOp::ShrAssign;
    }
    if (lexeme == "&=") {
        return AssignOp::AndAssign;
    }
    if (lexeme == "^=") {
        return AssignOp::XorAssign;
    }
    if (lexeme == "|=") {
        return AssignOp::OrAssign;
    }
    return std::nullopt;
}

const char* spelling(UnaryOp op) {
    switch (op) {
    case UnaryOp::Plus:
        return "+";
    case UnaryOp::Minus:
        return "-";
    case UnaryOp::LogicalNot:
        return "!";
    case UnaryOp::BitNot:
        return "~";
    case UnaryOp::Deref:
        return "*";
    case UnaryOp::Addr:
        return "&";
    case UnaryOp::Sizeof:
        return "sizeof";
    }
    return nullptr;
}

const char* spelling(IncDec op) {
    switch (op) {
    case IncDec::Inc:
        return "++";
    case IncDec::Dec:
        return "--";
    }
    return nullptr;
}

const char* spelling(BinaryOp op) {
    switch (op) {
    case BinaryOp::Add:
        return "+";
    case BinaryOp::Sub:
        return "-";
    case BinaryOp::Mul:
        return "*";
    case BinaryOp::Div:
        return "/";
    case BinaryOp::Mod:
        return "%";
    case BinaryOp::Shl:
        return "<<";
    case BinaryOp::Shr:
        return ">>";
    case BinaryOp::BitAnd:
        return "&";
    case BinaryOp::BitOr:
        return "|";
    case BinaryOp::BitXor:
        return "^";
    case BinaryOp::Lt:
        return "<";
    case BinaryOp::Gt:
        return ">";
    case BinaryOp::Le:
        return "<=";
    case BinaryOp::Ge:
        return ">=";
    case BinaryOp::Eq:
        return "==";
    case BinaryOp::Ne:
        return "!=";
    case BinaryOp::LogAnd:
        return "&&";
    case BinaryOp::LogOr:
        return "||";
    }
    return nullptr;
}

const char* spelling(AssignOp op) {
    switch (op) {
    case AssignOp::Assign:
        return "=";
    case AssignOp::MulAssign:
        return "*=";
    case AssignOp::DivAssign:
        return "/=";
    case AssignOp::ModAssign:
        return "%=";
    case AssignOp::AddAssign:
        return "+=";
    case AssignOp::SubAssign:
        return "-=";
    case AssignOp::ShlAssign:
        return "<<=";
    case AssignOp::ShrAssign:
        return ">>=";
    case AssignOp::AndAssign:
        return "&=";
    case AssignOp::XorAssign:
        return "^=";
    case AssignOp::OrAssign:
        return "|=";
    }
    return nullptr;
}

} // namespace type
