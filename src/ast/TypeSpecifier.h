#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <optional>
#include <string>

#include "types/Type.h"

namespace ast {

class AbstractSyntaxTreeVisitor;
class Declarator;
class Expression;
class ParseEnvironment;
struct TypeName;

class TypeSpecifier {
public:
    TypeSpecifier(type::Type type, std::string name);
    explicit TypeSpecifier(std::shared_ptr<Expression> typeofOperand);
    // typeof(type_name): keep TypeName (spec + dad) until resolve.
    explicit TypeSpecifier(TypeName typeName);
    ~TypeSpecifier();
    TypeSpecifier(const TypeSpecifier&);
    TypeSpecifier& operator=(const TypeSpecifier&);
    TypeSpecifier(TypeSpecifier&&) noexcept;
    TypeSpecifier& operator=(TypeSpecifier&&) noexcept;

    const std::string& getName() const;
    bool hasType() const;
    // Concrete type after parse-time or SA resolve. Throws if still unset.
    type::Type getType() const;
    void dropSpelling();

    void deferAbstractDeclarator(std::unique_ptr<Declarator> declarator);
    void resolveTypeof(AbstractSyntaxTreeVisitor& visitor);
    bool resolveTypeofAtParseTime(const ParseEnvironment& environment);
    bool needsSemanticResolve() const;
    void refoldConstantArrayBounds();

private:
    std::string name;
    std::optional<type::Type> type;
    std::shared_ptr<Expression> typeofOperand_;
    std::shared_ptr<TypeName> typeofTypeName_;
    std::shared_ptr<Declarator> deferredDeclarator_;

    void applyDeclarator();
};

} // namespace ast

#endif // TYPESPECIFIER_H_
