/*
 * VariableDefinition.cpp
 *
 *  Created on: Jul 28, 2014
 *      Author: vaidotas
 */

#include "VariableDefinition.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"
#include "DeclarationList.h"
#include "Expression.h"
#include "VariableDeclaration.h"

namespace semantic_analyzer {

VariableDefinition::VariableDefinition(std::unique_ptr<VariableDeclaration> declaration, std::unique_ptr<Expression> initializerExpression,
		SymbolTable *st, unsigned ln) :
		NonterminalNode(st, ln),
		declaration { std::move(declaration) },
		initializerExpression { std::move(initializerExpression) } {
	std::vector<Quadruple *> a_code = this->initializerExpression->getCode();
	code.insert(code.end(), a_code.begin(), a_code.end());
	code.push_back(
			new Quadruple(ASSIGN, this->initializerExpression->getPlace(), NULL,
					s_table->lookup(
							this->declaration->declarationList->getDeclarations().at(
									this->declaration->declarationList->getDeclarations().size() - 1)->getName())));
}

VariableDefinition::~VariableDefinition() {
}

void VariableDefinition::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
