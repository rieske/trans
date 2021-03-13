#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <string>
#include <fstream>
#include "Context.h"

class TranslationUnit {
public:
    TranslationUnit(const std::string sourceFileName);

    translation_unit::Context getContext() const;
    char getNextCharacter();

private:
    bool advanceLine();

    const std::string fileName;
    std::ifstream sourceFile;

    std::string currentLine;
    std::size_t lineOffset { 0 };
    std::size_t currentLineNumber { 0 };
};

#endif // TRANSLATIONUNIT_H_
