#ifndef _ARGV_PARSER_H_
#define _ARGV_PARSER_H_

#include <string>

using std::string;

class ArgvParser {
public:
	ArgvParser();

	int parse_argv(int argc, char **argv);

	string *get_custom_grammar() const;

private:
	string *custom_grammar;
};

#endif // _ARGV_PARSER_H_
