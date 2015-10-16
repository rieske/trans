#include "translation_unit/TranslationUnit.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"

#include <stdexcept>

using namespace testing;

TEST(TranslationUnit, opensTheSourceFileForReading) {
	ASSERT_NO_THROW(TranslationUnit translationUnit(getTestResourcePath("stubResources/sourceTranslationUnitInput.txt")));
}

TEST(TranslationUnit, throwsExceptionWhenNotAbleToOpenSourceFile) {
	ASSERT_THROW(TranslationUnit translationUnit(getTestResourcePath("stubResources/nonexistant_file.aaa")), std::runtime_error);
}

TEST(TranslationUnit, returnsCharactersFromInputFile) {
	TranslationUnit translationUnit(getTestResourcePath("stubResources/sourceTranslationUnitInput.txt"));

	ASSERT_THAT(translationUnit.getContext().getOffset(), Eq(1));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('h'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq(' '));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), Eq(1));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), Eq(2));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('a'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), Eq(3));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), Eq(4));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('e'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), Eq(4));
}
