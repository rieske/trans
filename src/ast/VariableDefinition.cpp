#include "VariableDefinition.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "VariableDeclaration.h"
#include "Expression.h"
#include "DeclarationList.h"
#include "Declaration.h"

namespace ast {

VariableDefinition::VariableDefinition(std::unique_ptr<VariableDeclaration> declaration,
        std::unique_ptr<Expression> initializerExpression) :
        declaration { std::move(declaration) },
        initializerExpression { std::move(initializerExpression) } {
}

VariableDefinition::~VariableDefinition() {
}

void VariableDefinition::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
