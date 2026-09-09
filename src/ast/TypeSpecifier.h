#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "translation_unit/Context.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"

namespace ast {

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
    TypeSpecifier(type::Type type, std::string name,
            translation_unit::Context context = translation_unit::Context { "", 0 },
            bool definesRecord = false);
    explicit TypeSpecifier(std::unique_ptr<Expression> typeofOperand);
    ~TypeSpecifier();
    TypeSpecifier(TypeSpecifier&&) noexcept;
    TypeSpecifier& operator=(TypeSpecifier&&) noexcept;
    TypeSpecifier(const TypeSpecifier&) = delete;
    TypeSpecifier& operator=(const TypeSpecifier&) = delete;

    const std::string& getName() const;
    const translation_unit::Context& getContext() const;
    bool hasType() const;
    type::Type getType() const;
    void dropSpelling();

    void deferAbstractDeclarator(std::unique_ptr<Declarator> declarator);
    void resolveTypeof(AbstractSyntaxTreeVisitor& visitor);
    bool resolveTypeofAtParseTime(const ParseEnvironment& environment);
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const;
    bool needsSemanticResolve() const;
    void refoldConstantArrayBounds(const VlaExpressionTable& exprs);
    bool definesRecord() const;
    void setEnumerators(std::vector<Enumerator> enumerators);
    const std::vector<Enumerator>& enumerators() const;

private:
    std::string name;
    translation_unit::Context context_ { "", 0 };
    std::optional<type::Type> type;
    std::unique_ptr<Expression> typeofOperand_;
    std::unique_ptr<Declarator> deferredDeclarator_;
    bool definesRecord_ { false };
    std::vector<Enumerator> enumerators_;

    void applyDeclarator();
};

type::Type foldConstantArrayBounds(const type::Type& t, const VlaExpressionTable& exprs);

} // namespace ast

#endif // TYPESPECIFIER_H_
