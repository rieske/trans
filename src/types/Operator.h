#ifndef TYPES_OPERATOR_H_
#define TYPES_OPERATOR_H_

#include <optional>
#include <stdexcept>
#include <string_view>

namespace type {

enum class UnaryOp {
    Plus,
    Minus,
    LogicalNot,
    BitNot,
    Deref,
    Addr,
    Sizeof,
};

enum class IncDec {
    Inc,
    Dec,
};

enum class ArithmeticOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
};

enum class ShiftOp {
    Shl,
    Shr,
};

enum class ComparisonOp {
    Lt,
    Gt,
    Le,
    Ge,
    Eq,
    Ne,
};

enum class BitwiseOp {
    BitAnd,
    BitOr,
    BitXor,
};

enum class AssignOp {
    Assign,
    MulAssign,
    DivAssign,
    ModAssign,
    AddAssign,
    SubAssign,
    ShlAssign,
    ShrAssign,
    AndAssign,
    XorAssign,
    OrAssign,
};

// Fold vocabulary. Nodes store the narrower op types above; convert with asBinary.
enum class BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Shl,
    Shr,
    BitAnd,
    BitOr,
    BitXor,
    Lt,
    Gt,
    Le,
    Ge,
    Eq,
    Ne,
    LogAnd,
    LogOr,
};

std::optional<UnaryOp> unaryOpFromLexeme(std::string_view lexeme);
std::optional<IncDec> incDecFromLexeme(std::string_view lexeme);
std::optional<ArithmeticOp> arithmeticOpFromLexeme(std::string_view lexeme);
std::optional<ShiftOp> shiftOpFromLexeme(std::string_view lexeme);
std::optional<ComparisonOp> comparisonOpFromLexeme(std::string_view lexeme);
std::optional<BitwiseOp> bitwiseOpFromLexeme(std::string_view lexeme);
std::optional<AssignOp> assignOpFromLexeme(std::string_view lexeme);

const char* spelling(UnaryOp op);
const char* spelling(IncDec op);
const char* spelling(BinaryOp op);
const char* spelling(AssignOp op);

constexpr BinaryOp asBinary(ArithmeticOp op) {
    switch (op) {
    case ArithmeticOp::Add:
        return BinaryOp::Add;
    case ArithmeticOp::Sub:
        return BinaryOp::Sub;
    case ArithmeticOp::Mul:
        return BinaryOp::Mul;
    case ArithmeticOp::Div:
        return BinaryOp::Div;
    case ArithmeticOp::Mod:
        return BinaryOp::Mod;
    }
    return BinaryOp::Add;
}

constexpr BinaryOp asBinary(ShiftOp op) {
    switch (op) {
    case ShiftOp::Shl:
        return BinaryOp::Shl;
    case ShiftOp::Shr:
        return BinaryOp::Shr;
    }
    return BinaryOp::Shl;
}

constexpr BinaryOp asBinary(ComparisonOp op) {
    switch (op) {
    case ComparisonOp::Lt:
        return BinaryOp::Lt;
    case ComparisonOp::Gt:
        return BinaryOp::Gt;
    case ComparisonOp::Le:
        return BinaryOp::Le;
    case ComparisonOp::Ge:
        return BinaryOp::Ge;
    case ComparisonOp::Eq:
        return BinaryOp::Eq;
    case ComparisonOp::Ne:
        return BinaryOp::Ne;
    }
    return BinaryOp::Lt;
}

constexpr BinaryOp asBinary(BitwiseOp op) {
    switch (op) {
    case BitwiseOp::BitAnd:
        return BinaryOp::BitAnd;
    case BitwiseOp::BitOr:
        return BinaryOp::BitOr;
    case BitwiseOp::BitXor:
        return BinaryOp::BitXor;
    }
    return BinaryOp::BitAnd;
}

inline const char* spelling(ArithmeticOp op) {
    return spelling(asBinary(op));
}

inline const char* spelling(ShiftOp op) {
    return spelling(asBinary(op));
}

inline const char* spelling(ComparisonOp op) {
    return spelling(asBinary(op));
}

inline const char* spelling(BitwiseOp op) {
    return spelling(asBinary(op));
}

template<typename Op>
Op requireOp(std::optional<Op> parsed) {
    if (!parsed) {
        throw std::logic_error { "internal compiler error: operator lexeme is not a known operator" };
    }
    return *parsed;
}

} // namespace type

#endif
