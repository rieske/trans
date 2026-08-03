#include "StringLiteralDecode.h"

#include <sstream>

namespace util {

std::vector<unsigned char> decodeCEscapes(const std::string& body) {
    std::vector<unsigned char> bytes;
    for (std::size_t i = 0; i < body.size(); ++i) {
        if (body[i] == '\\' && i + 1 < body.size()) {
            ++i;
            char e = body[i];
            switch (e) {
            case 'n':
                bytes.push_back('\n');
                break;
            case 't':
                bytes.push_back('\t');
                break;
            case 'r':
                bytes.push_back('\r');
                break;
            case 'a':
                bytes.push_back('\a');
                break;
            case 'b':
                bytes.push_back('\b');
                break;
            case 'f':
                bytes.push_back('\f');
                break;
            case 'v':
                bytes.push_back('\v');
                break;
            case '\\':
                bytes.push_back('\\');
                break;
            case '"':
                bytes.push_back('"');
                break;
            case '\'':
                bytes.push_back('\'');
                break;
            case '?':
                bytes.push_back('?');
                break;
            case 'x':
            case 'X': {
                unsigned value = 0;
                bool any = false;
                while (i + 1 < body.size()) {
                    char d = body[i + 1];
                    unsigned digit;
                    if (d >= '0' && d <= '9') {
                        digit = static_cast<unsigned>(d - '0');
                    } else if (d >= 'a' && d <= 'f') {
                        digit = static_cast<unsigned>(d - 'a' + 10);
                    } else if (d >= 'A' && d <= 'F') {
                        digit = static_cast<unsigned>(d - 'A' + 10);
                    } else {
                        break;
                    }
                    any = true;
                    ++i;
                    value = (value << 4) | digit;
                }
                bytes.push_back(any ? static_cast<unsigned char>(value & 0xffu) : 0);
                break;
            }
            default:
                if (e >= '0' && e <= '7') {
                    unsigned value = static_cast<unsigned>(e - '0');
                    int count = 1;
                    while (count < 3 && i + 1 < body.size() && body[i + 1] >= '0' && body[i + 1] <= '7') {
                        ++i;
                        ++count;
                        value = (value << 3) | static_cast<unsigned>(body[i] - '0');
                    }
                    bytes.push_back(static_cast<unsigned char>(value & 0xffu));
                } else {
                    bytes.push_back(static_cast<unsigned char>(e));
                }
                break;
            }
        } else {
            bytes.push_back(static_cast<unsigned char>(body[i]));
        }
    }
    return bytes;
}

std::vector<unsigned char> decodeStringLiteralBytes(const std::string& token) {
    std::string body = token;
    if (body.size() >= 2 && body.front() == '"' && body.back() == '"') {
        body = body.substr(1, body.size() - 2);
    }
    auto bytes = decodeCEscapes(body);
    bytes.push_back(0);
    return bytes;
}

bool decodeCharConstant(const std::string& token, long& value) {
    if (token.size() < 3 || token.front() != '\'' || token.back() != '\'') {
        return false;
    }
    const std::string inner = token.substr(1, token.size() - 2);
    const auto bytes = decodeCEscapes(inner);
    // Product: only single-character constants (including escape forms).
    if (bytes.size() != 1) {
        return false;
    }
    value = static_cast<long>(bytes[0]);
    return true;
}

int stringLiteralArrayLength(const std::string& token) {
    return static_cast<int>(decodeStringLiteralBytes(token).size());
}

std::string toNasmDbDirective(const std::string& token) {
    const auto bytes = decodeStringLiteralBytes(token);
    std::ostringstream declaration;
    declaration << "db ";
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) {
            declaration << ", ";
        }
        declaration << static_cast<unsigned>(bytes[i]);
    }
    return declaration.str();
}

} // namespace util
