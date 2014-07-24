#include "UnaryExpression.h"

#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string UnaryExpression::ID { "<u_expr>" };

UnaryExpression::UnaryExpression(TerminalSymbol unaryOperator, std::unique_ptr<Expression> castExpression, SymbolTable *st) :
		Expression(ID, { }, st, unaryOperator.line),
		castExpression { std::move(castExpression) },
		unaryOperator { unaryOperator } {
	saveExpressionAttributes(*this->castExpression);
	vector<Quadruple *>::iterator it = code.begin();
	SymbolEntry *temp;
	SymbolEntry *true_label;
	SymbolEntry *exit_label;
	switch (unaryOperator.value.at(0)) {
	case '&':
		extended_type = "p" + extended_type;
		temp = s_table->newTemp(basic_type, extended_type);
		code.insert(it, new Quadruple(ADDR, place, NULL, temp));
		place = temp;
		break;
	case '*':
		if (extended_type.size() && extended_type.at(0) == 'p') {
			extended_type = extended_type.substr(1, extended_type.size());
			temp = s_table->newTemp(basic_type, extended_type);
			code.insert(code.begin(), new Quadruple(DEREF, place, NULL, temp));
			place = temp;
		} else {
			semanticError("invalid type argument of ‘unary *’\n");
		}
		break;
	case '+':   // čia kogero nieko ir nereiks
		break;
	case '-':
		value = "rval";
		temp = s_table->newTemp(basic_type, extended_type);
		code.push_back(new Quadruple(UMINUS, place, NULL, temp));
		place = temp;
		break;
	case '!':   // TODO:
		value = "rval";
		temp = s_table->newTemp("int", "");
		true_label = s_table->newLabel();
		exit_label = s_table->newLabel();
		code.push_back(new Quadruple(CMP, place, 0, temp));
		code.push_back(new Quadruple(JE, true_label, NULL, NULL));
		code.push_back(new Quadruple("0", temp));
		code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
		code.push_back(new Quadruple(LABEL, true_label, NULL, NULL));
		code.push_back(new Quadruple("1", temp));
		code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
		place = temp;
		break;
	default:
		semanticError("Unidentified increment operator: " + unaryOperator.value);
		break;
	}
}

void UnaryExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
