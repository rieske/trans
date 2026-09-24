#ifndef UTIL_STRINGLITERALDECODE_H_
#define UTIL_STRINGLITERALDECODE_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace util {

// Decode a C string token ("...") into the bytes it stores in a char array,
// including the trailing NUL. Handles simple escapes, hex \xNN, and octal \nnn.
std::vector<unsigned char> decodeStringLiteralBytes(std::string_view token);

// Decode a C character-constant token ('a', '\n', '\xFE', '\033') to 0..255.
bool decodeCharConstant(const std::string& token, long& value);

// Byte length including trailing NUL (char x[] = "XXXXXX" has length 7).
// Wide prefixes (L, u, U) count code units, not storage bytes.
int stringLiteralArrayLength(const std::string &token);

// 1 for a narrow or u8 literal, 2 for u, 4 for L or U.
int stringLiteralUnitBytes(const std::string& token);

// Format decoded bytes as a NASM "db ..." directive (includes trailing NUL).
std::string toNasmDbDirective(const std::string &token);

// Format decoded bytes as a GAS ".byte ..." directive (includes trailing NUL).
std::string toGasByteDirective(const std::string &token);

// Inverse of decode for concat: bytes without a trailing NUL become a "..." token.
std::string encodeStringLiteralToken(const std::vector<unsigned char>& bytes);

// Code units of a wide literal, including the trailing NUL.
std::vector<std::uint32_t> wideStringCodeUnits(const std::string& token);

// Join string lexemes under a wide prefix (L, u, or U). Each piece is decoded
// as wide code units first, so a UTF-8 character stays one unit.
std::string concatWideLiterals(std::string_view prefix, const std::vector<std::string>& pieces);

} // namespace util

#endif // UTIL_STRINGLITERALDECODE_H_
