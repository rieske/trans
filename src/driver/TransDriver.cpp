#include "TransDriver.h"

#include <iostream>
#include <iterator>
#include <vector>

#include "code_generator/code_generator.h"
#include "parser/parser.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/syntax_tree.h"

using std::string;
using std::vector;

TransDriver::TransDriver(Configuration& configuration) :
		configuration(configuration) {
}

TransDriver::~TransDriver() {
}

void TransDriver::run() const {

	// FIXME:
	if (configuration.isScannerLoggingEnabled()) {
		Scanner::set_logging("scanner.log");
	}
	if (configuration.isParserLoggingEnabled()) {
		Parser::set_logging("parser.log");
	}

	vector<string> sourceFileNames = configuration.getSourceFileNames();
	vector<string>::const_iterator sourceFileNamesIterator;
	for (sourceFileNamesIterator = sourceFileNames.begin(); sourceFileNamesIterator != sourceFileNames.end(); ++sourceFileNamesIterator) {
		string sourceFileName = *sourceFileNamesIterator;
		std::ifstream sourceFile(sourceFileName.c_str());
		if (sourceFile.is_open()) {
			compile(sourceFileName);
		}
	}
}

void TransDriver::compile(const string& sourceFileName) const {

	std::cout << "Compiling " << sourceFileName << "...\n";
	Parser *parser = NULL;
	string customGrammarFileName = configuration.getCustomGrammarFileName();
	if (!customGrammarFileName.empty()) {
		parser = new Parser(customGrammarFileName);
	} else {
		parser = new Parser();
	}
	if (0 == parser->parse(sourceFileName.c_str())) {
		SyntaxTree *tree = parser->getSyntaxTree();
		if (tree != NULL && tree->getErrorFlag() == false) {
			std::cout << "Successful semantic analysis\n";
			tree->outputCode(std::cout);

			CodeGenerator *codeGen = new CodeGenerator(sourceFileName.c_str());
			if (0 == codeGen->generateCode(tree->getCode(), tree->getSymbolTable())) {
				codeGen->assemble();
				codeGen->link();
			} else {
				std::cerr << "Code generation failed!\n";
			}
			delete codeGen;
		} else {
			std::cerr << "Compilation failed with semantic errors!\n";
		}
	} else {
		std::cout << "Parsing failed!\n";
	}
	delete parser;
}
