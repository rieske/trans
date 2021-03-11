#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "types/Type.h"

namespace ast {

class Pointer: public AbstractSyntaxTreeNode {
public:
    Pointer(std::vector<type::Qualifier> qualifiers = {});
    ~Pointer() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    std::vector<type::Qualifier> getQualifiers() const;

private:
    std::vector<type::Qualifier> qualifiers;
};

} // namespace ast

#endif // _PTR_NODE_H_
