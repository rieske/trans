#include "parser/BNFFileReader.h"
#include "parser/CompileParsingTable.h"
#include "parser/GenerateParsingTable.h"

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: parsingTableGenerator <grammar.bnf> <output.cpp>\n";
        return 2;
    }
    try {
        parser::BNFFileReader reader;
        parser::Grammar grammar = reader.readGrammar(argv[1]);
        parser::writeParsingTableSource(parser::generateParsingTable(&grammar), argv[2]);
    } catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << "\n";
        return 1;
    }
    return 0;
}
