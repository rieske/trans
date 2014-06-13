#include "Action.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../scanner/Token.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"

using std::cerr;
using std::endl;
using std::shared_ptr;
using std::unique_ptr;
using std::string;

Action::Action(Logger logger) :
		logger { logger } {
}

Action::~Action() {
}
