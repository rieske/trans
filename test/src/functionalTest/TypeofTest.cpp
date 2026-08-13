#include "TestFixtures.h"

namespace {

TEST(Compiler, typeofC23TypeNameInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            typeof(int) x;
            x = 7;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 7");
}

TEST(Compiler, typeofC23ExpressionVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            typeof(x) y;
            x = 3;
            y = x;
            printf("%d %d %d", (int)sizeof(y), x, y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 3 3");
}

TEST(Compiler, typeofTypeNameInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            __typeof__(int) x;
            x = 7;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 7");
}

TEST(Compiler, typeofExpressionVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            __typeof__(x) y;
            x = 3;
            y = x;
            printf("%d %d %d", (int)sizeof(y), x, y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 3 3");
}

TEST(Compiler, typeofTypeNamePointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int v;
            __typeof__(int *) p;
            v = 9;
            p = &v;
            printf("%d %d", (int)sizeof(p), *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 9");
}

TEST(Compiler, typeofDereference) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int v;
            int *p;
            v = 5;
            p = &v;
            __typeof__(*p) y;
            y = *p;
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 5");
}

TEST(Compiler, typeofTypedefAlias) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef __typeof__(int) num;
        int main() {
            num x;
            x = 11;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 11");
}

TEST(Compiler, typeofNullptrTypedef) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef __typeof__(nullptr) nullptr_t;
        int main() {
            nullptr_t p;
            p = 0;
            printf("%d", (int)sizeof(p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, typeofCastUsesOperandType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            long y;
            x = 1;
            y = 99;
            y = (__typeof__(x))y;
            printf("%d %d", (int)sizeof((__typeof__(x))1), (int)y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 99");
}

TEST(Compiler, typeofDoesNotEvaluateOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 0;
            __typeof__(++n) y;
            y = 1;
            printf("%d %d", n, y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, typeofArrayAndAddressOfElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            __typeof__(a) b;
            __typeof__(&(a)[0]) p;
            b[0] = 1;
            b[1] = 2;
            b[2] = 3;
            p = &b[0];
            printf("%d %d %d", (int)sizeof(b), (int)sizeof(p), p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 8 2");
}

TEST(Compiler, typeofAfterBlockShadowUsesOuterType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int x;
        int main() {
            x = 0;
            {
                char x;
                __typeof__(x) inner;
                inner = 1;
                printf("%d ", (int)sizeof(inner));
            }
            __typeof__(x) outer;
            outer = 2;
            printf("%d", (int)sizeof(outer));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

TEST(Compiler, typeofNestedExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            __typeof__(__typeof__(x)) y;
            x = 3;
            y = x;
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 3");
}

TEST(Compiler, typeofNestedExpressionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            __typeof__(__typeof__(x) *) p;
            x = 9;
            p = &x;
            printf("%d %d", (int)sizeof(p), *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 9");
}

TEST(Compiler, typeofPointerCastFromDeref) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int v;
            int *p;
            v = 4;
            p = &v;
            int *q;
            q = (__typeof__(*p) *)&v;
            printf("%d %d", (int)sizeof(*q), *q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4");
}

TEST(Compiler, typedefTypeofVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int x;
        typedef __typeof__(x) num;
        int main() {
            num y;
            y = 6;
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 6");
}

TEST(Compiler, typedefTypeofAfterBlockShadowUsesOuterType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int x;
        int main() {
            x = 0;
            {
                char x;
                typedef __typeof__(x) Inner;
                Inner a;
                a = 1;
                printf("%d ", (int)sizeof(a));
            }
            typedef __typeof__(x) Outer;
            Outer b;
            b = 2;
            printf("%d", (int)sizeof(b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

TEST(Compiler, sizeofTypeofExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            printf("%d %d", (int)sizeof(__typeof__(x)), (int)sizeof(__typeof__(x) *));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 8");
}

TEST(Compiler, sizeofTypeofDeref) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int v;
            int *p;
            v = 1;
            p = &v;
            printf("%d", (int)sizeof(__typeof__(*p)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, typedefTypeofDeref) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int v;
        int *p;
        typedef __typeof__(*p) num;
        int main() {
            num x;
            x = 8;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 8");
}

TEST(Compiler, sizeofTypeofArithmetic) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            printf("%d", (int)sizeof(__typeof__(a + b)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, typeofUnresolvedStructMemberIsError) {
    SourceProgram program{R"prg(
        struct S {
            typeof(nope) n;
        };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("typeof");
}

TEST(Compiler, typeofPrefixThenTypedefUsesOperandType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 0;
            __typeof__(++n) y;
            typedef __typeof__(y) T;
            T z;
            y = 1;
            z = 2;
            printf("%d %d %d %d", n, y, z, (int)sizeof(z));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2 4");
}

TEST(Compiler, typeofPostfixThenTypedefUsesOperandType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 0;
            __typeof__(n++) y;
            typedef __typeof__(y) T;
            T z;
            y = 1;
            z = 2;
            printf("%d %d %d %d", n, y, z, (int)sizeof(z));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2 4");
}

TEST(Compiler, typeofArithmeticObjectDeclCompiles) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            __typeof__(a + b) x;
            x = 3;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 3");
}

TEST(Compiler, typedefTypeofAfterArithmeticObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int b;
            a = 1;
            b = 2;
            __typeof__(a + b) x;
            typedef __typeof__(x) T;
            T y;
            x = 3;
            y = 4;
            printf("%d %d %d", (int)sizeof(y), x, y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 3 4");
}

TEST(Compiler, typeofParameterThenTypedef) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int n) {
            typedef __typeof__(n) T;
            T x;
            x = n;
            printf("%d %d", (int)sizeof(x), x);
            return 0;
        }
        int main() {
            return f(7);
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 7");
}

TEST(Compiler, typeofParameterArrayDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int a[]) {
            __typeof__(a) p;
            p = a;
            printf("%d %d", (int)sizeof(p), p[0]);
            return 0;
        }
        int main() {
            int v[1];
            v[0] = 9;
            return f(v);
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 9");
}

TEST(Compiler, typeofLaterParameterUsesEarlierParameter) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int f(int n, __typeof__(n) m) {
            printf("%d %d %d", (int)sizeof(m), n, m);
            return 0;
        }
        int main() {
            return f(1, 2);
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 1 2");
}

TEST(Compiler, typeofParameterPrototypeDoesNotLeak) {
    SourceProgram program{R"prg(
        void f(int n);
        typedef __typeof__(n) T;
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("typeof");
}

TEST(Compiler, typeofGnuCastMacro) {
    SourceProgram program{R"prg(int printf(const char *, ...);
#define TYPEOF(x) (__typeof__(x))
        int main() {
            int x;
            long y;
            x = 0;
            y = 5;
            y = TYPEOF(x)(y);
            printf("%d", (int)y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, typeofMemberDot) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; char c; };
        int main() {
            struct S s;
            s.x = 11;
            __typeof__(s.x) y;
            y = s.x;
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 11");
}

TEST(Compiler, typeofMemberArrow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S s;
            struct S *p;
            s.x = 13;
            p = &s;
            __typeof__(p->x) y;
            y = p->x;
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 13");
}

TEST(Compiler, typeofMemberArrowFromArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S a[2];
            a[0].x = 19;
            __typeof__(a->x) y;
            y = a->x;
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 19");
}

TEST(Compiler, typeofArrayMemberPlus) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        struct O { struct S items[2]; };
        int main() {
            struct O o;
            o.items[0].x = 11;
            o.items[1].x = 22;
            __typeof__(o.items + 1) p;
            p = o.items + 1;
            printf("%d %d", (int)sizeof(*p), p->x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 22");
}

TEST(Compiler, typeofDerefMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct List { int *items; };
        int main() {
            int v;
            struct List list;
            struct List *p;
            v = 17;
            list.items = &v;
            p = &list;
            __typeof__(*(p->items)) y;
            y = *(p->items);
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 17");
}

TEST(Compiler, typeofDerefPointerPlus) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a;
            __typeof__(*(p + 1)) y;
            y = *(p + 1);
            printf("%d %d", (int)sizeof(y), y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 2");
}

TEST(Compiler, sizeofTypeofMemberAndPtrArith) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S s;
            struct S *p;
            int *q;
            int i;
            s.x = 0;
            p = &s;
            q = &s.x;
            i = 0;
            printf("%d %d",
                (int)sizeof(__typeof__(p->x)),
                (int)sizeof(__typeof__(*(q + i))));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4");
}

TEST(Compiler, typeofBarfUnlessCopyableMoveArrayShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Item { int v; };
        struct List { struct Item *items; int nr; };
#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)
#define BARF_UNLESS_COPYABLE(dst, src) \
    BUILD_ASSERT_OR_ZERO(__builtin_types_compatible_p( \
        __typeof__(*(dst)), __typeof__(*(src))))
#define MOVE_ARRAY(dst, src, n) \
    (void)(sizeof(*(dst)) + BARF_UNLESS_COPYABLE((dst), (src)) + (n))
        int main() {
            struct Item buf[4];
            struct List list;
            int index;
            list.items = buf;
            list.nr = 2;
            index = 0;
            MOVE_ARRAY(list.items + index + 1, list.items + index, list.nr - index);
            printf("%d", (int)sizeof(__typeof__(*(list.items))));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

// git BARF_UNLESS_UNSIGNED / BARF_UNLESS_SIGNED: ((typeof(v))-1) folded in
// BUILD_ASSERT_OR_ZERO's sizeof(char[N]). Must apply the typeof cast, not throw.
void expectTypeofMinusOneBuildAssert(const char* decl, const char* relOp) {
    SCOPED_TRACE(decl);
    std::string src = std::string("int printf(const char *, ...);\n        int main() {\n            ")
            + decl + ";\n            printf(\"%d\", (int)sizeof(char[1 - 2*!(((__typeof__(v)) -1) "
            + relOp + " 0)]));\n            return 0;\n        }\n";
    SourceProgram program { src };
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, typeofCastMinusOneBuildAssert) {
    expectTypeofMinusOneBuildAssert("unsigned v", ">");
    expectTypeofMinusOneBuildAssert("int v", "<");
    expectTypeofMinusOneBuildAssert("unsigned short v", ">");
    expectTypeofMinusOneBuildAssert("unsigned long v", ">");
}

} // namespace
