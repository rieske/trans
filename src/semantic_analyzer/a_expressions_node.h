#ifndef _A_EXPRESSIONS_NODE_H_
#define _A_EXPRESSIONS_NODE_H_


/**
 * @author Vaidotas Valuckas
 * priskyrimo išraiškų sąrašo klasė
 **/

#include "../parser/nonterminal_node.h"
#include "a_expr_node.h"

class AExpressionsNode : public NonterminalNode
{
    public:
        AExpressionsNode(string l, vector<Node *> &children, Production production);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        vector<AExprNode *> getExprs() const;

        void outputExprs(ostringstream &oss) const;

    private:
        vector<AExprNode *> exprs;
};

#endif // _A_EXPRESSIONS_NODE_H_
