#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <string>
#include <string_view>
#include "Context.h"

class TranslationUnit {
public:
    TranslationUnit(const std::string sourceFileName);

    translation_unit::Context getContext() const { return context_; }
    char getNextCharacter();

private:
    bool advanceLine();

    std::string source;
    std::size_t readPos { 0 };
    std::string_view currentLine;
    std::size_t lineOffset { 0 };
    translation_unit::Context context_;
};

#endif // TRANSLATIONUNIT_H_
