#include <iostream>
#include <stdio.h>
#include <string>

#include "scanner/token.h"
#include "scanner/scanner.h"
#include "parser/parser.h"
#include "util/TransConfiguration.h"
#include "code_generator/code_generator.h"

int main(int argc, char **argv)
{
    // suformuojam helpą
    std::string help = "Usage: ";
    help += argv[0];
    help +=    " [options] source_file\nOptions:\n";
    help +=    " -h\t\t\tDisplay this information\n";

    TransConfiguration *transConfiguration = new TransConfiguration(argc, argv);
    int srcfiles = transConfiguration->getSourceFileNames().size();
    if (transConfiguration->isScannerLoggingEnabled()) {
    	Scanner::set_logging("scanner.log");
    }
    if (transConfiguration->isParserLoggingEnabled()) {
    	Parser::set_logging("parser.log");
    }
    while (argc-- > srcfiles)
        *argv++;
    std::cout << "Compiling " << *argv << "...\n";

    while (srcfiles--)
    {
        Parser *parser = NULL;
        if (!transConfiguration->getCustomGrammarFileName().empty())
            parser = new Parser(new std::string(transConfiguration->getCustomGrammarFileName()));
        else
            parser = new Parser();
        if ( 0 == parser->parse(*argv) )
        {
            SyntaxTree *tree = parser->getSyntaxTree();
            if (tree != NULL && tree->getErrorFlag() == false)
            {   // sėkminga semantinė analizė
                std::cout << "Success!\n";
                tree->outputCode(std::cout);

                CodeGenerator *codeGen= new CodeGenerator(*argv);
                if ( 0 == codeGen->generateCode(tree->getCode(), tree->getSymbolTable()) )
                {
                    codeGen->assemble();
                    codeGen->link();
                }
                else
                {
                    std::cerr << "Code generation failed!\n";
                }
                delete codeGen;
            }
            else
            {
                std::cerr << "Compilation failed with semantic errors!\n";
            }
        }
        else
            std::cout << "Parsing failed!\n";
        delete parser;
    }
    delete transConfiguration;
    return 0;
}
