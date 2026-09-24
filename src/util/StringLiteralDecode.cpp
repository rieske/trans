#include "StringLiteralDecode.h"

#include <cstdint>
#include <cstdio>
#include <sstream>
#include <vector>

namespace util {

namespace {

bool hexDigit(char d, unsigned& digit) {
    if (d >= '0' && d <= '9') {
        digit = static_cast<unsigned>(d - '0');
        return true;
    }
    if (d >= 'a' && d <= 'f') {
        digit = static_cast<unsigned>(d - 'a' + 10);
        return true;
    }
    if (d >= 'A' && d <= 'F') {
        digit = static_cast<unsigned>(d - 'A' + 10);
        return true;
    }
    return false;
}

bool consumeEncodedByte(const std::string& body, std::size_t& pos, unsigned char& out) {
    if (pos >= body.size()) {
        return false;
    }
    if (body[pos] != '\\' || pos + 1 >= body.size()) {
        out = static_cast<unsigned char>(body[pos]);
        ++pos;
        return true;
    }
    ++pos;
    const char e = body[pos];
    switch (e) {
    case 'n':
        out = '\n';
        ++pos;
        return true;
    case 't':
        out = '\t';
        ++pos;
        return true;
    case 'r':
        out = '\r';
        ++pos;
        return true;
    case 'a':
        out = '\a';
        ++pos;
        return true;
    case 'b':
        out = '\b';
        ++pos;
        return true;
    case 'f':
        out = '\f';
        ++pos;
        return true;
    case 'v':
        out = '\v';
        ++pos;
        return true;
    case '\\':
        out = '\\';
        ++pos;
        return true;
    case '"':
        out = '"';
        ++pos;
        return true;
    case '\'':
        out = '\'';
        ++pos;
        return true;
    case '?':
        out = '?';
        ++pos;
        return true;
    case 'x':
    case 'X': {
        unsigned value = 0;
        bool any = false;
        unsigned digit = 0;
        while (pos + 1 < body.size() && hexDigit(body[pos + 1], digit)) {
            any = true;
            ++pos;
            value = (value << 4) | digit;
        }
        ++pos;
        out = any ? static_cast<unsigned char>(value & 0xffu) : 0;
        return true;
    }
    default:
        if (e >= '0' && e <= '7') {
            unsigned value = static_cast<unsigned>(e - '0');
            int count = 1;
            ++pos;
            while (count < 3 && pos < body.size() && body[pos] >= '0' && body[pos] <= '7') {
                value = (value << 3) | static_cast<unsigned>(body[pos] - '0');
                ++pos;
                ++count;
            }
            out = static_cast<unsigned char>(value & 0xffu);
            return true;
        }
        out = static_cast<unsigned char>(e);
        ++pos;
        return true;
    }
}

} // namespace

int stringLiteralUnitBytes(const std::string& token) {
    if (token.rfind("u8\"", 0) == 0) {
        return 1;
    }
    if (token.size() >= 2 && token[1] == '"') {
        if (token[0] == 'L' || token[0] == 'U') {
            return 4;
        }
        if (token[0] == 'u') {
            return 2;
        }
    }
    return 1;
}

static bool prefixThenQuote(std::string_view token, std::size_t prefixLen) {
    return token.size() > prefixLen && token[prefixLen] == '"';
}

static std::string_view literalBody(std::string_view token) {
    if (token.size() >= 2 && token[0] == 'u' && token[1] == '8' && prefixThenQuote(token, 2)) {
        return token.substr(2);
    }
    if (prefixThenQuote(token, 1)
            && (token[0] == 'L' || token[0] == 'U' || token[0] == 'u')) {
        return token.substr(1);
    }
    return token;
}

// Escape sequences are one code unit. A raw UTF-8 sequence is one character.
// Wide escapes keep the numeric value. A byte mask would split a code point
// that was written with more than two hex digits.
static std::uint32_t wideEscape(const std::string& body, std::size_t& pos) {
    ++pos;
    if (pos >= body.size()) {
        return '\\';
    }
    const char e = body[pos];
    auto named = [&](unsigned char value) {
        ++pos;
        return static_cast<std::uint32_t>(value);
    };
    switch (e) {
    case 'n': return named('\n');
    case 't': return named('\t');
    case 'r': return named('\r');
    case 'a': return named('\a');
    case 'b': return named('\b');
    case 'f': return named('\f');
    case 'v': return named('\v');
    case '\\': return named('\\');
    case '"': return named('"');
    case '\'': return named('\'');
    case '?': return named('?');
    case 'x':
    case 'X': {
        ++pos;
        std::uint32_t value = 0;
        bool any = false;
        unsigned digit = 0;
        while (pos < body.size() && hexDigit(body[pos], digit)) {
            any = true;
            value = (value << 4) | digit;
            ++pos;
        }
        return any ? value : 0;
    }
    case 'u':
    case 'U': {
        // \u and \U are fixed width, so a following hex digit is a new unit.
        const int width = e == 'U' ? 8 : 4;
        ++pos;
        std::uint32_t value = 0;
        unsigned digit = 0;
        int count = 0;
        while (count < width && pos < body.size() && hexDigit(body[pos], digit)) {
            value = (value << 4) | digit;
            ++pos;
            ++count;
        }
        return value;
    }
    default:
        if (e >= '0' && e <= '7') {
            std::uint32_t value = static_cast<unsigned>(e - '0');
            int count = 1;
            ++pos;
            while (count < 3 && pos < body.size() && body[pos] >= '0' && body[pos] <= '7') {
                value = (value << 3) | static_cast<unsigned>(body[pos] - '0');
                ++pos;
                ++count;
            }
            return value;
        }
        ++pos;
        return static_cast<unsigned char>(e);
    }
}

static std::uint32_t utf8CodePoint(const std::string& body, std::size_t& pos) {
    const auto lead = static_cast<unsigned char>(body[pos]);
    int need = 0;
    std::uint32_t cp = lead;
    if ((lead & 0xE0u) == 0xC0u) {
        cp = lead & 0x1Fu;
        need = 1;
    } else if ((lead & 0xF0u) == 0xE0u) {
        cp = lead & 0x0Fu;
        need = 2;
    } else if ((lead & 0xF8u) == 0xF0u) {
        cp = lead & 0x07u;
        need = 3;
    } else {
        ++pos;
        return lead;
    }
    if (pos + static_cast<std::size_t>(need) >= body.size()) {
        ++pos;
        return lead;
    }
    for (int i = 1; i <= need; ++i) {
        const auto cont = static_cast<unsigned char>(body[pos + static_cast<std::size_t>(i)]);
        if ((cont & 0xC0u) != 0x80u) {
            ++pos;
            return lead;
        }
        cp = (cp << 6) | (cont & 0x3Fu);
    }
    pos += static_cast<std::size_t>(need) + 1;
    return cp;
}

static std::vector<std::uint32_t> wideCodeUnits(const std::string& token) {
    const int unit = stringLiteralUnitBytes(token);
    std::string body { literalBody(token) };
    if (body.size() >= 2 && body.front() == '"' && body.back() == '"') {
        body = body.substr(1, body.size() - 2);
    }
    std::vector<std::uint32_t> units;
    std::size_t pos = 0;
    while (pos < body.size()) {
        std::uint32_t cp = 0;
        const bool escape = body[pos] == '\\';
        if (escape) {
            cp = wideEscape(body, pos);
        } else {
            cp = utf8CodePoint(body, pos);
        }
        // An escape is one unit, cut to the element width. Only a raw
        // character above U+FFFF is a UTF-16 surrogate pair.
        if (!escape && unit == 2 && cp > 0xFFFFu) {
            cp -= 0x10000u;
            units.push_back(0xD800u + (cp >> 10));
            units.push_back(0xDC00u + (cp & 0x3FFu));
        } else {
            if (unit == 2) {
                cp &= 0xFFFFu;
            }
            units.push_back(cp);
        }
    }
    units.push_back(0);
    return units;
}

std::vector<unsigned char> decodeStringLiteralBytes(std::string_view token) {
    token = literalBody(token);
    std::string body { token };
    if (body.size() >= 2 && body.front() == '"' && body.back() == '"') {
        body = body.substr(1, body.size() - 2);
    }
    std::vector<unsigned char> bytes;
    std::size_t pos = 0;
    unsigned char byte = 0;
    while (consumeEncodedByte(body, pos, byte)) {
        bytes.push_back(byte);
    }
    bytes.push_back(0);
    return bytes;
}

bool decodeCharConstant(const std::string& token, long& value) {
    if (token.size() < 3 || token.front() != '\'' || token.back() != '\'') {
        return false;
    }
    const std::string inner = token.substr(1, token.size() - 2);
    std::size_t pos = 0;
    unsigned char byte = 0;
    if (!consumeEncodedByte(inner, pos, byte) || pos != inner.size()) {
        return false;
    }
    value = static_cast<long>(static_cast<signed char>(byte));
    return true;
}

std::vector<std::uint32_t> wideStringCodeUnits(const std::string& token) {
    return wideCodeUnits(token);
}

int stringLiteralArrayLength(const std::string &token) {
    if (stringLiteralUnitBytes(token) == 1) {
        return static_cast<int>(decodeStringLiteralBytes(token).size());
    }
    return static_cast<int>(wideCodeUnits(token).size());
}

namespace {

std::string decodedBytesAsDecimalList(const std::string &token) {
    const int unit = stringLiteralUnitBytes(token);
    std::ostringstream list;
    bool first = true;
    auto emitByte = [&](unsigned piece) {
        if (!first) {
            list << ", ";
        }
        first = false;
        list << piece;
    };
    if (unit == 1) {
        for (unsigned char b : decodeStringLiteralBytes(token)) {
            emitByte(b);
        }
        return list.str();
    }
    for (std::uint32_t cp : wideCodeUnits(token)) {
        for (int i = 0; i < unit; ++i) {
            emitByte((cp >> (8 * i)) & 0xFFu);
        }
    }
    return list.str();
}

} // namespace

std::string toNasmDbDirective(const std::string &token) {
    return "db " + decodedBytesAsDecimalList(token);
}

std::string toGasByteDirective(const std::string &token) {
    return ".byte " + decodedBytesAsDecimalList(token);
}

std::string concatWideLiterals(std::string_view prefix, const std::vector<std::string>& pieces) {
    std::vector<std::uint32_t> units;
    for (const std::string& piece : pieces) {
        const std::string token = stringLiteralUnitBytes(piece) == 1
                ? std::string(prefix) + piece
                : piece;
        auto one = wideCodeUnits(token);
        if (!one.empty()) {
            one.pop_back();
        }
        units.insert(units.end(), one.begin(), one.end());
    }
    std::string body;
    body.push_back('"');
    for (std::uint32_t cp : units) {
        if (cp >= 0x20u && cp <= 0x7eu && cp != '\\' && cp != '"') {
            body.push_back(static_cast<char>(cp));
        } else {
            char encoded[16];
            std::snprintf(encoded, sizeof(encoded), "\\U%08x", cp);
            body += encoded;
        }
    }
    body.push_back('"');
    return std::string(prefix) + body;
}

std::string encodeStringLiteralToken(const std::vector<unsigned char>& bytes) {
    std::string out = "\"";
    out.reserve(bytes.size() * 4 + 2);
    for (unsigned char b : bytes) {
        switch (b) {
        case '\n': out += "\\n"; break;
        case '\t': out += "\\t"; break;
        case '\r': out += "\\r"; break;
        case '\a': out += "\\a"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\v': out += "\\v"; break;
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        default:
            if (b >= 0x20 && b <= 0x7e) {
                out += static_cast<char>(b);
            } else {
                char buf[5];
                std::snprintf(buf, sizeof(buf), "\\%03o", static_cast<unsigned>(b));
                out += buf;
            }
            break;
        }
    }
    out += '"';
    return out;
}

} // namespace util
