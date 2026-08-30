#ifndef _SCANNER_BUILDER_H_
#define _SCANNER_BUILDER_H_

#include "scanner/FiniteAutomaton.h"
#include "scanner/State.h"

#include <map>
#include <memory>
#include <unordered_set>
#include <vector>

namespace scanner {

class ScannerBuilder {
public:
    std::string addState(std::unique_ptr<State> state);
    void addTransition(std::string fromState, std::string transitionOn, std::string transitionTo);
    void addKeyword(std::string keyword);

    std::unique_ptr<FiniteAutomaton> build();

private:
    std::unordered_set<std::string> keywords;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> namedStateTransitions;

    State* startState { nullptr };
    std::map<std::string, std::unique_ptr<State>> namedStates;
};

} // namespace scanner

#endif //  _SCANNER_BUILDER_H_
