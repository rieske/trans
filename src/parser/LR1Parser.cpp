#include "LR1Parser.h"

#include <bits/shared_ptr_base.h>
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
	currentTokenIsForged = false;
	Token currentToken { scanner.nextToken() };

	for (EVER) {
		parse_state top = parsing_stack.top();
		auto& action = parsingTable->action(top, currentToken.getId());
		switch (action.which()) {
		case 's':
			shift(action.getState(), currentToken, scanner, *semanticAnalyzer);
			continue;
		case 'r':
			reduce(action.getReduction(), currentToken, *semanticAnalyzer);
			continue;
		case 'e':
			error(action, currentToken, scanner, *semanticAnalyzer);
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

void LR1Parser::shift(const parse_state state, Token& currentToken, Scanner& scanner,
		SemanticAnalyzer& semanticAnalyzer) {
	logger << "Stack: " << parsing_stack.top() << "\tpush " << state << "\t\tlookahead: " << currentToken.getLexeme()
			<< "\n";

	parsing_stack.push(state);
	semanticAnalyzer.makeTerminalNode(parsingTable->getTerminalById(currentToken.getId())->getName(), currentToken);
	if (currentTokenIsForged) {
		currentToken = scanner.currentToken();
		currentTokenIsForged = false;
	} else {
		currentToken = scanner.nextToken();
	}
}

void LR1Parser::reduce(const LR1Item& reduction, Token& currentToken, SemanticAnalyzer& semanticAnalyzer) {
	logger << reduction;

	for (unsigned i = reduction.getProduction().size(); i > 0; i--) {
		logger << "Stack: " << parsing_stack.top() << "\tpop " << parsing_stack.top() << "\n";
		parsing_stack.pop();
	}
	parse_state state = parsingTable->go_to(parsing_stack.top(), reduction.getDefiningSymbol());

	logger << "Stack: " << parsing_stack.top() << "\tpush " << state << "\t\tlookahead: " << currentToken.getLexeme()
			<< "\n";

	parsing_stack.push(state);
	semanticAnalyzer.makeNonTerminalNode(reduction.getDefiningSymbol()->getName(), reduction.getProduction().size(),
			reduction.productionStr());
}

void LR1Parser::error(const Action& action, Token& currentToken, Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) {
	semanticAnalyzer.syntaxError();
	action.error(currentToken);
	if (action.getForge() != 0 && !currentTokenIsForged) {
		currentTokenIsForged = true;
		currentToken = {action.getForge(), "", currentToken.line};
		logger << "Inserting " << *action.getExpected() << " into input stream.\n";
	} else {
		parsing_stack.push(action.getState());
		currentToken = scanner.nextToken();
		logger << "Stack: " << parsing_stack.top() << "\tpush " << action.getState() << "\t\tlookahead: "
				<< currentToken.getLexeme() << "\n";
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
