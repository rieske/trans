#include "NonterminalNodeFactory.h"

#include "../parser/NonterminalNode.h"

using std::unique_ptr;

namespace semantic_analyzer {

NonterminalNodeFactory::NonterminalNodeFactory() {
	// TODO Auto-generated constructor stub

}

NonterminalNodeFactory::~NonterminalNodeFactory() {
	// TODO Auto-generated destructor stub
}

unique_ptr<NonterminalNode> NonterminalNodeFactory::makeNonterminalNode(std::string definingSymbol) {
	NonterminalNode* nonterminalNode { nullptr };
	/*if (definingSymbol == "<u_op>")
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
	 throw std::runtime_error { "Error! Syntax node matching nonterminal " + definingSymbol + "found!\n" };
	 }*/
	return std::unique_ptr<NonterminalNode> { nonterminalNode };
}

} /* namespace semantic_analyzer */
