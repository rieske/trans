#include "translation_unit/TranslationUnit.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ResourceHelpers.h"

#include <stdexcept>
#include <string>

using namespace testing;

TEST(TranslationUnit, opensTheSourceFileForReading) {
	ASSERT_NO_THROW(TranslationUnit translationUnit(getTestResourcePath("src/translation_unitTest/sourceTranslationUnitInput.txt")));
}

TEST(TranslationUnit, throwsExceptionWhenNotAbleToOpenSourceFile) {
	ASSERT_THROW(TranslationUnit translationUnit("nonexistant_file.aaa"), std::runtime_error);
}

TEST(TranslationUnit, returnsCharactersFromInputFile) {
	TranslationUnit translationUnit(getTestResourcePath("src/translation_unitTest/sourceTranslationUnitInput.txt"));

	ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(1));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('h'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq(' '));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(1));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(2));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('a'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(3));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\n'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(4));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('e'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('s'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
	ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
	ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(4));
}

TEST(TranslationUnit, skipsPragmaAndIndentedHashLines) {
    const std::string path = writeTempSource("tu_hash_skip",
            "#pragma once\n"
            "\t# indent-hash\n"
            "int x;\n");
    TranslationUnit translationUnit(path);

    ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(3));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('i'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('n'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('t'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq(' '));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('x'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq(';'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
}

TEST(TranslationUnit, allHashLinesYieldImmediateEof) {
    const std::string path = writeTempSource("tu_all_hash",
            "#pragma once\n"
            "#define FOO 1\n");
    TranslationUnit translationUnit(path);

    ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(2));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
}

TEST(TranslationUnit, allIndentedHashLinesYieldImmediateEof) {
    const std::string path = writeTempSource("tu_all_indent_hash",
            "\t# pragma\n"
            "  # indent\n");
    TranslationUnit translationUnit(path);

    ASSERT_THAT(translationUnit.getContext().getOffset(), TypedEq<std::size_t>(2));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
    ASSERT_THAT(translationUnit.getNextCharacter(), Eq('\0'));
}
