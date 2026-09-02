#ifndef SYMBOLS_FUNCTION_FRAME_H_
#define SYMBOLS_FUNCTION_FRAME_H_

#include <map>
#include <string>
#include <vector>

#include "FunctionEntry.h"
#include "ValueEntry.h"

namespace symbols {

// SA→CG snapshot of one function definition. Locals are automatics keyed by IR name.
struct FunctionFrame {
    FunctionEntry symbol;
    std::map<std::string, ValueEntry> locals;
    std::vector<ValueEntry> arguments;
};

} // namespace symbols

#endif
