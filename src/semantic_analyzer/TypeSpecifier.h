#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <string>

#include "BasicType.h"
#include "NonterminalNode.h"

namespace semantic_analyzer {

class TypeSpecifier: public NonterminalNode {
public:
    TypeSpecifier(BasicType type, std::string name);
    virtual ~TypeSpecifier();

    const std::string& getName() const;
    BasicType getType() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) const override;

private:
    std::string name;
    BasicType type;
};

} /* namespace semantic_analyzer */

#endif /* TYPESPECIFIER_H_ */
