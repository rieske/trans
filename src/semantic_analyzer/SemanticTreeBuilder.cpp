#include "SemanticTreeBuilder.h"

#include <iostream>
#include <stack>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../parser/SyntaxTree.h"
#include "../parser/terminal_node.h"
#include "../scanner/Token.h"
#include "a_expr_node.h"
#include "a_expressions_node.h"
#include "add_expr_node.h"
#include "and_expr_node.h"
#include "block_node.h"
#include "cast_expr_node.h"
#include "decl_node.h"
#include "decls_node.h"
#include "dir_decl_node.h"
#include "eq_expr_node.h"
#include "factor_node.h"
#include "func_decl_node.h"
#include "io_stmt_node.h"
#include "jmp_stmt_node.h"
#include "log_and_expr_node.h"
#include "loop_hdr_node.h"
#include "matched_node.h"
#include "ml_expr_node.h"
#include "or_expr_node.h"
#include "param_list_node.h"
#include "postfix_expr_node.h"
#include "ptr_node.h"
#include "s_expr_node.h"
#include "term_node.h"
#include "u_expr_node.h"
#include "unmatched_node.h"
#include "var_decl_node.h"
#include "xor_expr_node.h"

using std::string;
using std::vector;

namespace semantic_analyzer {

SemanticTreeBuilder::SemanticTreeBuilder() :
		symbolTable { new SymbolTable() },
		currentScope { symbolTable } {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(string definingSymbol, Production production) {
	vector<ParseTreeNode *> children = getChildrenForReduction(production.size());

	ParseTreeNode *nonterminalNode { nullptr };
	if (definingSymbol == "<u_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<m_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<add_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<s_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<ml_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<eq_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<a_op>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<term>")
		nonterminalNode = new TermNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<postfix_expr>")
		nonterminalNode = new PostfixExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<u_expr>")
		nonterminalNode = new UExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<cast_expr>")
		nonterminalNode = new CastExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<factor>")
		nonterminalNode = new FactorNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<add_expr>")
		nonterminalNode = new AddExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<s_expr>")
		nonterminalNode = new SExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<ml_expr>")
		nonterminalNode = new MLExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<eq_expr>")
		nonterminalNode = new EQExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<and_expr>")
		nonterminalNode = new AndExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<xor_expr>")
		nonterminalNode = new XorExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<or_expr>")
		nonterminalNode = new OrExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<log_and_expr>")
		nonterminalNode = new LogAndExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<log_expr>")
		nonterminalNode = new LogExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<a_expressions>")
		nonterminalNode = new AExpressionsNode(definingSymbol, children, production);
	else if (definingSymbol == "<a_expr>")
		nonterminalNode = new AExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<expr>")
		nonterminalNode = new ExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<jmp_stmt>")
		nonterminalNode = new JmpStmtNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<io_stmt>")
		nonterminalNode = new IOStmtNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<loop_hdr>")
		nonterminalNode = new LoopHdrNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<unmatched>")
		nonterminalNode = new UnmatchedNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<matched>")
		nonterminalNode = new MatchedNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<stmt>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<statements>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<param_decl>")
		nonterminalNode = new ParamDeclNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<param_list>")
		nonterminalNode = new ParamListNode(definingSymbol, children, production);
	else if (definingSymbol == "<dir_decl>") {
		nonterminalNode = new DirDeclNode(definingSymbol, children, production, currentScope, currentLine);
		declaredParams = ((DirDeclNode *) nonterminalNode)->getParams();
	} else if (definingSymbol == "<ptr>")
		nonterminalNode = new PtrNode(definingSymbol, children, production);
	else if (definingSymbol == "<block>")
		nonterminalNode = new BlockNode(definingSymbol, children);
	else if (definingSymbol == "<decl>")
		nonterminalNode = new DeclNode(definingSymbol, children, production);
	else if (definingSymbol == "<decls>")
		nonterminalNode = new DeclsNode(definingSymbol, children, production);
	else if (definingSymbol == "<type_spec>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<var_decl>")
		nonterminalNode = new VarDeclNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<func_decl>")
		nonterminalNode = new FuncDeclNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<var_decls>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<func_decls>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<program>")
		nonterminalNode = new CarrierNode(definingSymbol, children);
	else {
		cerr << "Error! Syntax node matching nonterminal " << definingSymbol << "found!\n";
		return;
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

SyntaxTree SemanticTreeBuilder::build() {
	if (containsSemanticErrors) {
		throw std::runtime_error("Compilation failed with semantic errors!");
	}
	return {syntaxStack.top(), symbolTable};
}

}
