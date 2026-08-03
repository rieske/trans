#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <string>

#include "types/Type.h"

namespace ast {

class Expression;

class TypeSpecifier {
public:
    TypeSpecifier(type::Type type, std::string name);
    // __typeof__(expr): provisional type is void until SA resolves the operand.
    static TypeSpecifier makeTypeof(std::shared_ptr<Expression> operand);
    virtual ~TypeSpecifier() = default;

    const std::string& getName() const;
    // Concrete type after SA. For __typeof__(e), returns provisional void until
    // resolveTypeSpecifier (or setResolvedType) has run - do not trust getType()
    // while isTypeof() is still true.
    type::Type getType() const;

    // True only while still provisional (operand present, not yet setResolvedType).
    bool isTypeof() const { return typeofOperand != nullptr; }
    Expression* getTypeofOperand() const { return typeofOperand.get(); }
    // SA: write resolved type and clear the typeof operand so getType() is concrete.
    void setResolvedType(type::Type resolved);

private:
    TypeSpecifier(type::Type type, std::string name, std::shared_ptr<Expression> typeofOperand);

    std::string name;
    type::Type type;
    std::shared_ptr<Expression> typeofOperand;
};

} // namespace ast

#endif // TYPESPECIFIER_H_
