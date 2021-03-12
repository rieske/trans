#ifndef _GRAMMAR_BUILDER_H_
#define _GRAMMAR_BUILDER_H_

#include "parser/Grammar.h"

namespace parser {

class GrammarBuilder {
public:
    void defineRule(std::string nonterminal, std::vector<std::string> production);
    //Grammar build();

private:
    //int nextSymbolId {0};
    //std::map<std::string, GrammarSymbol> symbols;
};

}

#endif // _GRAMMAR_BUILDER_H_

