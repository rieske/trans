#include "driver/SourceTranslationUnit.h"
#include "scanner/Scanner.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <stdexcept>

using namespace testing;

class MockScanner: public Scanner {
public:
	MOCK_METHOD1(scan, Token(TranslationUnit& translationUnit));
};

TEST(SourceTranslationUnit, opensTheSourceFileForReading) {
	StrictMock<MockScanner> scanner;

	ASSERT_NO_THROW(SourceTranslationUnit translationUnit("test/stubResources/sourceTranslationUnitInput.txt", scanner));
}

TEST(SourceTranslationUnit, throwsExceptionWhenNotAbleToOpenSourceFile) {
	StrictMock<MockScanner> scanner;

	ASSERT_THROW(SourceTranslationUnit translationUnit("test/stubResources/nonexistant_file.aaa", scanner), std::runtime_error);
}

TEST(SourceTranslationUnit, usesScannerToGetNextToken) {
	StrictMock<MockScanner> scanner;
	SourceTranslationUnit translationUnit("test/programs/example_prog.src", scanner);

	Token expectedToken { 0, "expected" };
	EXPECT_CALL(scanner, scan(Ref(translationUnit))).WillOnce(Return(expectedToken));

	Token token = translationUnit.getNextToken();

	ASSERT_THAT(token.getLexeme(), Eq(expectedToken.getLexeme()));
}

TEST(SourceTranslationUnit, returnsCharactersFromInputFile) {
	StrictMock<MockScanner> scanner;
	SourceTranslationUnit translationUnit("test/stubResources/sourceTranslationUnitInput.txt", scanner);

	ASSERT_THAT(translationUnit.getCurrentLineNumber(), Eq(1));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('h'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq(' '));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getCurrentLineNumber(), Eq(1));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getCurrentLineNumber(), Eq(2));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('a'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getCurrentLineNumber(), Eq(3));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getCurrentLineNumber(), Eq(4));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('e'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
	ASSERT_THAT(translationUnit.getCurrentLineNumber(), Eq(4));
}
