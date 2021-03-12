#ifndef _GRAMMAR_BUILDER_H_
#define _GRAMMAR_BUILDER_H_

#include "parser/Grammar.h"

namespace parser {

class GrammarBuilder {
public:
    void defineRule(std::string nonterminal, std::vector<std::string> production);
    Grammar build();

private:
    int defineSymbol(std::string symbolName);
    bool symbolExists(std::string symbolName) const;
    bool nonterminalDefinitionExists(std::string nonterminal) const;

    int nextSymbolId {0};
    std::map<std::string, int> symbolIDs;

    std::vector<std::string> nonterminalDefinitions;
    std::multimap<std::string, std::vector<std::string>> productions;
};

}

#endif // _GRAMMAR_BUILDER_H_
