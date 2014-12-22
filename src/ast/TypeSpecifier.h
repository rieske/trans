#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"

namespace ast {
class BaseType;
}

namespace ast {

class TypeSpecifier: public AbstractSyntaxTreeNode {
public:
    TypeSpecifier(std::unique_ptr<BaseType> type, std::string name);
    TypeSpecifier(const TypeSpecifier& copyFrom);
    virtual ~TypeSpecifier();

    const std::string& getName() const;
    std::unique_ptr<BaseType> getType() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

private:
    const std::string name;
    const std::unique_ptr<const BaseType> type;
};

} /* namespace ast */

#endif /* TYPESPECIFIER_H_ */
