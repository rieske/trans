#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include "Context.h"

class Token;

class TranslationUnit {
public:
    TranslationUnit(const std::string sourceFileName);
    virtual ~TranslationUnit();

    virtual translation_unit::Context getContext() const;
    virtual char getNextCharacter();

private:
    bool advanceLine();

    const std::string fileName;
    std::ifstream sourceFile;

    std::string currentLine;
    std::size_t lineOffset { 0 };
    std::size_t currentLineNumber { 0 };
};

#endif /* TRANSLATIONUNIT_H_ */
