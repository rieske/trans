#ifndef _EXPR_NODE_H_
#define _EXPR_NODE_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "types/Type.h"
#include "code_generator/ValueEntry.h"
#include "translation_unit/Context.h"

namespace ast {

class Expression: public AbstractSyntaxTreeNode {
public:
    virtual ~Expression() {
    }

    virtual translation_unit::Context context() const = 0;

    void setType(Type type);
    Type getType() const;

    virtual bool isLval() const;

    static const std::string ID;

    void setResultSymbol(code_generator::ValueEntry resultSymbol);
    code_generator::ValueEntry* getResultSymbol() const;

protected:
    bool lval { false };

private:
    std::unique_ptr<Type> type;

    std::unique_ptr<code_generator::ValueEntry> resultSymbol { nullptr };
};

}

#endif // _EXPR_NODE_H_
