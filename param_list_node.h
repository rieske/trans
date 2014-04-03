#ifndef _PARAM_LIST_NODE_H_
#define _PARAM_LIST_NODE_H_

#include "nonterminal_node.h"
#include "param_decl_node.h"

/**
 * @author Vaidotas Valuckas
 * funkcijos parametrų sąrašo klasė
 **/

class ParamListNode : public NonterminalNode
{
    public:
        ParamListNode(string left, vector<Node *> &children, string reduction);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        vector<ParamDeclNode *> getParams() const;

        void outputParams(ostringstream &oss) const;

    private:
        vector<ParamDeclNode *> params;
};

#endif // _PARAM_LIST_NODE_H_
