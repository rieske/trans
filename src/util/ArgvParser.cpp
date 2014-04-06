#include "ArgvParser.h"

#include "parser/parser.h"
#include "scanner/scanner.h"

ArgvParser::ArgvParser() {
	custom_grammar = NULL;
}

int ArgvParser::parse_argv(int argc, char **argv) {
	if (argc <= 1) {
		return -1;
	}

	bool no_more_params = false;
	while (!no_more_params && --argc) {
		switch (**++argv) {
		case '-':   // parametras
			if (0 == strcmp(*argv, "-h")) {
				return -1;
			} else if (0 == strcmp(*argv, "-ls")) {  // scanner logging on
				Scanner::set_logging("scanner.log");
			} else if (0 == strcmp(*argv, "-lp")) {  // parser logging on
				Parser::set_logging("parser.log");
			} else if (0 == strcmp(*argv, "-g")) {  // read custom grammar
				if (--argc) {
					*++argv;
					custom_grammar = new string(*argv);
				} else {
					return -1;
				}
			} else
				return -1;
			break;
		default:
			no_more_params = true;
		}
	}
	return argc;
}

string *ArgvParser::get_custom_grammar() const {
	return custom_grammar;
}
