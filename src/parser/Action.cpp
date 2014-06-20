#include "Action.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

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

unique_ptr<Action> Action::deserialize(string serializedAction, const Grammar& grammar,
		const ParsingTable* parsingTable) {
	istringstream actionStream { serializedAction };
	char type;
	actionStream >> type;
	switch (type) {
	case SHIFT_ACTION: {
		parse_state state;
		actionStream >> state;
		return unique_ptr<Action> { new ShiftAction(state) };
	}
	case REDUCE_ACTION: {
		string nonterminalId;
		size_t productionId;
		actionStream >> nonterminalId >> productionId;
		return unique_ptr<Action> { new ReduceAction(grammar.getReductionById(nonterminalId, productionId), parsingTable) };
	}
	case ACCEPT_ACTION:
		return unique_ptr<Action> { new AcceptAction() };
	case ERROR_ACTION: {
		parse_state state;
		string forge;
		string expected;
		actionStream >> state >> forge >> expected;
		return unique_ptr<Action> { new ErrorAction(state, forge, expected) };
	}
	default:
		throw std::runtime_error("Error in parsing actionStream configuration file: invalid action type: " + type);
	}
}
