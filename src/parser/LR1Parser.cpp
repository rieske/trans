#include "LR1Parser.h"

#include <fstream>
#include <iostream>
#include <stack>
#include <string>

#include "../scanner/Token.h"
#include "Action.h"
#include "LookaheadActionTable.h"
#include "ParsingTable.h"
#include "SyntaxTree.h"
#include "TokenStream.h"

using std::unique_ptr;

bool LR1Parser::log = false;

LR1Parser::LR1Parser(ParsingTable* parsingTable) :
		parsingTable { parsingTable } {
}

LR1Parser::~LR1Parser() {
}

unique_ptr<SyntaxTree> LR1Parser::parse(Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) {
	TokenStream tokenStream { &scanner };
	tokenStream.nextToken();

	std::stack<parse_state> parsingStack;
	parsingStack.push(0);
	unique_ptr<SyntaxTree> syntaxTree { nullptr };
	while (!syntaxTree) {
		auto& parseAction = parsingTable->action(parsingStack.top(), tokenStream.getCurrentToken().id);
		syntaxTree = parseAction.perform(parsingStack, tokenStream, semanticAnalyzer);
	}
	if (log) {
		log_syntax_tree(*syntaxTree);
		syntaxTree->printTables();
		syntaxTree->logCode();
	}
	return syntaxTree;
}

void LR1Parser::set_logging() {
	log = true;
}

void LR1Parser::log_syntax_tree(SyntaxTree& syntaxTree) const {
	string outfile = "logs/syntax_tree.xml";
	std::ofstream xmlfile;
	xmlfile.open(outfile);
	if (!xmlfile.is_open()) {
		std::cerr << "Unable to create syntax tree xml file! Filename: " << outfile << std::endl;
		return;
	}
	xmlfile << syntaxTree.asXml();
}
