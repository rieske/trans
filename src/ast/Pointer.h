#ifndef _PTR_NODE_H_
#define _PTR_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "TypeQualifier.h"

namespace ast {

class Pointer: public AbstractSyntaxTreeNode {
public:
    Pointer(std::vector<TypeQualifier> qualifiers);
    Pointer(Pointer* pointer = nullptr, std::vector<TypeQualifier> qualifiers = { });
    ~Pointer() = default;
    Pointer(const Pointer& rhs);
    Pointer(Pointer&& rhs);

    Pointer& operator=(const Pointer& rhs);
    Pointer& operator=(Pointer&& rhs);

    int getDereferenceCount() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

private:
    std::unique_ptr<Pointer> pointer;
    std::vector<TypeQualifier> qualifiers;
};

}

#endif // _PTR_NODE_H_
