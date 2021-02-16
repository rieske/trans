#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <set>
#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "types/TypeQualifier.h"

namespace ast {

class Pointer: public AbstractSyntaxTreeNode {
public:
    Pointer(std::vector<TypeQualifier> qualifiers = {});
    ~Pointer() = default;
    Pointer(const Pointer& rhs);
    Pointer(Pointer&& rhs);

    Pointer& operator=(const Pointer& rhs);
    Pointer& operator=(Pointer&& rhs);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

    std::set<TypeQualifier> getQualifiers() const;

private:
    std::vector<TypeQualifier> qualifiers;
};

} // namespace ast

#endif // _PTR_NODE_H_
