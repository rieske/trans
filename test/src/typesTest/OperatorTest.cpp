#include "gtest/gtest.h"

#include "types/Operator.h"

namespace {

TEST(Operator, unaryOpFromLexeme) {
    EXPECT_EQ(type::unaryOpFromLexeme("+"), type::UnaryOp::Plus);
    EXPECT_EQ(type::unaryOpFromLexeme("-"), type::UnaryOp::Minus);
    EXPECT_EQ(type::unaryOpFromLexeme("!"), type::UnaryOp::LogicalNot);
    EXPECT_EQ(type::unaryOpFromLexeme("~"), type::UnaryOp::BitNot);
    EXPECT_EQ(type::unaryOpFromLexeme("*"), type::UnaryOp::Deref);
    EXPECT_EQ(type::unaryOpFromLexeme("&"), type::UnaryOp::Addr);
    EXPECT_EQ(type::unaryOpFromLexeme("sizeof"), type::UnaryOp::Sizeof);
    EXPECT_FALSE(type::unaryOpFromLexeme("++").has_value());
    EXPECT_FALSE(type::unaryOpFromLexeme("--").has_value());
    EXPECT_FALSE(type::unaryOpFromLexeme("").has_value());
    EXPECT_FALSE(type::unaryOpFromLexeme("??").has_value());
}

TEST(Operator, incDecFromLexeme) {
    EXPECT_EQ(type::incDecFromLexeme("++"), type::IncDec::Inc);
    EXPECT_EQ(type::incDecFromLexeme("--"), type::IncDec::Dec);
    EXPECT_FALSE(type::incDecFromLexeme("+").has_value());
    EXPECT_FALSE(type::incDecFromLexeme("sizeof").has_value());
}

TEST(Operator, arithmeticOpFromLexeme) {
    EXPECT_EQ(type::arithmeticOpFromLexeme("+"), type::ArithmeticOp::Add);
    EXPECT_EQ(type::arithmeticOpFromLexeme("-"), type::ArithmeticOp::Sub);
    EXPECT_EQ(type::arithmeticOpFromLexeme("*"), type::ArithmeticOp::Mul);
    EXPECT_EQ(type::arithmeticOpFromLexeme("/"), type::ArithmeticOp::Div);
    EXPECT_EQ(type::arithmeticOpFromLexeme("%"), type::ArithmeticOp::Mod);
    EXPECT_FALSE(type::arithmeticOpFromLexeme("<<").has_value());
    EXPECT_FALSE(type::arithmeticOpFromLexeme("+=").has_value());
}

TEST(Operator, shiftOpFromLexeme) {
    EXPECT_EQ(type::shiftOpFromLexeme("<<"), type::ShiftOp::Shl);
    EXPECT_EQ(type::shiftOpFromLexeme(">>"), type::ShiftOp::Shr);
    EXPECT_FALSE(type::shiftOpFromLexeme("<").has_value());
    EXPECT_FALSE(type::shiftOpFromLexeme("+").has_value());
}

TEST(Operator, comparisonOpFromLexeme) {
    EXPECT_EQ(type::comparisonOpFromLexeme("<"), type::ComparisonOp::Lt);
    EXPECT_EQ(type::comparisonOpFromLexeme(">"), type::ComparisonOp::Gt);
    EXPECT_EQ(type::comparisonOpFromLexeme("<="), type::ComparisonOp::Le);
    EXPECT_EQ(type::comparisonOpFromLexeme(">="), type::ComparisonOp::Ge);
    EXPECT_EQ(type::comparisonOpFromLexeme("=="), type::ComparisonOp::Eq);
    EXPECT_EQ(type::comparisonOpFromLexeme("!="), type::ComparisonOp::Ne);
    EXPECT_FALSE(type::comparisonOpFromLexeme("+").has_value());
    EXPECT_FALSE(type::comparisonOpFromLexeme("<<").has_value());
}

TEST(Operator, bitwiseOpFromLexeme) {
    EXPECT_EQ(type::bitwiseOpFromLexeme("&"), type::BitwiseOp::BitAnd);
    EXPECT_EQ(type::bitwiseOpFromLexeme("|"), type::BitwiseOp::BitOr);
    EXPECT_EQ(type::bitwiseOpFromLexeme("^"), type::BitwiseOp::BitXor);
    EXPECT_FALSE(type::bitwiseOpFromLexeme("&&").has_value());
    EXPECT_FALSE(type::bitwiseOpFromLexeme("+").has_value());
}

TEST(Operator, assignOpFromLexeme) {
    EXPECT_EQ(type::assignOpFromLexeme("="), type::AssignOp::Assign);
    EXPECT_EQ(type::assignOpFromLexeme("*="), type::AssignOp::MulAssign);
    EXPECT_EQ(type::assignOpFromLexeme("/="), type::AssignOp::DivAssign);
    EXPECT_EQ(type::assignOpFromLexeme("%="), type::AssignOp::ModAssign);
    EXPECT_EQ(type::assignOpFromLexeme("+="), type::AssignOp::AddAssign);
    EXPECT_EQ(type::assignOpFromLexeme("-="), type::AssignOp::SubAssign);
    EXPECT_EQ(type::assignOpFromLexeme("<<="), type::AssignOp::ShlAssign);
    EXPECT_EQ(type::assignOpFromLexeme(">>="), type::AssignOp::ShrAssign);
    EXPECT_EQ(type::assignOpFromLexeme("&="), type::AssignOp::AndAssign);
    EXPECT_EQ(type::assignOpFromLexeme("^="), type::AssignOp::XorAssign);
    EXPECT_EQ(type::assignOpFromLexeme("|="), type::AssignOp::OrAssign);
    EXPECT_FALSE(type::assignOpFromLexeme("+").has_value());
    EXPECT_FALSE(type::assignOpFromLexeme("==").has_value());
    EXPECT_FALSE(type::assignOpFromLexeme("").has_value());
}

TEST(Operator, spellingRoundTripsUnary) {
    const type::UnaryOp ops[] = {
            type::UnaryOp::Plus,
            type::UnaryOp::Minus,
            type::UnaryOp::LogicalNot,
            type::UnaryOp::BitNot,
            type::UnaryOp::Deref,
            type::UnaryOp::Addr,
            type::UnaryOp::Sizeof,
    };
    for (type::UnaryOp op : ops) {
        EXPECT_EQ(type::unaryOpFromLexeme(type::spelling(op)), op);
    }
}

TEST(Operator, spellingRoundTripsIncDec) {
    EXPECT_EQ(type::incDecFromLexeme(type::spelling(type::IncDec::Inc)), type::IncDec::Inc);
    EXPECT_EQ(type::incDecFromLexeme(type::spelling(type::IncDec::Dec)), type::IncDec::Dec);
}

TEST(Operator, spellingRoundTripsArithmetic) {
    const type::ArithmeticOp ops[] = {
            type::ArithmeticOp::Add,
            type::ArithmeticOp::Sub,
            type::ArithmeticOp::Mul,
            type::ArithmeticOp::Div,
            type::ArithmeticOp::Mod,
    };
    for (type::ArithmeticOp op : ops) {
        EXPECT_EQ(type::arithmeticOpFromLexeme(type::spelling(op)), op);
        EXPECT_STREQ(type::spelling(op), type::spelling(type::asBinary(op)));
    }
}

TEST(Operator, spellingRoundTripsShift) {
    EXPECT_EQ(type::shiftOpFromLexeme(type::spelling(type::ShiftOp::Shl)), type::ShiftOp::Shl);
    EXPECT_EQ(type::shiftOpFromLexeme(type::spelling(type::ShiftOp::Shr)), type::ShiftOp::Shr);
    EXPECT_STREQ(type::spelling(type::ShiftOp::Shl), type::spelling(type::asBinary(type::ShiftOp::Shl)));
}

TEST(Operator, spellingRoundTripsComparison) {
    const type::ComparisonOp ops[] = {
            type::ComparisonOp::Lt,
            type::ComparisonOp::Gt,
            type::ComparisonOp::Le,
            type::ComparisonOp::Ge,
            type::ComparisonOp::Eq,
            type::ComparisonOp::Ne,
    };
    for (type::ComparisonOp op : ops) {
        EXPECT_EQ(type::comparisonOpFromLexeme(type::spelling(op)), op);
        EXPECT_STREQ(type::spelling(op), type::spelling(type::asBinary(op)));
    }
}

TEST(Operator, spellingRoundTripsBitwise) {
    const type::BitwiseOp ops[] = {
            type::BitwiseOp::BitAnd,
            type::BitwiseOp::BitOr,
            type::BitwiseOp::BitXor,
    };
    for (type::BitwiseOp op : ops) {
        EXPECT_EQ(type::bitwiseOpFromLexeme(type::spelling(op)), op);
        EXPECT_STREQ(type::spelling(op), type::spelling(type::asBinary(op)));
    }
}

TEST(Operator, spellingRoundTripsAssign) {
    const type::AssignOp ops[] = {
            type::AssignOp::Assign,
            type::AssignOp::MulAssign,
            type::AssignOp::DivAssign,
            type::AssignOp::ModAssign,
            type::AssignOp::AddAssign,
            type::AssignOp::SubAssign,
            type::AssignOp::ShlAssign,
            type::AssignOp::ShrAssign,
            type::AssignOp::AndAssign,
            type::AssignOp::XorAssign,
            type::AssignOp::OrAssign,
    };
    for (type::AssignOp op : ops) {
        EXPECT_EQ(type::assignOpFromLexeme(type::spelling(op)), op);
    }
}

TEST(Operator, requireOpReturnsParsed) {
    EXPECT_EQ(type::requireOp(type::unaryOpFromLexeme("-")), type::UnaryOp::Minus);
    EXPECT_EQ(type::requireOp(type::incDecFromLexeme("++")), type::IncDec::Inc);
}

} // namespace
