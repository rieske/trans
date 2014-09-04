#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <string>

#include "BasicType.h"
#include "AbstractSyntaxTreeNode.h"

namespace ast {

class TypeSpecifier: public AbstractSyntaxTreeNode {
public:
    TypeSpecifier(BasicType type, std::string name);
    virtual ~TypeSpecifier();

    const std::string& getName() const;
    BasicType getType() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

private:
    std::string name;
    BasicType type;
};

} /* namespace ast */

#endif /* TYPESPECIFIER_H_ */
