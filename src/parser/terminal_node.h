#ifndef _TERMINAL_NODE_H_
#define _TERMINAL_NODE_H_

#include "ParseTreeNode.h"
#include "scanner/Token.h"

/**
 * @author Vaidotas Valuckas
 * Terminalinio gramatikos simbolio mazgas sintaksiniame medyje
 **/

class TerminalNode: public ParseTreeNode
{
    public:
        TerminalNode(string label, string lexeme);

        string getAttr() const;
        virtual ostringstream   &asXml(ostringstream &oss, unsigned depth)     const;

    private:
        string value;
};

#endif // _TERMINAL_NODE_H_
