#include "TestFixtures.h"

#include <string>

namespace {

// Completeness contract for parse-time typeof: one row per expression production
// (and the declaration contexts that must resolve it). Existing TypeofTest.cpp
// keeps git-macro shapes; this file is the grammar surface.

struct TypeofSizeCase {
    const char *name;
    const char *prelude;
    const char *inMain;
    const char *expr;
    const char *expected;
};

class TypeofSizeCatalog : public testing::TestWithParam<TypeofSizeCase> {};

std::string sizeofProgram(const TypeofSizeCase &c) {
    std::string src = "int printf(const char *, ...);\n";
    src += c.prelude;
    src += "int main() {\n";
    src += c.inMain;
    src += "    printf(\"%d\", (int)sizeof(typeof(";
    src += c.expr;
    src += ")));\n    return 0;\n}\n";
    return src;
}

TEST_P(TypeofSizeCatalog, SizeofMatches) {
    const TypeofSizeCase &c = GetParam();
    SourceProgram program{sizeofProgram(c)};
    program.compile();
    program.runAndExpect(c.expected);
}

INSTANTIATE_TEST_SUITE_P(Compiler, TypeofSizeCatalog, testing::Values(
    TypeofSizeCase{"intConstant", "", "", "1", "4"},
    TypeofSizeCase{"charConstantIsInt", "", "", "'a'", "4"},
    TypeofSizeCase{"floatConstant", "", "", "1.0f", "4"},
    TypeofSizeCase{"doubleConstant", "", "", "1.0", "8"},
    TypeofSizeCase{"boolConstant", "", "", "true", "1"},
    TypeofSizeCase{"nullptrConstant", "", "", "nullptr", "8"},
    TypeofSizeCase{"stringLiteralIsArray", "", "", "\":\"", "2"},
    TypeofSizeCase{"stringConcatIsArray", "", "", "\"a\" \"bc\"", "4"},
    TypeofSizeCase{"parenStringIsArray", "", "", "(\":\")", "2"},
    TypeofSizeCase{"identifier", "", "int x; x = 0;\n", "x", "4"},
    TypeofSizeCase{"arrayDoesNotDecay", "", "int a[3]; a[0] = 0;\n", "a", "12"},
    TypeofSizeCase{"subscript", "", "int a[3]; a[0] = 0;\n", "a[0]", "4"},
    TypeofSizeCase{"memberDot",
            "struct S { long m; };\n",
            "struct S s; s.m = 0;\n",
            "s.m", "8"},
    TypeofSizeCase{"memberArrow",
            "struct S { int m; };\n",
            "struct S s; struct S *p; s.m = 0; p = &s;\n",
            "p->m", "4"},
    TypeofSizeCase{"call", "int f(void);\n", "", "f()", "4"},
    TypeofSizeCase{"postfixInc", "", "int x; x = 0;\n", "x++", "4"},
    TypeofSizeCase{"prefixDec", "", "int x; x = 1;\n", "--x", "4"},
    TypeofSizeCase{"addressOf", "", "int x; x = 0;\n", "&x", "8"},
    TypeofSizeCase{"dereference", "", "int x; int *p; x = 0; p = &x;\n", "*p", "4"},
    TypeofSizeCase{"unaryPlus", "", "int x; x = 1;\n", "+x", "4"},
    TypeofSizeCase{"unaryPlusPromotesChar", "", "char c; c = 1;\n", "+c", "4"},
    TypeofSizeCase{"unaryMinus", "", "int x; x = 1;\n", "-x", "4"},
    TypeofSizeCase{"bitwiseNot", "", "int x; x = 1;\n", "~x", "4"},
    TypeofSizeCase{"logicalNot", "", "int x; x = 1;\n", "!x", "4"},
    TypeofSizeCase{"sizeofExpr", "", "int x; x = 0;\n", "sizeof x", "4"},
    TypeofSizeCase{"cast", "", "int x; x = 0;\n", "(long)x", "8"},
    TypeofSizeCase{"compoundLiteralScalar", "", "", "(int){1}", "4"},
    TypeofSizeCase{"compoundLiteralStruct",
            "struct S { int a; };\n",
            "",
            "(struct S){0}", "4"},
    TypeofSizeCase{"mul", "", "int x; int y; x = 2; y = 3;\n", "x * y", "4"},
    TypeofSizeCase{"add", "", "int x; int y; x = 2; y = 3;\n", "x + y", "4"},
    TypeofSizeCase{"pointerAdd", "", "int x; int *p; x = 0; p = &x;\n", "p + 1", "8"},
    TypeofSizeCase{"shift", "", "int x; x = 1;\n", "x << 1", "4"},
    TypeofSizeCase{"shiftPromotesChar", "", "char c; c = 1;\n", "c << 1", "4"},
    TypeofSizeCase{"less", "", "int x; int y; x = 1; y = 2;\n", "x < y", "4"},
    TypeofSizeCase{"equal", "", "int x; int y; x = 1; y = 1;\n", "x == y", "4"},
    TypeofSizeCase{"bitand", "", "int x; int y; x = 1; y = 3;\n", "x & y", "4"},
    TypeofSizeCase{"bitxor", "", "int x; int y; x = 1; y = 3;\n", "x ^ y", "4"},
    TypeofSizeCase{"bitor", "", "int x; int y; x = 1; y = 3;\n", "x | y", "4"},
    TypeofSizeCase{"logicalAnd", "", "int x; int y; x = 1; y = 0;\n", "x && y", "4"},
    TypeofSizeCase{"logicalOr", "", "int x; int y; x = 1; y = 0;\n", "x || y", "4"},
    TypeofSizeCase{"conditional", "", "int x; int y; x = 1; y = 2;\n", "x ? x : y", "4"},
    TypeofSizeCase{"conditionalPointer",
            "",
            "int x; int y; int *p; int *q; x = 1; p = &x; q = &y;\n",
            "x ? p : q", "8"},
    TypeofSizeCase{"assign", "", "int x; int y; x = 0; y = 1;\n", "x = y", "4"},
    TypeofSizeCase{"compoundAssign", "", "int x; int y; x = 1; y = 2;\n", "x += y", "4"},
    TypeofSizeCase{"comma", "", "int x; long y; x = 0; y = 0;\n", "(x, y)", "8"},
    TypeofSizeCase{"genericSelectedArm", "", "", "_Generic(0, int: 1.0, default: 0)", "8"},
    TypeofSizeCase{"statementExpression", "", "", "({ int z; z = 1; z; })", "4"},
    TypeofSizeCase{"statementExpressionOuterLocal", "", "long y; y = 0;\n", "({ y; })", "8"},
    TypeofSizeCase{"nestedStatementExpression", "", "",
            "({ int z; z = 1; ({ int w; w = z; w; }); })", "4"}
), [](const testing::TestParamInfo<TypeofSizeCase> &info) {
    return std::string{info.param.name};
});

struct TypeofCompatCase {
    const char *name;
    const char *prelude;
    const char *inMain;
    const char *left;
    const char *right;
    const char *expected;
};

class TypeofCompatCatalog : public testing::TestWithParam<TypeofCompatCase> {};

std::string compatProgram(const TypeofCompatCase &c) {
    std::string src = "int printf(const char *, ...);\n";
    src += c.prelude;
    src += "int main() {\n";
    src += c.inMain;
    src += "    printf(\"%d\", __builtin_types_compatible_p(typeof(";
    src += c.left;
    src += "), ";
    src += c.right;
    src += "));\n    return 0;\n}\n";
    return src;
}

TEST_P(TypeofCompatCatalog, CompatibleMatches) {
    const TypeofCompatCase &c = GetParam();
    SourceProgram program{compatProgram(c)};
    program.compile();
    program.runAndExpect(c.expected);
}

INSTANTIATE_TEST_SUITE_P(Compiler, TypeofCompatCatalog, testing::Values(
    TypeofCompatCase{"stringIsCharArray", "", "", "\":\"", "char[2]", "1"},
    TypeofCompatCase{"stringIsNotCharPointer", "", "", "\":\"", "char *", "0"},
    TypeofCompatCase{"stringIsNotConstCharPointer", "", "", "\":\"", "const char *", "0"},
    TypeofCompatCase{"arrayIsNotPointer", "", "int a[3]; a[0] = 0;\n", "a", "int *", "0"},
    TypeofCompatCase{"functionVsPointerToSame",
            "int f(void);\n",
            "",
            "f", "int (*)(void)", "0"}
), [](const testing::TestParamInfo<TypeofCompatCase> &info) {
    return std::string{info.param.name};
});

TEST(Compiler, typeofObjectDeclarationUsesOperandType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            typeof(1 + 2) x;
            x = 7;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 7");
}

TEST(Compiler, typeofTypedefUsesOperandType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef typeof(1.0) D;
        int main() {
            D x;
            x = 1.0;
            printf("%d", (int)sizeof(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, typeofPointerToFunctionDesignator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(void);
        int main() {
            printf("%d %d",
                (int)sizeof(typeof(f) *),
                __builtin_types_compatible_p(typeof(f) *, int (*)(void)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1");
}

TEST(Compiler, typeofAbstractPointerToStringArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            typeof(":") *p;
            printf("%d %d", (int)sizeof(p), (int)sizeof(*p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 2");
}

TEST(Compiler, typeofUnknownIdentifierIsError) {
    SourceProgram program{R"prg(
        int main() {
            return (int)sizeof(typeof(nope));
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("cannot determine type of typeof operand");
}

} // namespace
