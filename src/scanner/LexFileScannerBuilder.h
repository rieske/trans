#ifndef _LEX_FILE_SCANNER_BUILDER_
#define _LEX_FILE_SCANNER_BUILDER_

#include "scanner/FiniteAutomaton.h"
#include "scanner/State.h"
#include <string>
#include <map>
#include <memory>

namespace scanner {

class LexFileScannerBuilder {
public:
    FiniteAutomaton* fromConfiguration(std::string configPath);

private:
    int nextKeywordId { 1 };
    std::map<std::string, unsigned> keywordIds;
    std::map<std::string, std::unique_ptr<State>> namedStates;

    State* addNewState(std::unique_ptr<State> stateToAdd);
    void parseKeywords(std::string keywordsRecord);
};

} // namespace scanner

#endif // _LEX_FILE_SCANNER_BUILDER_
