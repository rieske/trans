#include <iostream>
#include <stdio.h>
#include <string>

#include "token.h"
#include "scanner.h"
#include "parser.h"
#include "argv_parser.h"
#include "code_generator.h"

int main(int argc, char **argv)
{
    // suformuojam helpą
    std::string help = "Usage: ";
    help += argv[0];
    help +=    " [options] source_file\nOptions:\n";
    help +=    " -h\t\t\tDisplay this information\n";

    Argv_parser *argv_parser = new Argv_parser();
    int srcfiles = argv_parser->parse_argv(argc, argv);
    if (!srcfiles)
    {
        std::cout << help;
        delete argv_parser;
        return 1;
    }
    while (argc-- > srcfiles)
        *argv++;
    std::cout << "Compiling " << *argv << "...\n";

    while (srcfiles--)
    {
        Parser *parser = NULL;
        if (NULL != argv_parser->get_custom_grammar())
            parser = new Parser(argv_parser->get_custom_grammar());
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
    delete argv_parser;
    return 0;
}
