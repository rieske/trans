#ifndef _ARGV_PARSER_H_
#define _ARGV_PARSER_H_

#include <string>
#include <string.h>

using std::string;

class Argv_parser
{
    public:
        Argv_parser();

        int parse_argv(int argc, char **argv);

        string *get_custom_grammar() const;

    private:
        string *custom_grammar;
};

#endif // _ARGV_PARSER_H_
