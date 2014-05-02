#include "LR1Parser.h"

#include <cstdlib>
#include <fstream>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "../driver/TranslationUnit.h"
#include "../scanner/Token.h"
#include "../semantic_analyzer/a_expr_node.h"
#include "../semantic_analyzer/a_expressions_node.h"
#include "../semantic_analyzer/add_expr_node.h"
#include "../semantic_analyzer/and_expr_node.h"
#include "../semantic_analyzer/block_node.h"
#include "../semantic_analyzer/cast_expr_node.h"
#include "../semantic_analyzer/decl_node.h"
#include "../semantic_analyzer/decls_node.h"
#include "../semantic_analyzer/dir_decl_node.h"
#include "../semantic_analyzer/eq_expr_node.h"
#include "../semantic_analyzer/factor_node.h"
#include "../semantic_analyzer/func_decl_node.h"
#include "../semantic_analyzer/io_stmt_node.h"
#include "../semantic_analyzer/jmp_stmt_node.h"
#include "../semantic_analyzer/log_and_expr_node.h"
#include "../semantic_analyzer/loop_hdr_node.h"
#include "../semantic_analyzer/matched_node.h"
#include "../semantic_analyzer/ml_expr_node.h"
#include "../semantic_analyzer/or_expr_node.h"
#include "../semantic_analyzer/param_list_node.h"
#include "../semantic_analyzer/postfix_expr_node.h"
#include "../semantic_analyzer/ptr_node.h"
#include "../semantic_analyzer/s_expr_node.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "../semantic_analyzer/SemanticComponentsFactory.h"
#include "../semantic_analyzer/SyntaxTreeBuilder.h"
#include "../semantic_analyzer/term_node.h"
#include "../semantic_analyzer/u_expr_node.h"
#include "../semantic_analyzer/unmatched_node.h"
#include "../semantic_analyzer/var_decl_node.h"
#include "../semantic_analyzer/xor_expr_node.h"
#include "action.h"
#include "ParsingTable.h"
#include "rule.h"
#include "terminal_node.h"

#define EVER ;;

using std::unique_ptr;
using std::cerr;
using std::cout;
using std::endl;

bool LR1Parser::log = false;
ofstream LR1Parser::logfile;

LR1Parser::LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory) :
		parsingTable { parsingTable },
		semanticComponentsFactory { semanticComponentsFactory } {
	output = NULL;
	token = NULL;
	next_token = NULL;
	if (log) {
		configure_logging();
	}
	parsing_stack.push(0);
}

LR1Parser::~LR1Parser() {
	if (logfile.is_open()) {
		logfile.close();
	}
}

unique_ptr<SyntaxTree> LR1Parser::parse(TranslationUnit& translationUnit) {
	unique_ptr<SyntaxTreeBuilder> syntaxTreeBuilder { semanticComponentsFactory->newSyntaxTreeBuilder() };
	syntaxTreeBuilder->withSourceFileName(translationUnit.getFileName());
	success = true;
	can_forge = true;
	token = new Token { translationUnit.getNextToken() };
	Action *action = NULL;
	for (EVER) {
		long top = parsing_stack.top();
		// FIXME: adjust parsing table
		if (token->getId() != 0) {
			action = parsingTable->action(top, token->getId());
		} else {
			action = parsingTable->action(top, -1);
		}
		if (action != NULL) {
			switch (action->which()) {
			case 's':
				shift(action, translationUnit, *syntaxTreeBuilder);
				continue;
			case 'a':       // accept
				if (success) {
					unique_ptr<SyntaxTree> syntaxTree = syntaxTreeBuilder->build();
					if (log) {
						log_syntax_tree(*syntaxTree);
						syntaxTree->printTables();
						syntaxTree->logCode();
					}
					return syntaxTree;
				}
				throw std::runtime_error("Parsing failed");
			case 'r':
				reduce(action, *syntaxTreeBuilder);
				action = NULL;
				continue;
			case 'e':
				success = false;
				error(action, translationUnit);
				action = NULL;
				continue;
			default:
				action->error(token);
				cerr << "error" << endl;
				if (output != NULL) {
					output->close();
					delete output;
				}
				throw std::runtime_error("Unrecognized action");
			}
		} else {
			throw std::runtime_error("NULL entry in action table!");
		}
	}
	if (output != NULL) {
		output->close();
		delete output;
	}
	throw std::runtime_error("Parsing failed");
}

void LR1Parser::set_logging(const char *lf) {
	log = true;
	string logpath = "logs/";
	logpath += lf;
	logfile.open(logpath.c_str());
	if (!logfile.is_open()) {
		cerr << "Unable to open log file for writing! Filename: " << lf << endl;
		log = false;
	}
}

void LR1Parser::configure_logging() {
	const char *outfile = "logs/parser.out";
	output = new ofstream;
	output->open(outfile);
	if (!output->is_open()) {
		cerr << "Unable to create parser output file! Filename: " << outfile << endl;
		delete output;
		output = NULL;
	}
}

void LR1Parser::shift(Action *action, TranslationUnit& translationUnit, SyntaxTreeBuilder& syntaxTreeBuilder) {
	if (log) {
		*output << "Stack: " << parsing_stack.top() << "\tpush " << action->getState() << "\t\tlookahead: " << token->getLexeme() << endl;
	}
	parsing_stack.push(action->getState());
	if (success) {
		syntaxTreeBuilder.makeTerminalNode(parsingTable->getTerminalById(token->getId()), *token);
	}
	if (next_token != NULL) {
		token = next_token;
		next_token = NULL;
	} else {
		token = new Token { translationUnit.getNextToken() };
		can_forge = true;
	}
	action = NULL;
}

void LR1Parser::reduce(Action *action, SyntaxTreeBuilder& syntaxTreeBuilder) {
	Rule *reduction = NULL;
	Action *gt = NULL;
	reduction = action->getReduction();
	if (reduction != NULL) {
		if (log) {
			reduction->log(*output);
		}
	} else {
		throw std::runtime_error("NULL reduction found!");
	}
	for (unsigned i = reduction->getRight()->size(); i > 0; i--) {
		if (log) {
			*output << "Stack: " << parsing_stack.top() << "\tpop " << parsing_stack.top() << "\t\t";
			if (i > 1) {
				*output << endl;
			}
		}
		parsing_stack.pop();
	}
	gt = parsingTable->go_to(parsing_stack.top(), *reduction->getLeft());
	if (gt != NULL) {
		if (log) {
			*output << "Stack: " << parsing_stack.top() << "\tpush " << gt->getState() << "\t\tlookahead: " << token->getLexeme() << endl;
		}
		if (success) {
			syntaxTreeBuilder.makeNonTerminalNode(*reduction->getLeft(), reduction->getRight()->size(), reduction->rightStr());
		}
		parsing_stack.push(gt->getState());
		action = NULL;
	} else {
		throw std::runtime_error("NULL goto entry found!");
	}
}

void LR1Parser::error(Action *action, TranslationUnit& translationUnit) {
	if (action->error(token)) {
		if (action->getForge() != 0 && can_forge) {
			next_token = token;
			token = new Token(action->getForge(), "");
			if (log) {
				cerr << "Inserting " << action->getExpected() << " into input stream.\n";
			}
			can_forge = false;
		} else {
			parsing_stack.push(action->getState());
			token = new Token { translationUnit.getNextToken() };
			if (log) {
				cerr << "Stack: " << parsing_stack.top() << "\tpush " << action->getState() << "\t\tlookahead: " << token->getLexeme()
						<< endl;
			}
		}
	} else {
		throw std::runtime_error("Parsing failed!");
	}
}

void LR1Parser::log_syntax_tree(SyntaxTree& syntaxTree) const {
	const char *outfile = "logs/syntax_tree.xml";
	ofstream xmlfile;
	xmlfile.open(outfile);
	if (!xmlfile.is_open()) {
		cerr << "Unable to create syntax tree xml file! Filename: " << outfile << endl;
		return;
	}
	xmlfile << syntaxTree.asXml();
}
