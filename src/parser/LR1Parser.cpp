#include "LR1Parser.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../driver/TranslationUnit.h"
#include "../scanner/Token.h"
#include "../semantic_analyzer/SemanticComponentsFactory.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "../semantic_analyzer/SyntaxTreeBuilder.h"
#include "action.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"
#include "ParsingTable.h"

#define EVER ;;

using std::unique_ptr;
using std::shared_ptr;
using std::cerr;
using std::cout;
using std::endl;

bool LR1Parser::log = false;
ofstream LR1Parser::logfile;

LR1Parser::LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory, Logger logger) :
		parsingTable { parsingTable },
		semanticComponentsFactory { semanticComponentsFactory },
		logger { logger } {
	output = nullptr;
	token = nullptr;
	next_token = nullptr;
	if (log) {
		configure_logging();
	}
	parsing_stack.push(0);
}

LR1Parser::~LR1Parser() {
	if (logfile.is_open()) {
		logfile.close();
	}
	if (output != nullptr) {
		output->close();
		delete output;
	}
}

unique_ptr<SyntaxTree> LR1Parser::parse(TranslationUnit& translationUnit) {
	unique_ptr<SyntaxTreeBuilder> syntaxTreeBuilder { semanticComponentsFactory->newSyntaxTreeBuilder() };
	syntaxTreeBuilder->withSourceFileName(translationUnit.getFileName());
	success = true;
	can_forge = true;
	token = new Token { translationUnit.getNextToken() };
	for (EVER) {
		long top = parsing_stack.top();
		auto& action = parsingTable->action(top, token->getId());
		switch (action.which()) {
		case 's':
			shift(action, translationUnit, *syntaxTreeBuilder);
			continue;
		case 'r':
			reduce(action, *syntaxTreeBuilder);
			continue;
		case 'e':
			error(action, translationUnit);
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
		default:
			throw std::runtime_error("Unrecognized action in parsing table");
		}
	}
	throw std::runtime_error("Parsing failed");
}

void LR1Parser::shift(const Action& shiftAction, TranslationUnit& translationUnit, SyntaxTreeBuilder& syntaxTreeBuilder) {
	logger << "Stack: " << parsing_stack.top() << "\tpush " << shiftAction.getState() << "\t\tlookahead: " << token->getLexeme() << "\n";

	parsing_stack.push(shiftAction.getState());
	if (success) {
		syntaxTreeBuilder.makeTerminalNode(parsingTable->getTerminalById(token->getId())->getName(), *token);
	}
	if (next_token != NULL) {
		token = next_token;
		next_token = NULL;
	} else {
		token = new Token { translationUnit.getNextToken() };
		can_forge = true;
	}
}

void LR1Parser::reduce(const Action& reduceAction, SyntaxTreeBuilder& syntaxTreeBuilder) {
	auto reduction = reduceAction.getReduction();
	logger << reduction;

	for (unsigned i = reduction.getProduction().size(); i > 0; i--) {
		logger << "Stack: " << parsing_stack.top() << "\tpop " << parsing_stack.top() << "\n";
		parsing_stack.pop();
	}
	parse_state state = parsingTable->go_to(parsing_stack.top(), reduction.getDefiningSymbol());

	logger << "Stack: " << parsing_stack.top() << "\tpush " << state << "\t\tlookahead: " << token->getLexeme() << "\n";

	parsing_stack.push(state);
	if (success) {
		syntaxTreeBuilder.makeNonTerminalNode(reduction.getDefiningSymbol()->getName(), reduction.getProduction().size(),
				reduction.productionStr());
	}
}

void LR1Parser::error(const Action& action, TranslationUnit& translationUnit) {
	success = false;
	action.error(token);
	if (action.getForge() != 0 && can_forge) {
		next_token = token;
		token = new Token(action.getForge(), "");
		logger << "Inserting " << action.getExpected() << " into input stream.\n";
		can_forge = false;
	} else {
		parsing_stack.push(action.getState());
		token = new Token { translationUnit.getNextToken() };
		logger << "Stack: " << parsing_stack.top() << "\tpush " << action.getState() << "\t\tlookahead: " << token->getLexeme() << "\n";
	}
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

void LR1Parser::log_syntax_tree(SyntaxTree& syntaxTree) const {
	string outfile = "logs/syntax_tree.xml";
	ofstream xmlfile;
	xmlfile.open(outfile);
	if (!xmlfile.is_open()) {
		cerr << "Unable to create syntax tree xml file! Filename: " << outfile << endl;
		return;
	}
	xmlfile << syntaxTree.asXml();
}
