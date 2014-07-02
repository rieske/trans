#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"

class ParseTreeNode;
class SymbolTable;

/**
 * @author Vaidotas Valuckas
 * Sintaksinio medžio klasė
 **/

class SyntaxTree
{
    public:
        SyntaxTree();
        ~SyntaxTree();

        void setTree(ParseTreeNode *t);

        static const char *getFileName();
        static unsigned getLine();

        void setFileName(const char *);
        void setLine(unsigned l);
        void setErrorFlag();

        bool hasSemanticErrors() const;

        void outputCode(ostream &of) const;

        SymbolTable *getSymbolTable() const;
        vector<Quadruple *> getCode() const;

        string asXml() const;

        void printTables() const;

        void logXml();
        void logTables();
        void logCode();

    private:
        ParseTreeNode *tree;

        SymbolTable *s_table;
        vector<Quadruple *> code;

        static const char *filename;
        bool error;
};

#endif // _SYNTAX_TREE_H_
