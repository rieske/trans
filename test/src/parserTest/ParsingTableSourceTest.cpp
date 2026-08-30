#include "parser/CompileParsingTable.h"
#include "parser/GenerateParsingTable.h"
#include "parser/GrammarBuilder.h"
#include "parser/ParsingTable.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"

#include <fstream>
#include <iterator>
#include <string>

using namespace parser;
using testing::HasSubstr;

namespace {

Grammar expressionGrammar() {
    GrammarBuilder builder;
    builder.defineRule("<expr>", {"<term>", "+", "<expr>"});
    builder.defineRule("<expr>", {"<term>"});
    builder.defineRule("<term>", {"<factor>", "*", "<term>"});
    builder.defineRule("<term>", {"<factor>"});
    builder.defineRule("<factor>", {"(", "<expr>", ")"});
    builder.defineRule("<factor>", {"<operand>"});
    builder.defineRule("<operand>", {"identifier"});
    builder.defineRule("<operand>", {"constant"});
    return builder.build();
}

} // namespace

TEST(ParsingTableSource, writesProductTableFromGeneratedTable) {
    Grammar grammar = expressionGrammar();
    ParsingTable table = generateParsingTable(&grammar, AutomatonKind::LALR1);

    const std::string path = getTestResourcePath("programs/tmp/generated_parsing_table.cpp");
    ScopedTempFile sourceFile { path };
    writeParsingTableSource(table, path);

    std::ifstream sourceStream { path };
    const std::string source { std::istreambuf_iterator<char>(sourceStream),
            std::istreambuf_iterator<char>() };
    EXPECT_THAT(source, HasSubstr("loadProduct"));
    EXPECT_THAT(source, HasSubstr("ParsingTable"));
    EXPECT_THAT(source, HasSubstr(std::to_string(table.stateCount()) + ";"));
    EXPECT_THAT(source, HasSubstr(std::to_string(grammar.ruleCount()) + ";"));
}
