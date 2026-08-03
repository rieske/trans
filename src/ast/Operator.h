#ifndef OPERATOR_H_
#define OPERATOR_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

// Discriminator for Operator, set from the source lexeme at construction.
// Unary and binary + / - share Add / Sub; unary * / & use Deref / AddressOf
// (binary * / & use Mul / BitAnd) via makeUnary vs default (binary) construction.
enum class OperatorKind {
    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    // Shift
    Shl,
    Shr,
    // Bitwise
    BitAnd,
    BitOr,
    BitXor,
    // Relational / equality
    Lt,
    Gt,
    Le,
    Ge,
    Eq,
    Ne,
    // Logical
    LogicalAnd,
    LogicalOr,
    LogicalNot,
    // Unary
    AddressOf,
    Deref,
    BitNot,
    Sizeof,
    // Inc / dec
    Inc,
    Dec,
    // Assignment
    Assign,
    AddAssign,
    SubAssign,
    MulAssign,
    DivAssign,
    ModAssign,
    AndAssign,
    OrAssign,
    XorAssign,
    ShlAssign,
    ShrAssign,
    // Structural / other
    Call,       // ()
    Index,      // []
    Comma,
    Cast,       // type-name placeholder on TypeCast
    Unknown
};

// Binary / assignment / relational mapping (also used by Operator(string)).
OperatorKind binaryOperatorKindFromLexeme(const std::string& lexeme);
// Unary / prefix / postfix mapping (* → Deref, & → AddressOf, ++, sizeof, ...).
OperatorKind unaryOperatorKindFromLexeme(const std::string& lexeme);

// For compound assign (+= ...), the corresponding binary op. Unknown if not compound.
OperatorKind compoundAssignBase(OperatorKind kind);
bool isCompoundAssign(OperatorKind kind);

class Operator: public AbstractSyntaxTreeNode {
public:
    // Binary / assignment / comparison / logical / structural operators.
    explicit Operator(std::string lexeme);
    // Unary / prefix / postfix operators (* & + - ! ~ ++ -- sizeof).
    static std::unique_ptr<Operator> makeUnary(std::string lexeme);

    virtual ~Operator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::string getLexeme() const;
    OperatorKind getKind() const { return kind; }

private:
    Operator(std::string lexeme, OperatorKind kind);

    std::string lexeme;
    OperatorKind kind { OperatorKind::Unknown };
};

} // namespace ast

#endif // OPERATOR_H_
