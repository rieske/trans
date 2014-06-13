#include "LR1Parser.h"

#include <fstream>
#include <string>

#include "../scanner/Token.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../semantic_analyzer/SemanticComponentsFactory.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "Action.h"
#include "TokenStream.h"


using std::unique_ptr;

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

	unique_ptr<SyntaxTree> syntaxTree { nullptr };
	while (!syntaxTree) {
		auto& parseAction = parsingTable->action(parsing_stack.top(), tokenStream.getCurrentToken().id);
		syntaxTree = parseAction.perform(parsing_stack, tokenStream, *semanticAnalyzer);
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
		cerr << "Unable to create syntax tree xml file! Filename: " << outfile << endl;
		return;
	}
	xmlfile << syntaxTree.asXml();
}
