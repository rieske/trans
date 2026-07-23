#ifndef UTIL_STRINGLITERALDECODE_H_
#define UTIL_STRINGLITERALDECODE_H_

#include <string>
#include <vector>

namespace util {

// Decode a C string token ("...") into the bytes it stores in a char array,
// including the trailing NUL. Handles simple escapes, hex \xNN, and octal \nnn.
std::vector<unsigned char> decodeStringLiteralBytes(const std::string &token);

// Byte length including trailing NUL (char x[] = "XXXXXX" has length 7).
int stringLiteralArrayLength(const std::string &token);

// Format decoded bytes as a NASM "db ..." directive (includes trailing NUL).
std::string toNasmDbDirective(const std::string &token);

} // namespace util

#endif // UTIL_STRINGLITERALDECODE_H_
