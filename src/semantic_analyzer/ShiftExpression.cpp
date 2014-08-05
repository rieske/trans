#include "ShiftExpression.h"

#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

const std::string ShiftExpression::ID { "<s_expr>" };

ShiftExpression::ShiftExpression(std::unique_ptr<Expression> shiftExpression, TerminalSymbol shiftOperator,
		std::unique_ptr<Expression> additionExpression, SymbolTable *st) :
		Expression(st, shiftOperator.line),
		shiftExpression { std::move(shiftExpression) },
		shiftOperator { shiftOperator },
		additionExpression { std::move(additionExpression) } {
	code = this->shiftExpression->getCode();
	value = "rval";
	basicType = this->shiftExpression->getBasicType();
	extended_type = this->shiftExpression->getExtendedType();
	SymbolEntry *arg1 = this->shiftExpression->getPlace();
	SymbolEntry *arg2 = this->additionExpression->getPlace();
	BasicType arg2type = this->additionExpression->getBasicType();
	string arg2etype = this->additionExpression->getExtendedType();
	if (arg2type == BasicType::INTEGER && arg2etype == "") {
		vector<Quadruple *> arg2code = this->additionExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		SymbolEntry *res = s_table->newTemp(basicType, extended_type);
		place = res;
		switch (shiftOperator.value.at(0)) {
		case '<':   // <<
			code.push_back(new Quadruple(SHL, arg1, arg2, res));
			break;
		case '>':   // >>
			code.push_back(new Quadruple(SHR, arg1, arg2, res));
			break;
		default:
			semanticError("unidentified add_op operator!\n");
		}
	} else {
		semanticError(" argument of type int required for shift expression\n");
	}
}

}

void semantic_analyzer::ShiftExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}
