#include "SemanticTreeBuilder.h"

#include <iterator>
#include <sstream>
#include <stack>
#include <stdexcept>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"
#include "../parser/TerminalNode.h"
#include "../scanner/Token.h"
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
#include "UnaryExpression.h"
#include "UnmatchedNode.h"
#include "VariableDeclaration.h"

using std::string;
using std::vector;
using parser::ParseTreeNode;
using parser::TerminalNode;

namespace semantic_analyzer {

SemanticTreeBuilder::SemanticTreeBuilder() :
		symbolTable { new SymbolTable() },
		currentScope { symbolTable } {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(string definingSymbol, parser::Production production) {
	vector<ParseTreeNode *> children = getChildrenForReduction(production.size());

	ParseTreeNode *nonterminalNode { nullptr };
	if (definingSymbol == "<u_op>" || definingSymbol == "<m_op>" || definingSymbol == "<add_op>" || definingSymbol == "<s_op>"
			|| definingSymbol == "<ml_op>" || definingSymbol == "<eq_op>" || definingSymbol == "<a_op>" || definingSymbol == "<stmt>"
			|| definingSymbol == "<statements>" || definingSymbol == "<var_decls>" || definingSymbol == "<func_decls>"
			|| definingSymbol == "<program>" || definingSymbol == "<type_spec>") {
		nonterminalNode = new Carrier(definingSymbol, children);
	} else if (definingSymbol == Term::ID) {
		if (production.produces( { "(", Expression::ID, ")" })) {
			nonterminalNode = new Term((Expression*) children[1], currentScope, currentLine);
		} else { // FIXME::
			nonterminalNode = new Term(children[0], (*production.begin())->getSymbol(), currentScope, currentLine);
		}
	} else if (definingSymbol == PostfixExpression::ID) {
		if (production.produces( { PostfixExpression::ID, "[", Expression::ID, "]" })) {
			nonterminalNode = new PostfixExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { PostfixExpression::ID, "(", AssignmentExpressionList::ID, ")" })) {
			nonterminalNode = new PostfixExpression((Expression*) children[0], (AssignmentExpressionList*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { PostfixExpression::ID, "(", ")" })) {
			nonterminalNode = new PostfixExpression((Expression*) children[0], currentScope, currentLine);
		} else if (production.produces( { PostfixExpression::ID, "++" }) || production.produces( { PostfixExpression::ID, "--" })) {
			nonterminalNode = new PostfixExpression((Expression*) children[0], children[1]->getAttr(), currentScope, currentLine);
		} else if (production.produces( { Term::ID })) {
			nonterminalNode = new PostfixExpression((Term*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == UnaryExpression::ID) {
		if (production.produces( { "++", UnaryExpression::ID }) || production.produces( { "--", UnaryExpression::ID })) {
			nonterminalNode = new UnaryExpression(children[0]->getAttr(), (UnaryExpression*) children[1], currentScope, currentLine);
		} else if (production.produces( { "<u_op>", CastExpression::ID })) {
			nonterminalNode = new UnaryExpression(children[0]->getAttr(), (CastExpression*) children[1], currentScope, currentLine);
		} else if (production.produces( { PostfixExpression::ID })) {
			nonterminalNode = new UnaryExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == CastExpression::ID) {
		if (production.produces( { "(", "<type_spec>", ")", CastExpression::ID })) {
			nonterminalNode = new CastExpression(children[1], (Expression*) children[3], currentScope, currentLine);
		} else if (production.produces( { "(", "<type_spec>", Pointer::ID, ")", CastExpression::ID })) {
			nonterminalNode = new CastExpression(children[1], (Pointer*) children[2], (Expression*) children[4], currentScope, currentLine);
		} else if (production.produces( { UnaryExpression::ID })) {
			nonterminalNode = new CastExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == Factor::ID) {
		if (production.produces( { Factor::ID, "<m_op>", CastExpression::ID })) {
			nonterminalNode = new Factor((Expression*) children[0], children[1]->getAttr(), (Expression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { CastExpression::ID })) {
			nonterminalNode = new Factor((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == AdditionExpression::ID) {
		if (production.produces( { Factor::ID })) {
			nonterminalNode = new AdditionExpression((Expression*) children[0], currentScope, currentLine);
		} else if (production.produces( { AdditionExpression::ID, "<add_op>", Factor::ID })) {
			nonterminalNode = new AdditionExpression((Expression*) children[0], children[1]->getAttr(), (Expression*) children[2],
					currentScope, currentLine);
		}
	} else if (definingSymbol == ShiftExpression::ID) {
		if (production.produces( { ShiftExpression::ID, "<s_op>", AdditionExpression::ID })) {
			nonterminalNode = new ShiftExpression((Expression*) children[0], children[1]->getAttr(), (Expression*) children[2],
					currentScope, currentLine);
		} else if (production.produces( { AdditionExpression::ID })) {
			nonterminalNode = new ShiftExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == ComparisonExpression::ID) {
		if (production.produces( { ComparisonExpression::ID, "<ml_op>", ShiftExpression::ID })) {
			nonterminalNode = new ComparisonExpression((Expression*) children[0], children[1]->getAttr(), (Expression*) children[2],
					currentScope, currentLine);
		} else if (production.produces( { ShiftExpression::ID })) {
			nonterminalNode = new ComparisonExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == EqualityExpression::ID) {
		if (production.produces( { EqualityExpression::ID, "<eq_op>", ComparisonExpression::ID })) {
			nonterminalNode = new EqualityExpression((EqualityExpression*) children[0], children[1]->getAttr(),
					(ComparisonExpression*) children[2], currentScope, currentLine);
		} else if (production.produces( { ComparisonExpression::ID })) {
			nonterminalNode = new EqualityExpression((ComparisonExpression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == BitwiseAndExpression::ID) {
		if (production.produces( { BitwiseAndExpression::ID, "&", EqualityExpression::ID })) {
			nonterminalNode = new BitwiseAndExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { EqualityExpression::ID })) {
			nonterminalNode = new BitwiseAndExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == BitwiseXorExpression::ID) {
		if (production.produces( { BitwiseXorExpression::ID, "^", BitwiseAndExpression::ID })) {
			nonterminalNode = new BitwiseXorExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { BitwiseAndExpression::ID })) {
			nonterminalNode = new BitwiseXorExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == BitwiseOrExpression::ID) {
		if (production.produces( { BitwiseOrExpression::ID, "|", "<xor_expr>" })) {
			nonterminalNode = new BitwiseOrExpression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { "<xor_expr>" })) {
			nonterminalNode = new BitwiseOrExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == LogicalAndExpression::ID) {
		if (production.produces( { LogicalAndExpression::ID, "&&", BitwiseOrExpression::ID })) {
			nonterminalNode = new LogicalAndExpression((LogicalAndExpression*) children[0], (Expression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { BitwiseOrExpression::ID })) {
			nonterminalNode = new LogicalAndExpression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == LogicalOrExpression::ID) {
		if (production.produces( { LogicalOrExpression::ID, "||", LogicalAndExpression::ID })) {
			nonterminalNode = new LogicalOrExpression((LogicalOrExpression*) children[0], (LogicalAndExpression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { LogicalAndExpression::ID })) {
			nonterminalNode = new LogicalOrExpression((LogicalAndExpression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == AssignmentExpressionList::ID) {
		//AssignmentExpressionList* assignmentExpressionList;
		if (production.produces( { AssignmentExpression::ID })) {
			//assignmentExpressionList = new AExpressionsNode(definingSymbol, assignmentExpressions.top());
			//assignmentExpressions.pop();
			nonterminalNode = new AssignmentExpressionList((AssignmentExpression*) children[0]);
		} else if (production.produces( { AssignmentExpressionList::ID, ",", AssignmentExpression::ID })) {
			//assignmentExpressionList = new AExpressionsNode(definingSymbol, assignmentExpressionLists.top(), assignmentExpressions.top());
			//assignmentExpressionLists.pop();
			//assignmentExpressions.pop();
			nonterminalNode = new AssignmentExpressionList((AssignmentExpressionList*) children[0], (AssignmentExpression*) children[2]);
		}
		//assignmentExpressionLists.push(assignmentExpressionList);
	} else if (definingSymbol == AssignmentExpression::ID) {
		if (production.produces( { LogicalOrExpression::ID })) {
			nonterminalNode = new AssignmentExpression((LogicalOrExpression*) children[0], currentScope, currentLine);
		} else if (production.produces( { UnaryExpression::ID, "<a_op>", AssignmentExpression::ID })) {
			nonterminalNode = new AssignmentExpression((Expression*) children[0], children[1]->getAttr(), (Expression*) children[2],
					currentScope, currentLine);
		}
	} else if (definingSymbol == Expression::ID) {
		if (production.produces( { Expression::ID, ",", AssignmentExpression::ID })) {
			nonterminalNode = new Expression((Expression*) children[0], (Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { AssignmentExpression::ID })) {
			nonterminalNode = new Expression((Expression*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == JumpStatement::ID) {
		if (production.produces( { "continue", ";" }) || production.produces( { "break", ";" })) {
			nonterminalNode = new JumpStatement(children[0]->getAttr(), currentScope, currentLine);
		} else if (production.produces( { "return", Expression::ID, ";" })) {
			nonterminalNode = new JumpStatement((Expression*) children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == IOStatement::ID) {
		if (production.produces( { "output", Expression::ID, ";" }) || production.produces( { "input", Expression::ID, ";" })) {
			nonterminalNode = new IOStatement(children[0]->getAttr(), (Expression*) children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == LoopHeader::ID) {
		if (production.produces( { "while", "(", Expression::ID, ")" })) {
			nonterminalNode = new LoopHeader((Expression*) children[2], currentScope, currentLine);
		} else if (production.produces( { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")" })) {
			nonterminalNode = new LoopHeader((Expression*) children[2], (Expression*) children[4], (Expression*) children[6], currentScope,
					currentLine);
		}
	} else if (definingSymbol == UnmatchedNode::ID) {
		if (production.produces( { "if", "(", Expression::ID, ")", "<stmt>" })) {
			nonterminalNode = new UnmatchedNode((Expression*) children[2], children[4], currentScope, currentLine);
		} else if (production.produces( { "if", "(", Expression::ID, ")", MatchedNode::ID, "else", UnmatchedNode::ID })) {
			nonterminalNode = new UnmatchedNode((Expression*) children[2], children[4], children[6], currentScope, currentLine);
		} else if (production.produces( { LoopHeader::ID, UnmatchedNode::ID })) {
			nonterminalNode = new UnmatchedNode((LoopHeader*) children[0], children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == MatchedNode::ID) {
		if (production.produces( { Expression::ID, ";" })) {
			nonterminalNode = new MatchedNode((Expression*) children[0], currentScope, currentLine);
		} else if (production.produces( { IOStatement::ID }) || production.produces( { Block::ID }) || production.produces( { ";" })
				|| production.produces( { JumpStatement::ID })) {
			nonterminalNode = new MatchedNode(children[0], currentScope, currentLine);
		} else if (production.produces( { "if", "(", Expression::ID, ")", MatchedNode::ID, "else", MatchedNode::ID })) {
			nonterminalNode = new MatchedNode((Expression*) children[2], children[4], children[6], currentScope, currentLine);
		} else if (production.produces( { LoopHeader::ID, MatchedNode::ID })) {
			nonterminalNode = new MatchedNode((LoopHeader*) children[0], children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == ParameterDeclaration::ID) {
		if (production.produces( { "<type_spec>", Declaration::ID })) {
			nonterminalNode = new ParameterDeclaration(children[0], (Declaration*) children[1], currentScope, currentLine);
		}
	} else if (definingSymbol == ParameterList::ID) {
		if (production.produces( { ParameterList::ID, ",", ParameterDeclaration::ID })) {
			nonterminalNode = new ParameterList((ParameterList*) children[0], (ParameterDeclaration*) children[2]);
		} else if (production.produces( { ParameterDeclaration::ID })) {
			nonterminalNode = new ParameterList((ParameterDeclaration*) children[0]);
		}
	} else if (definingSymbol == DirectDeclaration::ID) {
		if (production.produces( { "(", Declaration::ID, ")" })) {
			nonterminalNode = new DirectDeclaration((Declaration*) children[1], currentScope, currentLine);
		} else if (production.produces( { "id" })) {
			nonterminalNode = new DirectDeclaration(children[0], currentScope, currentLine);
		} else if (production.produces( { DirectDeclaration::ID, "(", ParameterList::ID, ")" })) {
			nonterminalNode = new DirectDeclaration((DirectDeclaration*) children[0], (ParameterList*) children[2], currentScope,
					currentLine);
			declaredParams = ((DirectDeclaration *) nonterminalNode)->getParams();
		} else if (production.produces( { DirectDeclaration::ID, "[", LogicalOrExpression::ID, "]" })) {
			nonterminalNode = new DirectDeclaration((DirectDeclaration*) children[0], (LogicalOrExpression*) children[2], currentScope,
					currentLine);
		} else if (production.produces( { DirectDeclaration::ID, "(", ")" })) {
			nonterminalNode = new DirectDeclaration((DirectDeclaration*) children[0], currentScope, currentLine);
		}
	} else if (definingSymbol == Pointer::ID) {
		if (production.produces( { Pointer::ID, "*" })) {
			nonterminalNode = new Pointer((Pointer*) children[0]);
		} else if (production.produces( { "*" })) {
			nonterminalNode = new Pointer();
		}
	} else if (definingSymbol == Block::ID)
		nonterminalNode = new Block(children);
	else if (definingSymbol == Declaration::ID) {
		if (production.produces( { Pointer::ID, DirectDeclaration::ID })) {
			nonterminalNode = new Declaration((Pointer*) children[0], (DirectDeclaration*) children[1]);
		} else if (production.produces( { DirectDeclaration::ID })) {
			nonterminalNode = new Declaration((DirectDeclaration*) children[0]);
		}
	} else if (definingSymbol == DeclarationList::ID) {
		if (production.produces( { DeclarationList::ID, ",", Declaration::ID })) {
			nonterminalNode = new DeclarationList((DeclarationList*) children[0], (Declaration*) children[2]);
		} else if (production.produces( { Declaration::ID })) {
			nonterminalNode = new DeclarationList((Declaration*) children[0]);
		}
	} else if (definingSymbol == VariableDeclaration::ID) {
		if (production.produces( { "<type_spec>", DeclarationList::ID, ";" })) {
			nonterminalNode = new VariableDeclaration(children[0], (DeclarationList*) children[1], currentScope, currentLine);
		} else if (production.produces( { "<type_spec>", DeclarationList::ID, "=", AssignmentExpression::ID, ";" })) {
			nonterminalNode = new VariableDeclaration(children[0], (DeclarationList*) children[1], (Expression*) children[3], currentScope,
					currentLine);
		}
	} else if (definingSymbol == FunctionDeclaration::ID) {
		if (production.produces( { "<type_spec>", Declaration::ID, Block::ID })) {
			nonterminalNode = new FunctionDeclaration(children[0], (Declaration*) children[1], children[2], currentScope, currentLine);
		}
	}

	if (!nonterminalNode) {
		noSemanticActionsFoundFor(definingSymbol, production);
	}

	containsSemanticErrors |= nonterminalNode->getErrorFlag();
	syntaxStack.push(nonterminalNode);
}

void SemanticTreeBuilder::makeTerminalNode(const Token& token) {
	TerminalNode *t_node = new TerminalNode(token.id, token.lexeme);
	adjustScope(token.lexeme);
	currentLine = token.line;
	syntaxStack.push(t_node);
}

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

}
