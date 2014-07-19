#include "SemanticTreeBuilder.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"
#include "AbstractSyntaxTree.h"
#include "AdditionExpression.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "BitwiseAndExpression.h"
#include "BitwiseOrExpression.h"
#include "BitwiseXorExpression.h"
#include "Block.h"
#include "CastExpression.h"
#include "ComparisonExpression.h"
#include "Declaration.h"
#include "DeclarationList.h"
#include "DirectDeclaration.h"
#include "EqualityExpression.h"
#include "Factor.h"
#include "FunctionDeclaration.h"
#include "IOStatement.h"
#include "JumpStatement.h"
#include "LogicalAndExpression.h"
#include "LoopHeader.h"
#include "MatchedNode.h"
#include "ParameterList.h"
#include "Pointer.h"
#include "PostfixExpression.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "UnaryExpression.h"
#include "UnmatchedNode.h"
#include "VariableDeclaration.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

SemanticTreeBuilder::SemanticTreeBuilder() :
		symbolTable { new SymbolTable() },
		currentScope { symbolTable } {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(string definingSymbol, parser::Production production) {
	vector<AbstractSyntaxTreeNode*> children = getChildrenForReduction(production.size());

	NonterminalNode *nonterminalNode { nullptr };
	if (definingSymbol == "<u_op>" || definingSymbol == "<m_op>" || definingSymbol == "<add_op>" || definingSymbol == "<s_op>"
			|| definingSymbol == "<ml_op>" || definingSymbol == "<eq_op>" || definingSymbol == "<a_op>" || definingSymbol == "<stmt>"
			|| definingSymbol == "<statements>" || definingSymbol == "<var_decls>" || definingSymbol == "<func_decls>"
			|| definingSymbol == "<program>" || definingSymbol == "<type_spec>") {
		nonterminalNode = new Carrier(definingSymbol, children);
	} else if (definingSymbol == Term::ID) {
		if (production.produces( { "(", Expression::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = (Expression*) children[1];
		} else {
			nonterminalNode = new Term(context.popTerminal(), currentScope, currentLine);
		}
	} else if (definingSymbol == PostfixExpression::ID) {
		if (production.produces( { PostfixExpression::ID, "[", Expression::ID, "]" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new PostfixExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { PostfixExpression::ID, "(", AssignmentExpressionList::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new PostfixExpression((Expression*) children[0], (AssignmentExpressionList*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { PostfixExpression::ID, "(", ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new PostfixExpression((Expression*) children[0], currentScope, currentLine);
		} else if (production.produces( { PostfixExpression::ID, "++" }) || production.produces( { PostfixExpression::ID, "--" })) {
			nonterminalNode = new PostfixExpression((Expression*) children[0], context.popTerminal(), currentScope, currentLine);
		} else if (production.produces( { Term::ID })) {
			nonterminalNode = (Term*) children[0];
		}
	} else if (definingSymbol == UnaryExpression::ID) {
		if (production.produces( { "++", UnaryExpression::ID }) || production.produces( { "--", UnaryExpression::ID })) {
			nonterminalNode = new UnaryExpression(context.popTerminal(), (UnaryExpression*) children[1], currentScope, currentLine);
		} else if (production.produces( { "<u_op>", CastExpression::ID })) {
			nonterminalNode = new UnaryExpression(context.popTerminal(), (CastExpression*) children[1], currentScope, currentLine);
		} else if (production.produces( { PostfixExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == CastExpression::ID) {
		if (production.produces( { "(", "<type_spec>", ")", CastExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new CastExpression(context.popTerminal(), (Expression*) children[3], currentScope, currentLine);
			context.popTerminal();
		} else if (production.produces( { "(", "<type_spec>", Pointer::ID, ")", CastExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new CastExpression(context.popTerminal(), (Pointer*) children[2], (Expression*) children[4], currentScope,
					currentLine);
			context.popTerminal();
		} else if (production.produces( { UnaryExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == Factor::ID) {
		if (production.produces( { Factor::ID, "<m_op>", CastExpression::ID })) {
			nonterminalNode = new Factor((Expression*) children[0], context.popTerminal(), (Expression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { CastExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == AdditionExpression::ID) {
		if (production.produces( { Factor::ID })) {
			nonterminalNode = (Expression*) children[0];
		} else if (production.produces( { AdditionExpression::ID, "<add_op>", Factor::ID })) {
			nonterminalNode = new AdditionExpression((Expression*) children[0], context.popTerminal(), (Expression*) children[2],
					currentScope, currentLine);
		}
	} else if (definingSymbol == ShiftExpression::ID) {
		if (production.produces( { ShiftExpression::ID, "<s_op>", AdditionExpression::ID })) {
			nonterminalNode = new ShiftExpression((Expression*) children[0], context.popTerminal(), (Expression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { AdditionExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == ComparisonExpression::ID) {
		if (production.produces( { ComparisonExpression::ID, "<ml_op>", ShiftExpression::ID })) {
			nonterminalNode = new ComparisonExpression((Expression*) children[0], context.popTerminal(), (Expression*) children[2],
					currentScope, currentLine);
		} else if (production.produces( { ShiftExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == EqualityExpression::ID) {
		if (production.produces( { EqualityExpression::ID, "<eq_op>", ComparisonExpression::ID })) {
			nonterminalNode = new EqualityExpression((EqualityExpression*) children[0], context.popTerminal(),
					(ComparisonExpression*) children[2], currentScope, currentLine);
		} else if (production.produces( { ComparisonExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == BitwiseAndExpression::ID) {
		if (production.produces( { BitwiseAndExpression::ID, "&", EqualityExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new BitwiseAndExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { EqualityExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == BitwiseXorExpression::ID) {
		if (production.produces( { BitwiseXorExpression::ID, "^", BitwiseAndExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new BitwiseXorExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { BitwiseAndExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == BitwiseOrExpression::ID) {
		if (production.produces( { BitwiseOrExpression::ID, "|", BitwiseXorExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new BitwiseOrExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { BitwiseXorExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == LogicalAndExpression::ID) {
		if (production.produces( { LogicalAndExpression::ID, "&&", BitwiseOrExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new LogicalAndExpression((LogicalAndExpression*) children[0], (Expression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { BitwiseOrExpression::ID })) {
			nonterminalNode = new LogicalAndExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == LogicalOrExpression::ID) {
		if (production.produces( { LogicalOrExpression::ID, "||", LogicalAndExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new LogicalOrExpression((LogicalOrExpression*) children[0], (LogicalAndExpression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { LogicalAndExpression::ID })) {
			nonterminalNode = new LogicalOrExpression((LogicalAndExpression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == AssignmentExpressionList::ID) {
		if (production.produces( { AssignmentExpression::ID })) {
			nonterminalNode = new AssignmentExpressionList((AssignmentExpression*) children[0]);
		} else if (production.produces( { AssignmentExpressionList::ID, ",", AssignmentExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new AssignmentExpressionList((AssignmentExpressionList*) children[0], (AssignmentExpression*) children[2]);
		}
	} else if (definingSymbol == AssignmentExpression::ID) {
		if (production.produces( { LogicalOrExpression::ID })) {
			nonterminalNode = new AssignmentExpression((LogicalOrExpression*) children[0], currentScope, currentLine);
		} else if (production.produces( { UnaryExpression::ID, "<a_op>", AssignmentExpression::ID })) {
			nonterminalNode = new AssignmentExpression((Expression*) children[0], context.popTerminal(), (Expression*) children[2],
					currentScope, currentLine);
		}
	} else if (definingSymbol == Expression::ID) {
		if (production.produces( { Expression::ID, ",", AssignmentExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new Expression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { AssignmentExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == JumpStatement::ID) {
		if (production.produces( { "continue", ";" }) || production.produces( { "break", ";" })) {
			nonterminalNode = new JumpStatement(context.popTerminal().value, currentScope, currentLine);
			context.popTerminal();
		} else if (production.produces( { "return", Expression::ID, ";" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new JumpStatement((Expression*) children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == IOStatement::ID) {
		if (production.produces( { "output", Expression::ID, ";" }) || production.produces( { "input", Expression::ID, ";" })) {
			context.popTerminal();
			nonterminalNode = new IOStatement(context.popTerminal(), (Expression*) children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == LoopHeader::ID) {
		if (production.produces( { "while", "(", Expression::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new LoopHeader((Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new LoopHeader((Expression*) children[2], (Expression*) children[4], (Expression*) children[6], currentScope,
					currentLine);
		}
	} else if (definingSymbol == UnmatchedNode::ID) {
		if (production.produces( { "if", "(", Expression::ID, ")", "<stmt>" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new UnmatchedNode((Expression*) children[2], children[4], currentScope, currentLine);
		} else if (production.produces( { "if", "(", Expression::ID, ")", MatchedNode::ID, "else", UnmatchedNode::ID })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new UnmatchedNode((Expression*) children[2], children[4], children[6], currentScope, currentLine);
		} else if (production.produces( { LoopHeader::ID, UnmatchedNode::ID })) {
			nonterminalNode = new UnmatchedNode((LoopHeader*) children[0], children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == MatchedNode::ID) {
		if (production.produces( { Expression::ID, ";" })) {
			context.popTerminal();
			nonterminalNode = (Expression*) children[0];
		} else if (production.produces( { IOStatement::ID }) || production.produces( { Block::ID }) || production.produces( { ";" })
				|| production.produces( { JumpStatement::ID })) {
			if (production.produces( { ";" })) {
				context.popTerminal();
			}
			nonterminalNode = (NonterminalNode*) children[0];
		} else if (production.produces( { "if", "(", Expression::ID, ")", MatchedNode::ID, "else", MatchedNode::ID })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new MatchedNode((Expression*) children[2], children[4], children[6], currentScope, currentLine);
		} else if (production.produces( { LoopHeader::ID, MatchedNode::ID })) {
			nonterminalNode = new MatchedNode((LoopHeader*) children[0], children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == ParameterDeclaration::ID) {
		if (production.produces( { "<type_spec>", Declaration::ID })) {
			nonterminalNode = new ParameterDeclaration(context.popTerminal(), (Declaration*) children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == ParameterList::ID) {
		if (production.produces( { ParameterList::ID, ",", ParameterDeclaration::ID })) {
			context.popTerminal();
			nonterminalNode = new ParameterList((ParameterList*) children[0], (ParameterDeclaration*) children[2]);
		} else if (production.produces( { ParameterDeclaration::ID })) {
			nonterminalNode = new ParameterList((ParameterDeclaration*) children[0]);
		}
	} else if (definingSymbol == DirectDeclaration::ID) {
		if (production.produces( { "(", Declaration::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new DirectDeclaration((Declaration*) children[1], currentScope, currentLine);
		} else if (production.produces( { "id" })) {
			nonterminalNode = new DirectDeclaration(context.popTerminal(), currentScope, currentLine);
		} else if (production.produces( { DirectDeclaration::ID, "(", ParameterList::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new DirectDeclaration((DirectDeclaration*) children[0], (ParameterList*) children[2], currentScope,
					currentLine);
			declaredParams = ((DirectDeclaration *) nonterminalNode)->getParams();
		} else if (production.produces( { DirectDeclaration::ID, "[", LogicalOrExpression::ID, "]" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new DirectDeclaration((DirectDeclaration*) children[0], (LogicalOrExpression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { DirectDeclaration::ID, "(", ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new DirectDeclaration((DirectDeclaration*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == Pointer::ID) {
		if (production.produces( { Pointer::ID, "*" })) {
			context.popTerminal();
			nonterminalNode = new Pointer((Pointer*) children[0]);
		} else if (production.produces( { "*" })) {
			context.popTerminal();
			nonterminalNode = new Pointer();
		}
	} else if (definingSymbol == Block::ID) {
		nonterminalNode = new Block(children);
	} else if (definingSymbol == Declaration::ID) {
		if (production.produces( { Pointer::ID, DirectDeclaration::ID })) {
			nonterminalNode = new Declaration((Pointer*) children[0], (DirectDeclaration*) children[1]);
		} else if (production.produces( { DirectDeclaration::ID })) {
			nonterminalNode = (DirectDeclaration*) children[0];
		}
	} else if (definingSymbol == DeclarationList::ID) {
		if (production.produces( { DeclarationList::ID, ",", Declaration::ID })) {
			context.popTerminal();
			nonterminalNode = new DeclarationList((DeclarationList*) children[0], (Declaration*) children[2]);
		} else if (production.produces( { Declaration::ID })) {
			nonterminalNode = new DeclarationList((Declaration*) children[0]);
		}
	} else if (definingSymbol == VariableDeclaration::ID) {
		if (production.produces( { "<type_spec>", DeclarationList::ID, ";" })) {
			context.popTerminal();
			nonterminalNode = new VariableDeclaration(context.popTerminal(), (DeclarationList*) children[1], currentScope, currentLine);
		} else if (production.produces( { "<type_spec>", DeclarationList::ID, "=", AssignmentExpression::ID, ";" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new VariableDeclaration(context.popTerminal(), (DeclarationList*) children[1], (Expression*) children[3],
					currentScope, currentLine);
		}
	} else if (definingSymbol == FunctionDeclaration::ID) {
		if (production.produces( { "<type_spec>", Declaration::ID, Block::ID })) {
			nonterminalNode = new FunctionDeclaration(context.popTerminal(), (Declaration*) children[1], children[2], currentScope,
					currentLine);
		}
	}

	if (!nonterminalNode) {
		noSemanticActionsFoundFor(definingSymbol, production);
	}

	containsSemanticErrors |= nonterminalNode->getErrorFlag();
	syntaxStack.push(nonterminalNode);
}

void SemanticTreeBuilder::makeTerminalNode(std::string type, std::string value, size_t line) {
	adjustScope(value);
	currentLine = line;
	// XXX: create dummy until the context separation is resolved
	NonterminalNode *t_node = new NonterminalNode(value, {});
	syntaxStack.push(t_node);

	context.pushTerminal( { type, value, line });
}

// FIXME: incorporate scope data into the AST
void SemanticTreeBuilder::adjustScope(string lexeme) {
	if (lexeme == "{") {
		currentScope = currentScope->newScope();
		for (const auto declaredParam : declaredParams) {
			currentScope->insertParam(declaredParam->getPlace()->getName(), declaredParam->getPlace()->getBasicType(),
					declaredParam->getPlace()->getExtendedType(), currentLine);
		}
		declaredParams.clear();
	} else if (lexeme == "}") {
		currentScope = currentScope->getOuterScope();
	}
}

std::unique_ptr<parser::SyntaxTree> SemanticTreeBuilder::build() {
	if (containsSemanticErrors) {
		throw std::runtime_error("Compilation failed with semantic errors!");
	}
	return std::unique_ptr<parser::SyntaxTree>(new AbstractSyntaxTree(syntaxStack.top(), symbolTable));
}

void SemanticTreeBuilder::noSemanticActionsFoundFor(std::string definingSymbol, const parser::Production& production) const {
	std::ostringstream productionString;
	for (auto& symbol : production) {
		productionString << *symbol << " ";
	}
	throw std::runtime_error { "no semantic actions found for " + definingSymbol + " ::= " + productionString.str() };
}

vector<AbstractSyntaxTreeNode*> SemanticTreeBuilder::getChildrenForReduction(int childrenCount) {
	vector<AbstractSyntaxTreeNode*> children;
	for (int i = childrenCount; i > 0; --i) {
		children.push_back(syntaxStack.top());
		syntaxStack.pop();
	}
	std::reverse(children.begin(), children.end());
	return children;
}

}
