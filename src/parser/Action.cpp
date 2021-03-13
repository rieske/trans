#include "Action.h"

#include <sstream>

#include "AcceptAction.h"
#include "ErrorAction.h"
#include "Grammar.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

namespace parser {

const char SHIFT_ACTION = 's';
const char REDUCE_ACTION = 'r';
const char ERROR_ACTION = 'e';
const char ACCEPT_ACTION = 'a';

Action::~Action() = default;

std::unique_ptr<Action> Action::deserialize(std::string serializedAction, const ParsingTable& parsingTable, const Grammar& grammar) {
    std::istringstream actionStream { serializedAction };
    char type;
    actionStream >> type;
    switch (type) {
    case SHIFT_ACTION: {
        parse_state state;
        actionStream >> state;
        return std::make_unique<ShiftAction>(state);
    }
    case REDUCE_ACTION: {
        size_t productionId;
        actionStream >> productionId;
        const auto& production = grammar.getRuleById(productionId);
        return std::make_unique<ReduceAction>(production, &parsingTable);
    }
    case ACCEPT_ACTION:
        return std::make_unique<AcceptAction>();
    case ERROR_ACTION: {
        parse_state state;
        std::string forge;
        int expected;
        actionStream >> state >> forge >> expected;
        return std::make_unique<ErrorAction>(state, forge, expected, &grammar);
    }
    default:
        throw std::runtime_error("Error in parsing actionStream configuration file: invalid action type: " + std::to_string(type));
    }
}

} // namespace parser

