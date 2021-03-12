#ifndef _GRAMMAR_BUILDER_H_
#define _GRAMMAR_BUILDER_H_

#include "parser/Grammar.h"

namespace parser {

class GrammarBuilder {
public:
    void defineRule(std::string nonterminal, std::vector<std::string> production);
    //Grammar build();
    void build();

private:
    bool symbolExists(std::string symbolName) const;
    int defineSymbol(std::string symbolName);
    bool nonterminalDefinitionExists(std::string nonterminal) const;
    //std::map<std::string, GrammarSymbol> symbols;
    int nextSymbolId {0};
    std::map<std::string, int> symbolIDs;

    std::vector<std::string> nonterminalDefinitions;
    std::multimap<std::string, std::vector<std::string>> productions;
};

}

#endif // _GRAMMAR_BUILDER_H_

