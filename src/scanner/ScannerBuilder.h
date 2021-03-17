#ifndef _SCANNER_BUILDER_H_
#define _SCANNER_BUILDER_H_

#include "scanner/FiniteAutomaton.h"
#include "scanner/State.h"

#include <memory>
#include <map>
#include <vector>

namespace scanner {

class ScannerBuilder {
public:
    std::string addState(std::unique_ptr<State> state);
    void addTransition(std::string fromState, std::string transitionOn, std::string transitionTo);
    void addKeyword(std::string keyword);

    FiniteAutomaton* build();

private:
    int nextKeywordId {1};
    std::map<std::string, int> keywordIds;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> namedStateTransitions;

    State* startState { nullptr };
    std::map<std::string, std::unique_ptr<State>> namedStates;
};

} // namespace scanner

#endif //  _SCANNER_BUILDER_H_
