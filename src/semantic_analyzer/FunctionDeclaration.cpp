#include "FunctionDeclaration.h"

#include <algorithm>
#include <sstream>
#include <string>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "ParameterList.h"
#include "BasicType.h"

namespace semantic_analyzer {

FunctionDeclaration::FunctionDeclaration(std::unique_ptr<Declaration> declaration, SymbolTable *st) :
        FunctionDeclaration(std::move(declaration), std::unique_ptr<ParameterList> { new ParameterList() }, st) {
}

FunctionDeclaration::FunctionDeclaration(std::unique_ptr<Declaration> declaration, std::unique_ptr<ParameterList> parameterList,
        SymbolTable *st) :
        Declaration(declaration->getName(), "f" + declaration->getType(), declaration->getLineNumber(), st),
        parameterList { std::move(parameterList) } {
    for (const auto& param : this->parameterList->getDeclaredParameters()) {
        params.push_back(param.get());
    }
    int errLine;
    if (0 != (errLine = s_table->insert(name, BasicType::FUNCTION, type, sourceLine))) {
        std::ostringstream errorDescription;
        errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
        semanticError(errorDescription.str());
    } else {
        SymbolEntry *place = s_table->lookup(name);
        for (unsigned i = 0; i < params.size(); i++) {
            if (params[i]->getPlace() == NULL) {
                semanticError("params.place == nullptr");
            }
            SymbolEntry *entry = params[i]->getPlace();
            place->setParam(entry);
        }
    }
}

FunctionDeclaration::~FunctionDeclaration() {
}

const vector<ParameterDeclaration *> FunctionDeclaration::getParams() const {
    return params;
}

void FunctionDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
