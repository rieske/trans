#ifndef DECLARATOR_H_
#define DECLARATOR_H_

#include <string>

#include "AbstractSyntaxTreeNode.h"

namespace code_generator {
class ValueEntry;
} /* namespace code_generator */

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {

class Declarator: public AbstractSyntaxTreeNode {
public:
    virtual ~Declarator() {
    }

    const static std::string ID;

    virtual std::string getName() const = 0;
    virtual int getDereferenceCount() const = 0;

    virtual translation_unit::Context getContext() const = 0;

    virtual void setHolder(code_generator::ValueEntry holder) = 0;
    virtual code_generator::ValueEntry* getHolder() const = 0;
};

} /* namespace ast */

#endif /* DECLARATOR_H_ */
