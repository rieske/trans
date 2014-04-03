#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include "nonterminal_node.h"

/**
 * @author Vaidotas Valuckas
 * funkcijos parametro klasė
 **/

class ParamDeclNode : public NonterminalNode
{
    public:
        ParamDeclNode(string left, vector<Node*> &children, string reduction, SymbolTable *st, unsigned ln);

        string getBasicType() const;
        string getExtendedType() const;

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        void output_attr(ostringstream &oss, unsigned nr) const;

        SymbolEntry *getPlace() const;

    private:
        string name;
        string basic_type;
        string extended_type;

        SymbolEntry *place;
};

#endif // _PARAM_DECL_NODE_H_
