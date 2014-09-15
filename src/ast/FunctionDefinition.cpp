#include "FunctionDefinition.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

#include "FunctionDeclaration.h"

namespace ast {

const std::string FunctionDefinition::ID { "<func_decl>" };

FunctionDefinition::FunctionDefinition(TypeSpecifier returnType, std::unique_ptr<FunctionDeclaration> declaration,
        std::unique_ptr<AbstractSyntaxTreeNode> body) :
        returnType { returnType },
        declaration { std::move(declaration) },
        body { std::move(body) } {
}

FunctionDefinition::~FunctionDefinition() {
}

void FunctionDefinition::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
