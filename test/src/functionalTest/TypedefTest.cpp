#include "TestFixtures.h"

namespace {

TEST(Compiler, typedefBasicInt) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int myint;
        int main() {
            myint x;
            x = 42;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, typedefPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int* intptr;
        int main() {
            int v;
            intptr p;
            v = 7;
            p = &v;
            *p = 9;
            printf("%d", v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, typedefInParameterAndReturn) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int num;
        num add(num a, num b) {
            return a + b;
        }
        int main() {
            printf("%d", add(20, 22));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, typedefChained) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int i32;
        typedef i32 integer;
        int main() {
            integer x;
            x = 5;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, typedefStruct) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef struct Point {
            int x;
            int y;
        } Point;
        int main() {
            Point p;
            p.x = 3;
            p.y = 4;
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

// git grep.h: typedef name reused as a struct member after `*`.
// The declarator must stay an identifier, not be re-promoted to typedef_name.
TEST(Compiler, typedefNameReusedAsPointerMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int pcre2_code;
        typedef int pcre2_match_data;
        struct grep_pat {
            pcre2_code *pcre2_pattern;
            pcre2_match_data *pcre2_match_data;
        };
        int main() {
            struct grep_pat g;
            int v;
            v = 7;
            g.pcre2_pattern = &v;
            g.pcre2_match_data = &v;
            printf("%d %d", *g.pcre2_pattern, *g.pcre2_match_data);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

// git ref-filter.c: typedef name reused as a local variable; later uses are the object.
// Shadow ends at block scope so the typedef remains usable afterward.
TEST(Compiler, typedefNameShadowedByLocalVariable) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int cmp_type;
        int foo() {
            cmp_type cmp_type;
            cmp_type = 11;
            return cmp_type;
        }
        int bar() {
            cmp_type x;
            x = 31;
            return x;
        }
        int main() {
            printf("%d %d", foo(), bar());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 31");
}

// Shadow is brace-scoped: after the inner block, the name is a typedef again.
TEST(Compiler, typedefShadowEndsAtClosingBrace) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int unit_t;
        int main() {
            int sum;
            sum = 0;
            {
                unit_t unit_t;
                unit_t = 5;
                sum = sum + unit_t;
            }
            {
                unit_t x;
                x = 7;
                sum = sum + x;
            }
            printf("%d", sum);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}


TEST(Compiler, typedefNameShadowedByParameter) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int T;
        int f(T T) {
            T = 1;
            return T;
        }
        int main() {
            printf("%d", f(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// After a definition with a shadowed parameter, the typedef remains usable.
TEST(Compiler, typedefUsableAfterParameterShadowDefinition) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int T;
        int f(T T) {
            return T + 1;
        }
        int main() {
            T x;
            x = f(40);
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("41");
}

// Prototype with a shadowed param name must not poison later typedef uses.
TEST(Compiler, typedefUsableAfterParameterShadowPrototype) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int T;
        int f(T T);
        int f(T T) {
            return T;
        }
        int main() {
            T x;
            x = f(7);
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

// After a primitive type-spec, a typedef spelling is a declarator name (`int T`),
// not a second type. Block scope object shadows the typedef for the assignment.
TEST(Compiler, typedefSpellingAsDeclaratorAfterInt) {
    SourceProgram program{R"prg(#include <stdio.h>
        typedef int T;
        int main() {
            int T;
            T = 3;
            printf("%d", T);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

} // namespace
