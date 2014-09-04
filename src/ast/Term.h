#ifndef _TERM_NODE_H_
#define _TERM_NODE_H_

#include <string>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace ast {

class Term: public Expression {
public:
    Term(TerminalSymbol term);
    virtual ~Term();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    const TranslationUnitContext& getContext() const override;

    std::string getType() const;
    std::string getValue() const;

    static const std::string ID;

private:
    const TerminalSymbol term;
};

}

#endif // _TERM_NODE_H_
