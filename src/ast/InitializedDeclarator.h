#ifndef INITIALIZEDDECLARATOR_H_
#define INITIALIZEDDECLARATOR_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Declarator.h"
#include "Expression.h"

namespace ast {

// One field store for local struct brace/designated initialization.
struct StructFieldInit {
    int offsetBytes { 0 };
    bool zeroInitialize { false }; // emit AssignConstant 0 into source before store
    // When set, emit AssignConstant(value) into source before store (char array
    // from string literal: local char buf[] = "hi").
    std::optional<std::string> constantValue;
    std::unique_ptr<symbols::ValueEntry> source;  // value to store (or zero temp)
    std::unique_ptr<symbols::ValueEntry> address; // field address temp
    // When set, source is filled with AddressOf(operand) before store (array->pointer decay).
    std::optional<std::string> addressOfOperand;
};

class InitializedDeclarator: public AbstractSyntaxTreeNode {
public:
    InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer = nullptr);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    type::Type getFundamentalType(const type::Type& baseType);

    Declarator* getDeclarator() const;
    bool hasInitializer() const;
    Expression* getInitializer() const;
    symbols::ValueEntry* getInitializerHolder() const;

    translation_unit::Context getContext() const;

    void setHolder(symbols::ValueEntry holder);
    symbols::ValueEntry* getHolder() const;

    void addStructFieldInit(StructFieldInit init);
    const std::vector<StructFieldInit>& getStructFieldInits() const;

private:
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<Expression> initializer;

    std::vector<StructFieldInit> structFieldInits;
};

} // namespace ast

#endif // INITIALIZEDDECLARATOR_H_
