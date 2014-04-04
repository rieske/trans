#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include "../code_generator/symbol_table.h"
#include "../parser/node.h"
#include "../parser/terminal_node.h"
#include "../parser/nonterminal_node.h"
#include "carrier_node.h"
#include "dir_decl_node.h"
#include "decl_node.h"
#include "ptr_node.h"
#include "decls_node.h"
#include "var_decl_node.h"
#include "term_node.h"
#include "postfix_expr_node.h"
#include "u_expr_node.h"
#include "cast_expr_node.h"
#include "factor_node.h"
#include "add_expr_node.h"
#include "s_expr_node.h"
#include "ml_expr_node.h"
#include "eq_expr_node.h"
#include "and_expr_node.h"
#include "xor_expr_node.h"
#include "or_expr_node.h"
#include "log_and_expr_node.h"
#include "log_expr_node.h"
#include "a_expr_node.h"
#include "expr_node.h"
#include "func_decl_node.h"
#include "a_expressions_node.h"
#include "param_decl_node.h"
#include "param_list_node.h"
#include "jmp_stmt_node.h"
#include "io_stmt_node.h"
#include "loop_hdr_node.h"
#include "unmatched_node.h"
#include "matched_node.h"
#include "block_node.h"

/**
 * @author Vaidotas Valuckas
 * Sintaksinio medžio klasė
 **/

class SyntaxTree
{
    public:
        SyntaxTree();
        ~SyntaxTree();

        void setTree(Node *t);

        static char *getFileName();
        static unsigned getLine();

        void setFileName(char *);
        void setLine(unsigned l);
        void setErrorFlag();

        bool getErrorFlag() const;

        void outputCode(ostream &of) const;

        SymbolTable *getSymbolTable() const;
        vector<Quadruple *> getCode() const;

        string asXml() const;

        void printTables() const;

        void logTables();
        void logCode();

    private:
        Node *tree;

        SymbolTable *s_table;
        vector<Quadruple *> code;

        static char *filename;
        static unsigned line;
        bool error;
};

#endif // _SYNTAX_TREE_H_
