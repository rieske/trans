#include "SemanticSyntaxTreeBuilder.h"

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
//#include "param_decl_node.h"
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

SemanticSyntaxTreeBuilder::SemanticSyntaxTreeBuilder() {
}

SemanticSyntaxTreeBuilder::~SemanticSyntaxTreeBuilder() {
}

void SemanticSyntaxTreeBuilder::makeNonTerminalNode(string left, int childrenCount, string reduction) {
	vector<Node *> children = getChildrenForReduction(childrenCount);

	Node *n_node = NULL;
	if (left == "<u_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<m_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<add_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<s_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<ml_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<eq_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<a_op>")
		n_node = new CarrierNode(left, children);
	else if (left == "<term>")
		n_node = new TermNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<postfix_expr>")
		n_node = new PostfixExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<u_expr>")
		n_node = new UExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<cast_expr>")
		n_node = new CastExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<factor>")
		n_node = new FactorNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<add_expr>")
		n_node = new AddExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<s_expr>")
		n_node = new SExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<ml_expr>")
		n_node = new MLExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<eq_expr>")
		n_node = new EQExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<and_expr>")
		n_node = new AndExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<xor_expr>")
		n_node = new XorExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<or_expr>")
		n_node = new OrExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<log_and_expr>")
		n_node = new LogAndExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<log_expr>")
		n_node = new LogExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<a_expressions>")
		n_node = new AExpressionsNode(left, children, reduction);
	else if (left == "<a_expr>")
		n_node = new AExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<expr>")
		n_node = new ExprNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<jmp_stmt>")
		n_node = new JmpStmtNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<io_stmt>")
		n_node = new IOStmtNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<loop_hdr>")
		n_node = new LoopHdrNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<unmatched>")
		n_node = new UnmatchedNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<matched>")
		n_node = new MatchedNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<stmt>")
		n_node = new CarrierNode(left, children);
	else if (left == "<statements>")
		n_node = new CarrierNode(left, children);
	else if (left == "<param_decl>")
		n_node = new ParamDeclNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<param_list>")
		n_node = new ParamListNode(left, children, reduction);
	else if (left == "<dir_decl>") {
		n_node = new DirDeclNode(left, children, reduction, currentScope, currentLine);
		declaredParams = ((DirDeclNode *) n_node)->getParams();
	} else if (left == "<ptr>")
		n_node = new PtrNode(left, children, reduction);
	else if (left == "<block>")
		n_node = new BlockNode(left, children);
	else if (left == "<decl>")
		n_node = new DeclNode(left, children, reduction);
	else if (left == "<decls>")
		n_node = new DeclsNode(left, children, reduction);
	else if (left == "<type_spec>")
		n_node = new CarrierNode(left, children);
	else if (left == "<var_decl>")
		n_node = new VarDeclNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<func_decl>")
		n_node = new FuncDeclNode(left, children, reduction, currentScope, currentLine);
	else if (left == "<var_decls>")
		n_node = new CarrierNode(left, children);
	else if (left == "<func_decls>")
		n_node = new CarrierNode(left, children);
	else if (left == "<program>")
		n_node = new CarrierNode(left, children);
	else {
		cerr << "Error! Syntax node matching nonterminal " << left << "found!\n";
		return;
	}
	if (true == n_node->getErrorFlag()) {
		syntaxTree->setErrorFlag();
	}
	syntaxStack.push(n_node);
}

void SemanticSyntaxTreeBuilder::makeTerminalNode(string terminal, Token token) {
	TerminalNode *t_node = new TerminalNode(terminal, token.getLexeme());
	adjustScope(token.getLexeme());
	currentLine = token.line;
	syntaxStack.push(t_node);
}

void SemanticSyntaxTreeBuilder::adjustScope(string lexeme) {
	if (lexeme == "{") {
		currentScope = currentScope->newScope();
		if (declaredParams.size()) {
			for (unsigned i = 0; i < declaredParams.size(); i++) {
				currentScope->insertParam(declaredParams[i]->getPlace()->getName(), declaredParams[i]->getPlace()->getBasicType(),
						declaredParams[i]->getPlace()->getExtendedType(), currentLine);
			}
			declaredParams.clear();
		}
	} else if (lexeme == "}") {
		currentScope = currentScope->getOuterScope();
	}
}
