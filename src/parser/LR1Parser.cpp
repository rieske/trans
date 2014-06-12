#include "LR1Parser.h"

#include <stddef.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../scanner/Scanner.h"
#include "../scanner/Token.h"
#include "../semantic_analyzer/SemanticComponentsFactory.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "action.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"

#define EVER ;;

using std::unique_ptr;
using std::shared_ptr;
using std::cerr;
using std::cout;
using std::endl;

bool LR1Parser::log = false;

LR1Parser::LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory, Logger logger) :
		parsingTable { parsingTable },
		semanticComponentsFactory { semanticComponentsFactory },
		logger { logger } {
	parsing_stack.push(0);
}

LR1Parser::~LR1Parser() {
}

unique_ptr<SyntaxTree> LR1Parser::parse(Scanner& scanner) {
	unique_ptr<SemanticAnalyzer> semanticAnalyzer { semanticComponentsFactory->newSemanticAnalyzer() };
	TokenStream tokenStream { &scanner };
	tokenStream.nextToken();

	for (EVER) {
		parse_state currentState = parsing_stack.top();
		auto& action = parsingTable->action(currentState, tokenStream.getCurrentToken().id);
		switch (action.which()) {
		case 's':
			shift(action.getState(), tokenStream, *semanticAnalyzer);
			continue;
		case 'r':
			reduce(action.getReduction(), tokenStream, *semanticAnalyzer);
			continue;
		case 'e':
			error(action, tokenStream, *semanticAnalyzer);
			continue;
		case 'a': {      // accept
			unique_ptr<SyntaxTree> syntaxTree = semanticAnalyzer->build();
			if (log) {
				log_syntax_tree(*syntaxTree);
				syntaxTree->printTables();
				syntaxTree->logCode();
			}
			return syntaxTree;
		}
		default:
			throw std::runtime_error("Unrecognized action in parsing table");
		}
	}
	throw std::runtime_error("Parsing failed");
}

void LR1Parser::shift(const parse_state state, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) {
	logger << "Stack: " << parsing_stack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme << "\n";

	parsing_stack.push(state);
	semanticAnalyzer.makeTerminalNode(tokenStream.getCurrentToken());
	tokenStream.nextToken();
}

void LR1Parser::reduce(const LR1Item& reduction, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) {
	logger << reduction;

	for (size_t i = reduction.getProduction().size(); i > 0; i--) {
		logger << "Stack: " << parsing_stack.top() << "\tpop " << parsing_stack.top() << "\n";
		parsing_stack.pop();
	}
	parse_state state = parsingTable->go_to(parsing_stack.top(), reduction.getDefiningSymbol());

	logger << "Stack: " << parsing_stack.top() << "\tpush " << state << "\t\tlookahead: " << tokenStream.getCurrentToken().lexeme << "\n";

	parsing_stack.push(state);
	semanticAnalyzer.makeNonTerminalNode(reduction.getDefiningSymbol()->getName(), reduction.getProduction().size(),
			reduction.productionStr());
}

void LR1Parser::error(const Action& action, TokenStream& tokenStream, SemanticAnalyzer& semanticAnalyzer) {
	semanticAnalyzer.syntaxError();
	action.error(tokenStream.getCurrentToken());
	if (!action.getForge().empty() && !tokenStream.currentTokenIsForged()) {
		tokenStream.forgeToken( { action.getForge(), action.getForge(), tokenStream.getCurrentToken().line });
		logger << "Inserting " << *action.getExpected() << " into input stream.\n";
	} else {
		parsing_stack.push(action.getState());
		tokenStream.nextToken();
		logger << "Stack: " << parsing_stack.top() << "\tpush " << action.getState() << "\t\tlookahead: "
				<< tokenStream.getCurrentToken().lexeme << "\n";
	}
}

void LR1Parser::set_logging() {
	log = true;
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
