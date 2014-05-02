#include "driver/TransCompiler.h"
#include "driver/TranslationUnit.h"
#include "semantic_analyzer/SemanticComponentsFactory.h"
#include "semantic_analyzer/SyntaxTreeBuilder.h"
#include "semantic_analyzer/SyntaxTree.h"
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
	std::unique_ptr<SyntaxTree> parse(TranslationUnit& translationUnit) {
		return std::unique_ptr<SyntaxTree> { parseProxy(translationUnit) };
	}

	MOCK_METHOD1(parseProxy, SyntaxTree*(TranslationUnit&));
};

class MockSemanticComponentsFactory: public SemanticComponentsFactory {
public:
	MOCK_CONST_METHOD0(newSyntaxTreeBuilder, SyntaxTreeBuilder*());
};

class MockSyntaxTreeBuilder: public SyntaxTreeBuilder {
public:
};

TEST(TransCompiler, invokesParserOnTranslationUnit) {

	StrictMock<MockTranslationUnit> translationUnit;
	EXPECT_CALL(translationUnit, getFileName());

	std::unique_ptr<MockParser> parser { new StrictMock<MockParser> };

	EXPECT_CALL(*parser, parseProxy(Ref(translationUnit)));
	TransCompiler compiler(std::move(parser));

	compiler.compile(translationUnit);
}
