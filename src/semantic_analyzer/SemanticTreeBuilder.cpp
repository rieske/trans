#include "SemanticTreeBuilder.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"
#include "AbstractSyntaxTree.h"
#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclaration.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "BitwiseExpression.h"
#include "Block.h"
#include "ComparisonExpression.h"
#include "DeclarationList.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FunctionCall.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "Identifier.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "IOStatement.h"
#include "JumpStatement.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "NoArgFunctionCall.h"
#include "NoArgFunctionDeclaration.h"
#include "ParameterList.h"
#include "Pointer.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ReturnStatement.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"
#include "WhileLoopHeader.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

SemanticTreeBuilder::SemanticTreeBuilder() :
		symbolTable { new SymbolTable() },
		context { symbolTable } {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(string definingSymbol, parser::Production production) {
	syntaxNodeFactory.updateContext(definingSymbol, production.producedSequence(), context);

	std::cerr << definingSymbol << " ::= ";
	for (const auto producedSymbol : production) {
		std::cerr << producedSymbol->getDefinition() << " ";
	}
	std::cerr << std::endl;

	vector<AbstractSyntaxTreeNode*> children = getChildrenForReduction(production.size());
	NonterminalNode* nonterminalNode { nullptr };
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
			TerminalSymbol terminal = context.popTerminal();
			nonterminalNode = new Term(terminal, context.scope(), context.line());
		}
	} else if (definingSymbol == PostfixExpression::ID) {
		if (production.produces( { PostfixExpression::ID, "[", Expression::ID, "]" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new ArrayAccess(std::unique_ptr<Expression> { (Expression*) children[0] }, std::unique_ptr<Expression> {
					(Expression*) children[2] }, context.scope(), context.line());
		} else if (production.produces( { PostfixExpression::ID, "(", AssignmentExpressionList::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new FunctionCall(std::unique_ptr<Expression> { (Expression*) children[0] },
					std::unique_ptr<AssignmentExpressionList> { (AssignmentExpressionList*) children[2] }, context.scope(), context.line());
		} else if (production.produces( { PostfixExpression::ID, "(", ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new NoArgFunctionCall(std::unique_ptr<Expression> { (Expression*) children[0] }, context.scope(),
					context.line());
		} else if (production.produces( { PostfixExpression::ID, "++" }) || production.produces( { PostfixExpression::ID, "--" })) {
			nonterminalNode = new PostfixExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					context.scope());
		} else if (production.produces( { Term::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == UnaryExpression::ID) {
		if (production.produces( { "++", UnaryExpression::ID }) || production.produces( { "--", UnaryExpression::ID })) {
			nonterminalNode = new PrefixExpression(context.popTerminal(), std::unique_ptr<Expression> { (Expression*) children[1] },
					context.scope());
		} else if (production.produces( { "<u_op>", TypeCast::ID })) {
			nonterminalNode = new UnaryExpression(context.popTerminal(), std::unique_ptr<Expression> { (Expression*) children[1] },
					context.scope());
		} else if (production.produces( { PostfixExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == TypeCast::ID) {
		if (production.produces( { "(", "<type_spec>", ")", TypeCast::ID })) {
			context.popTerminal();
			nonterminalNode = new TypeCast(context.popTerminal(), std::unique_ptr<Expression> { (Expression*) children[3] },
					context.scope());
			context.popTerminal();
		} else if (production.produces( { "(", "<type_spec>", Pointer::ID, ")", TypeCast::ID })) {
			context.popTerminal();
			nonterminalNode = new PointerCast(context.popTerminal(), std::unique_ptr<Pointer> { (Pointer*) children[2] },
					std::unique_ptr<Expression> { (Expression*) children[4] }, context.scope());
			context.popTerminal();
		} else if (production.produces( { UnaryExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == ArithmeticExpression::MULTIPLICATION) {
		if (production.produces( { ArithmeticExpression::MULTIPLICATION, "<m_op>", TypeCast::ID })) {
			nonterminalNode = new ArithmeticExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { TypeCast::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == ArithmeticExpression::ADDITION) {
		if (production.produces( { ArithmeticExpression::MULTIPLICATION })) {
			nonterminalNode = (Expression*) children[0];
		} else if (production.produces( { ArithmeticExpression::ADDITION, "<add_op>", ArithmeticExpression::MULTIPLICATION })) {
			nonterminalNode = new ArithmeticExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		}
	} else if (definingSymbol == ShiftExpression::ID) {
		if (production.produces( { ShiftExpression::ID, "<s_op>", ArithmeticExpression::ADDITION })) {
			nonterminalNode = new ShiftExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { ArithmeticExpression::ADDITION })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == ComparisonExpression::DIFFERENCE) {
		if (production.produces( { ComparisonExpression::DIFFERENCE, "<ml_op>", ShiftExpression::ID })) {
			nonterminalNode = new ComparisonExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { ShiftExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == ComparisonExpression::EQUALITY) {
		if (production.produces( { ComparisonExpression::EQUALITY, "<eq_op>", ComparisonExpression::DIFFERENCE })) {
			nonterminalNode = new ComparisonExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { ComparisonExpression::DIFFERENCE })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == BitwiseExpression::AND) {
		if (production.produces( { BitwiseExpression::AND, "&", ComparisonExpression::EQUALITY })) {
			nonterminalNode = new BitwiseExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { ComparisonExpression::EQUALITY })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == BitwiseExpression::XOR) {
		if (production.produces( { BitwiseExpression::XOR, "^", BitwiseExpression::AND })) {
			nonterminalNode = new BitwiseExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { BitwiseExpression::AND })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == BitwiseExpression::OR) {
		if (production.produces( { BitwiseExpression::OR, "|", BitwiseExpression::XOR })) {
			nonterminalNode = new BitwiseExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { BitwiseExpression::XOR })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == LogicalAndExpression::ID) {
		if (production.produces( { LogicalAndExpression::ID, "&&", BitwiseExpression::OR })) {
			context.popTerminal();
			nonterminalNode = new LogicalAndExpression(std::unique_ptr<Expression> { (Expression*) children[0] },
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope(), context.line());
		} else if (production.produces( { BitwiseExpression::OR })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == LogicalOrExpression::ID) {
		if (production.produces( { LogicalOrExpression::ID, "||", Expression::ID })) {
			context.popTerminal();
			nonterminalNode = new LogicalOrExpression(std::unique_ptr<Expression> { (Expression*) children[0] },
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope(), context.line());
		} else if (production.produces( { LogicalAndExpression::ID })) {
			((Expression*) children[0])->backpatch();
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == AssignmentExpressionList::ID) {
		if (production.produces( { AssignmentExpression::ID })) {
			nonterminalNode = new AssignmentExpressionList(std::unique_ptr<Expression> { (Expression*) children[0] });
		} else if (production.produces( { AssignmentExpressionList::ID, ",", AssignmentExpression::ID })) {
			context.popTerminal();
			((AssignmentExpressionList*) children[0])->addExpression(std::unique_ptr<Expression> { (AssignmentExpression*) children[2] });
			nonterminalNode = (AssignmentExpressionList*) children[0];
		}
	} else if (definingSymbol == AssignmentExpression::ID) {
		if (production.produces( { LogicalOrExpression::ID })) {
			((LogicalOrExpression*) children[0])->backpatch();
			nonterminalNode = (LogicalOrExpression*) children[0];
		} else if (production.produces( { UnaryExpression::ID, "<a_op>", AssignmentExpression::ID })) {
			nonterminalNode = new AssignmentExpression(std::unique_ptr<Expression> { (Expression*) children[0] }, context.popTerminal(),
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		}
	} else if (definingSymbol == Expression::ID) {
		if (production.produces( { Expression::ID, ",", AssignmentExpression::ID })) {
			context.popTerminal();
			nonterminalNode = new ExpressionList(std::unique_ptr<Expression> { (Expression*) children[0] }, std::unique_ptr<Expression> {
					(Expression*) children[2] }, context.scope(), context.line());
		} else if (production.produces( { AssignmentExpression::ID })) {
			nonterminalNode = (Expression*) children[0];
		}
	} else if (definingSymbol == JumpStatement::ID) {
		if (production.produces( { "continue", ";" }) || production.produces( { "break", ";" })) {
			nonterminalNode = new JumpStatement(context.popTerminal());
			context.popTerminal();
		} else if (production.produces( { "return", Expression::ID, ";" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new ReturnStatement(std::unique_ptr<Expression> { (Expression*) children[1] });
		}
	} else if (definingSymbol == IOStatement::ID) {
		if (production.produces( { "output", Expression::ID, ";" }) || production.produces( { "input", Expression::ID, ";" })) {
			context.popTerminal();
			nonterminalNode = new IOStatement(context.popTerminal(), std::unique_ptr<Expression> { (Expression*) children[1] });
		}
	} else if (definingSymbol == LoopHeader::ID) {
		if (production.produces( { "while", "(", Expression::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new WhileLoopHeader(std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope());
		} else if (production.produces( { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new ForLoopHeader(std::unique_ptr<Expression> { (Expression*) children[2] }, std::unique_ptr<Expression> {
					(Expression*) children[4] }, std::unique_ptr<Expression> { (Expression*) children[6] }, context.scope());
		}
	} else if (definingSymbol == "<unmatched>") {
		if (production.produces( { "if", "(", Expression::ID, ")", "<stmt>" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new IfStatement(std::unique_ptr<Expression> { (Expression*) children[2] },
					std::unique_ptr<AbstractSyntaxTreeNode> { children[4] }, context.scope());
		} else if (production.produces( { "if", "(", Expression::ID, ")", "<matched>", "else", "<unmatched>" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new IfElseStatement(std::unique_ptr<Expression> { (Expression*) children[2] },
					std::unique_ptr<AbstractSyntaxTreeNode> { children[4] }, std::unique_ptr<AbstractSyntaxTreeNode> { children[6] },
					context.scope());
		} else if (production.produces( { LoopHeader::ID, "<unmatched>" })) {
			nonterminalNode = new LoopStatement(std::unique_ptr<LoopHeader> { (LoopHeader*) children[0] },
					std::unique_ptr<AbstractSyntaxTreeNode> { children[1] }, context.scope());
		}
	} else if (definingSymbol == "<matched>") {
		if (production.produces( { Expression::ID, ";" })) {
			context.popTerminal();
			nonterminalNode = (Expression*) children[0];
		} else if (production.produces( { IOStatement::ID }) || production.produces( { Block::ID }) || production.produces( { ";" })
				|| production.produces( { JumpStatement::ID })) {
			if (production.produces( { ";" })) {
				context.popTerminal();
			}
			nonterminalNode = (NonterminalNode*) children[0];
		} else if (production.produces( { "if", "(", Expression::ID, ")", "<matched>", "else", "<matched>" })) {
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new IfElseStatement(std::unique_ptr<Expression> { (Expression*) children[2] },
					std::unique_ptr<AbstractSyntaxTreeNode> { children[4] }, std::unique_ptr<AbstractSyntaxTreeNode> { children[6] },
					context.scope());
		} else if (production.produces( { LoopHeader::ID, "<matched>" })) {
			nonterminalNode = new LoopStatement(std::unique_ptr<LoopHeader> { (LoopHeader*) children[0] },
					std::unique_ptr<AbstractSyntaxTreeNode> { children[1] }, context.scope());
		}
	} else if (definingSymbol == ParameterDeclaration::ID) {
		if (production.produces( { "<type_spec>", Declaration::ID })) {
			nonterminalNode = new ParameterDeclaration(context.popTerminal(), std::unique_ptr<Declaration> { (Declaration*) children[1] },
					context.scope());
		}
	} else if (definingSymbol == ParameterList::ID) {
		if (production.produces( { ParameterList::ID, ",", ParameterDeclaration::ID })) {
			context.popTerminal();
			((ParameterList*) children[0])->addParameterDeclaration(std::unique_ptr<ParameterDeclaration> {
					(ParameterDeclaration*) children[2] });
			nonterminalNode = (ParameterList*) children[0];
		} else if (production.produces( { ParameterDeclaration::ID })) {
			nonterminalNode = new ParameterList(std::unique_ptr<ParameterDeclaration> { (ParameterDeclaration*) children[0] });
		}
	} else if (definingSymbol == "<dir_decl>") {
		if (production.produces( { "(", Declaration::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = (Declaration*) children[1];
		} else if (production.produces( { "id" })) {
			nonterminalNode = new Identifier(context.popTerminal());
		} else if (production.produces( { "<dir_decl>", "(", ParameterList::ID, ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new FunctionDeclaration(std::unique_ptr<Declaration> { (Declaration*) children[0] },
					std::unique_ptr<ParameterList> { (ParameterList*) children[2] }, context.scope(), context.line());
			declaredParams = ((FunctionDeclaration *) nonterminalNode)->getParams();
		} else if (production.produces( { "<dir_decl>", "[", LogicalOrExpression::ID, "]" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new ArrayDeclaration(std::unique_ptr<Declaration> { (Declaration*) children[0] },
					std::unique_ptr<Expression> { (Expression*) children[2] }, context.scope(), context.line());
		} else if (production.produces( { "<dir_decl>", "(", ")" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new NoArgFunctionDeclaration(std::unique_ptr<Declaration> { (Declaration*) children[0] }, context.scope(),
					context.line());
		}
	} else if (definingSymbol == Pointer::ID) {
		if (production.produces( { Pointer::ID, "*" })) {
			context.popTerminal();
			((Pointer*) children[0])->dereference();
			nonterminalNode = (Pointer*) children[0];
		} else if (production.produces( { "*" })) {
			context.popTerminal();
			nonterminalNode = new Pointer();
		}
	} else if (definingSymbol == Block::ID) {
		context.popTerminal();
		context.popTerminal();
		nonterminalNode = new Block(children);
	} else if (definingSymbol == Declaration::ID) {
		if (production.produces( { Pointer::ID, "<dir_decl>" })) {
			((Declaration*) children[1])->dereference(((Pointer*) children[0])->getType());
			nonterminalNode = (Declaration*) children[1];
		} else if (production.produces( { "<dir_decl>" })) {
			nonterminalNode = (Declaration*) children[0];
		}
	} else if (definingSymbol == DeclarationList::ID) {
		if (production.produces( { DeclarationList::ID, ",", Declaration::ID })) {
			context.popTerminal();
			((DeclarationList*) children[0])->addDeclaration(std::unique_ptr<Declaration> { (Declaration*) children[2] });
			nonterminalNode = (DeclarationList*) children[0];
		} else if (production.produces( { Declaration::ID })) {
			nonterminalNode = new DeclarationList(std::unique_ptr<Declaration> { (Declaration*) children[0] });
		}
	} else if (definingSymbol == VariableDeclaration::ID) {
		if (production.produces( { "<type_spec>", DeclarationList::ID, ";" })) {
			context.popTerminal();
			nonterminalNode = new VariableDeclaration(context.popTerminal(), std::unique_ptr<DeclarationList> {
					(DeclarationList*) children[1] }, context.scope());
		} else if (production.produces( { "<type_spec>", DeclarationList::ID, "=", AssignmentExpression::ID, ";" })) {
			context.popTerminal();
			context.popTerminal();
			nonterminalNode = new VariableDefinition(
					std::unique_ptr<VariableDeclaration> { new VariableDeclaration(context.popTerminal(), std::unique_ptr<DeclarationList> {
							(DeclarationList*) children[1] }, context.scope()) }, std::unique_ptr<Expression> { (Expression*) children[3] },
					context.scope(), context.line());
		}
	} else if (definingSymbol == FunctionDefinition::ID) {
		if (production.produces( { "<type_spec>", Declaration::ID, Block::ID })) {
			nonterminalNode = new FunctionDefinition(context.popTerminal(), std::unique_ptr<Declaration> { (Declaration*) children[1] },
					std::unique_ptr<AbstractSyntaxTreeNode> { children[2] }, context.scope());
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
	context.setLine(line);
	// XXX: create dummy until the context separation is resolved
	NonterminalNode *t_node = new NonterminalNode(value);
	syntaxStack.push(t_node);

	context.pushTerminal( { type, value, line });
}

// FIXME: incorporate scope data into the AST
void SemanticTreeBuilder::adjustScope(string lexeme) {
	if (lexeme == "{") {
		context.innerScope();
		for (const auto declaredParam : declaredParams) {
			context.scope()->insertParam(declaredParam->getPlace()->getName(), declaredParam->getPlace()->getBasicType(),
					declaredParam->getPlace()->getExtendedType(), context.line());
		}
		declaredParams.clear();
	} else if (lexeme == "}") {
		context.outerScope();
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
