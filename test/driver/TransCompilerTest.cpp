#include "driver/TransCompiler.h"
#include "driver/TranslationUnit.h"
#include "parser/Parser.h"
#include "scanner/Token.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

class MockTranslationUnit: public TranslationUnit {
public:
	MOCK_CONST_METHOD0(getFileName, std::string());
	MOCK_METHOD0(getNextToken, Token());
	MOCK_METHOD0(getNextCharacter, char ());
};

class MockParser: public Parser {
public:
	MOCK_METHOD1(parse, int(TranslationUnit&));
	MOCK_CONST_METHOD0(getSyntaxTree, SyntaxTree*());
};

TEST(TransCompiler, invokesParserOnTranslationUnit) {

	StrictMock<MockTranslationUnit> translationUnit;
	EXPECT_CALL(translationUnit, getFileName());

	std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };
	EXPECT_CALL(*parser, parse(Ref(translationUnit)));
	EXPECT_CALL(*parser, getSyntaxTree());
	TransCompiler compiler(std::move(parser));

	compiler.compile(translationUnit);
}
