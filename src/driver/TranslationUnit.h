#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include "scanner/TranslationUnitContext.h"

class Token;

class TranslationUnit {
public:
    TranslationUnit(const std::string sourceFileName);
    virtual ~TranslationUnit();

    virtual TranslationUnitContext getContext() const;
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
