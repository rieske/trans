#include "Action.h"

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <sstream>

#include "AcceptAction.h"
#include "ErrorAction.h"
#include "Grammar.h"
#include "LR1Item.h"
#include "ParsingTable.h"
#include "ReduceAction.h"
#include "ShiftAction.h"

using std::unique_ptr;
using std::string;
using std::istringstream;

const char SHIFT_ACTION = 's';
const char REDUCE_ACTION = 'r';
const char ERROR_ACTION = 'e';
const char ACCEPT_ACTION = 'a';

unique_ptr<Action> Action::deserialize(string& serializedAction, const Grammar& grammar,
		const std::unordered_map<parse_state, std::map<std::shared_ptr<const GrammarSymbol>, parse_state>>* gotoTable) {
	istringstream actionStream { serializedAction };
	char type;
	actionStream >> type;
	parse_state state;
	actionStream >> state;
	switch (type) {
	case SHIFT_ACTION:
		return unique_ptr<Action> { new ShiftAction(state) };
	case REDUCE_ACTION: {
		size_t nonterminalId;
		size_t productionId;
		actionStream >> nonterminalId >> productionId;
		return unique_ptr<Action> { new ReduceAction(grammar.getReductionById(nonterminalId, productionId), gotoTable) };
	}
	case ACCEPT_ACTION:
		return unique_ptr<Action> { new AcceptAction() };
	case ERROR_ACTION: {
		string forge;
		string expected;
		actionStream >> forge >> expected;
		return unique_ptr<Action> { new ErrorAction(state, forge, expected) };
	}
	default:
		throw std::runtime_error("Error in parsing actionStream configuration file: invalid action type: " + type);
	}
}
