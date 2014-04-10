#ifndef _PARSER_H_
#define _PARSER_H_

#include <stack>
#include <fstream>
#include "parsing_table.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/syntax_tree.h"

using std::stack;
using std::ofstream;

/**
 * @author Vaidotas
 * Pagrindinė LR(1) parserio klasė
 **/

class Parser
{
    public:
        Parser();
        Parser(string gra);
        ~Parser();

        int     parse(const char *src);
        SyntaxTree *getSyntaxTree() const;

        static void set_logging(const char *lf);

    private:
        void shift(Action *);
        void reduce(Action *);
        void error(Action *);

        void adjustScope();
        void mknode(string left, vector<Node *> children, string reduction);

        void configure_logging();

        void log_syntax_tree() const;

        void fail(string err);

        Scanner *scanner;
        Parsing_table *p_table;
        Token *token;
        Token *next_token;
        bool can_forge;
        bool success;
        stack<long> parsing_stack;
        stack<Node *> syntax_stack;

        SyntaxTree *syntax_tree;
        SymbolTable *current_scope;
        vector<ParamDeclNode *> params;

        unsigned line;
        static bool     log;
        static ofstream logfile;
        ofstream        *output;
        bool custom_grammar;
};

#endif // _PARSER_H_
