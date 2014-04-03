#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include "quadruple.h"
#include "parser/nonterminal_node.h"

class ExprNode : public NonterminalNode
{
    public:
        ExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        virtual string getBasicType() const;
        virtual string getExtendedType() const;
        virtual string getValue() const;

        virtual SymbolEntry *getPlace() const;
        virtual SymbolEntry *getLval() const;

        void printCode() const;

    protected:

        void getAttributes(int subtree);

        Quadruple *backpatch(vector<Quadruple *> *bpList);
        Quadruple *backpatch(vector<Quadruple *> *bpList, Quadruple* q);

        string basic_type;
        string extended_type;
        string value;

        SymbolEntry *place;
        SymbolEntry *lval;
};

#endif // _EXPR_NODE_H_
