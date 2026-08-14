#ifndef SEMANTIC_ANALYZER_CHAR_ARRAY_STRING_INIT_H_
#define SEMANTIC_ANALYZER_CHAR_ARRAY_STRING_INIT_H_

#include <optional>
#include <string>
#include <vector>

#include "types/Type.h"

namespace ast {
class Expression;
}

namespace semantic_analyzer {

// C 6.7.9: a string literal initializes an array of character type as a list
// of codes, including the terminating nul (truncated if the array is shorter).
bool isCharArrayStringInit(const type::Type& destArray, const ast::Expression* value);

// Empty if dest/value is not that form. On excess, sets error and returns empty.
std::optional<std::vector<unsigned char>> charArrayBytesFromString(
        const type::Type& destArray, const ast::Expression* value, std::string* error);

} // namespace semantic_analyzer

#endif
