#include "FunctionDefinition.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "FunctionDeclaration.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"
#include "Declaration.h"

namespace semantic_analyzer {

const std::string FunctionDefinition::ID { "<func_decl>" };

FunctionDefinition::FunctionDefinition(TypeSpecifier returnType, std::unique_ptr<FunctionDeclaration> declaration,
        std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *s_t) :
        NonterminalNode(s_t, 0),
        returnType { returnType },
        declaration { std::move(declaration) },
        body { std::move(body) } {

    /*	for (const auto declaredParam : declaration->getParams()) {
     s_table->insertParam(declaredParam->getPlace()->getName(), declaredParam->getPlace()->getBasicType(),
     declaredParam->getPlace()->getExtendedType(), sourceLine);
     }*/

    name = this->declaration->getName();
    basicType = returnType.getType();
    extended_type = this->declaration->getType();
    code = this->body->getCode();
    SymbolEntry *entry = s_table->lookup(name);
    if (entry != NULL) {
        if (BasicType::FUNCTION == entry->getBasicType() || basicType == entry->getBasicType()) {
            entry->setBasicType(basicType);
            entry->setExtendedType(extended_type);
        } else {
            semanticError("function definition type does not match declared type for function: " + name);
        }
        code.insert(code.begin(), new Quadruple(PROC, entry, NULL, NULL));
        code.insert(code.end(), new Quadruple(ENDPROC, entry, NULL, NULL));
    } else {
        semanticError("fatal error: could't lookup function entry to fill return values!\n");
        exit(1);
    }
}

FunctionDefinition::~FunctionDefinition() {
}

void FunctionDefinition::accept(const AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

}
