#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "types/IntegerConstant.h"
#include "types/Type.h"

namespace ast {

// One enumeration constant, as written in an enum definition.
struct Enumerator {
    std::string name;
    type::IntegerConstant value;
};

class AbstractSyntaxTreeVisitor;
class Declarator;
class Expression;
class ParseEnvironment;
class VlaExpressionTable;

class TypeSpecifier {
public:
    TypeSpecifier(type::Type type, std::string name);
    explicit TypeSpecifier(std::shared_ptr<Expression> typeofOperand);
    TypeSpecifier(const TypeSpecifier&);
    TypeSpecifier& operator=(const TypeSpecifier&);
    TypeSpecifier(TypeSpecifier&&) noexcept;
    TypeSpecifier& operator=(TypeSpecifier&&) noexcept;

    const std::string& getName() const;
    bool hasType() const;
    type::Type getType() const;
    void dropSpelling();

    void deferAbstractDeclarator(std::unique_ptr<Declarator> declarator);
    void resolveTypeof(AbstractSyntaxTreeVisitor& visitor);
    bool resolveTypeofAtParseTime(const ParseEnvironment& environment);
    bool needsSemanticResolve() const;
    void refoldConstantArrayBounds(const VlaExpressionTable& exprs);
    void markDefinesRecord();
    bool definesRecord() const;
    void setEnumerators(std::vector<Enumerator> enumerators);
    const std::vector<Enumerator>& enumerators() const;

private:
    std::string name;
    std::optional<type::Type> type;
    std::shared_ptr<Expression> typeofOperand_;
    std::shared_ptr<Declarator> deferredDeclarator_;
    bool definesRecord_ { false };
    std::vector<Enumerator> enumerators_;

    void applyDeclarator();
};

type::Type foldConstantArrayBounds(const type::Type& t, const VlaExpressionTable& exprs);

} // namespace ast

#endif // TYPESPECIFIER_H_
