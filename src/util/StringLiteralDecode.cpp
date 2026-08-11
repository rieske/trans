#include "StringLiteralDecode.h"

#include <cstdio>
#include <sstream>

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

std::vector<unsigned char> decodeStringLiteralBytes(const std::string &token) {
    std::string body = token;
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
    value = byte;
    return true;
}

int stringLiteralArrayLength(const std::string &token) { return static_cast<int>(decodeStringLiteralBytes(token).size()); }

namespace {

std::string decodedBytesAsDecimalList(const std::string &token) {
    const auto bytes = decodeStringLiteralBytes(token);
    std::ostringstream list;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) {
            list << ", ";
        }
        list << static_cast<unsigned>(bytes[i]);
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
