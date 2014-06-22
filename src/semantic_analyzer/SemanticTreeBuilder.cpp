#include "SemanticTreeBuilder.h"

#include <iostream>
#include <stack>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
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
#include "SyntaxTree.h"
#include "term_node.h"
#include "u_expr_node.h"
#include "unmatched_node.h"
#include "var_decl_node.h"
#include "xor_expr_node.h"

using std::string;
using std::vector;

SemanticTreeBuilder::SemanticTreeBuilder() :
		currentScope { syntaxTree->getSymbolTable() } {
}

SemanticTreeBuilder::~SemanticTreeBuilder() {
}

void SemanticTreeBuilder::makeNonterminalNode(string definingSymbol, Production production) {
	vector<Node *> children = getChildrenForReduction(production.size());

	Node *n_node = NULL;
	if (definingSymbol == "<u_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<m_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<add_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<s_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<ml_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<eq_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<a_op>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<term>")
		n_node = new TermNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<postfix_expr>")
		n_node = new PostfixExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<u_expr>")
		n_node = new UExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<cast_expr>")
		n_node = new CastExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<factor>")
		n_node = new FactorNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<add_expr>")
		n_node = new AddExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<s_expr>")
		n_node = new SExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<ml_expr>")
		n_node = new MLExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<eq_expr>")
		n_node = new EQExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<and_expr>")
		n_node = new AndExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<xor_expr>")
		n_node = new XorExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<or_expr>")
		n_node = new OrExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<log_and_expr>")
		n_node = new LogAndExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<log_expr>")
		n_node = new LogExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<a_expressions>")
		n_node = new AExpressionsNode(definingSymbol, children, production);
	else if (definingSymbol == "<a_expr>")
		n_node = new AExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<expr>")
		n_node = new ExprNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<jmp_stmt>")
		n_node = new JmpStmtNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<io_stmt>")
		n_node = new IOStmtNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<loop_hdr>")
		n_node = new LoopHdrNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<unmatched>")
		n_node = new UnmatchedNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<matched>")
		n_node = new MatchedNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<stmt>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<statements>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<param_decl>")
		n_node = new ParamDeclNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<param_list>")
		n_node = new ParamListNode(definingSymbol, children, production);
	else if (definingSymbol == "<dir_decl>") {
		n_node = new DirDeclNode(definingSymbol, children, production, currentScope, currentLine);
		declaredParams = ((DirDeclNode *) n_node)->getParams();
	} else if (definingSymbol == "<ptr>")
		n_node = new PtrNode(definingSymbol, children, production);
	else if (definingSymbol == "<block>")
		n_node = new BlockNode(definingSymbol, children);
	else if (definingSymbol == "<decl>")
		n_node = new DeclNode(definingSymbol, children, production);
	else if (definingSymbol == "<decls>")
		n_node = new DeclsNode(definingSymbol, children, production);
	else if (definingSymbol == "<type_spec>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<var_decl>")
		n_node = new VarDeclNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<func_decl>")
		n_node = new FuncDeclNode(definingSymbol, children, production, currentScope, currentLine);
	else if (definingSymbol == "<var_decls>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<func_decls>")
		n_node = new CarrierNode(definingSymbol, children);
	else if (definingSymbol == "<program>")
		n_node = new CarrierNode(definingSymbol, children);
	else {
		cerr << "Error! Syntax node matching nonterminal " << definingSymbol << "found!\n";
		return;
	}
	if (true == n_node->getErrorFlag()) {
		syntaxTree->setErrorFlag();
	}
	syntaxStack.push(n_node);
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
		if (declaredParams.size()) {
			for (unsigned i = 0; i < declaredParams.size(); i++) {
				currentScope->insertParam(declaredParams[i]->getPlace()->getName(),
						declaredParams[i]->getPlace()->getBasicType(), declaredParams[i]->getPlace()->getExtendedType(),
						currentLine);
			}
			declaredParams.clear();
		}
	} else if (lexeme == "}") {
		currentScope = currentScope->getOuterScope();
	}
}
