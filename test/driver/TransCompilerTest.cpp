#include "driver/TransCompiler.h"
#include "driver/TranslationUnit.h"
#include "semantic_analyzer/SemanticComponentsFactory.h"
#include "semantic_analyzer/SyntaxTreeBuilder.h"
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
	MOCK_METHOD2(parse, int(TranslationUnit&, SyntaxTreeBuilder&));
	MOCK_CONST_METHOD0(getSyntaxTree, SyntaxTree*());
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
	std::unique_ptr<MockSemanticComponentsFactory> semanticComponentsFactory { new StrictMock<MockSemanticComponentsFactory> };
	StrictMock<MockSyntaxTreeBuilder>* syntaxTreeBuilder { new StrictMock<MockSyntaxTreeBuilder> };
	ON_CALL(*semanticComponentsFactory, newSyntaxTreeBuilder()).WillByDefault(Return(syntaxTreeBuilder));
	EXPECT_CALL(*semanticComponentsFactory, newSyntaxTreeBuilder());
	EXPECT_CALL(*parser, parse(Ref(translationUnit), Ref(*syntaxTreeBuilder)));
	EXPECT_CALL(*parser, getSyntaxTree());
	TransCompiler compiler(std::move(parser), std::move(semanticComponentsFactory));

	compiler.compile(translationUnit);
}
