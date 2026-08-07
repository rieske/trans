#ifndef UTIL_STRINGLITERALDECODE_H_
#define UTIL_STRINGLITERALDECODE_H_

#include <string>
#include <vector>

namespace util {

// Sole C escape-sequence decoder (string bodies and char constants).
// Scanner, ConstantExpression, and global string init all use this path.
std::vector<unsigned char> decodeCEscapes(const std::string& body);

// Decode a C string token ("...") into the bytes it stores in a char array,
// including the trailing NUL. Handles simple escapes, hex \xNN, and octal \nnn.
std::vector<unsigned char> decodeStringLiteralBytes(const std::string& token);

// Decode a character constant token ('a', '\n', '\x1b', '\033') to its value.
// Returns false for multi-char or malformed constants.
bool decodeCharConstant(const std::string& token, long& value);

// Byte length including trailing NUL (char x[] = "XXXXXX" has length 7).
int stringLiteralArrayLength(const std::string& token);

// Format decoded bytes as a NASM "db ..." directive (includes trailing NUL).
std::string toNasmDbDirective(const std::string& token);

} // namespace util

#endif // UTIL_STRINGLITERALDECODE_H_
