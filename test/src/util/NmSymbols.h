#ifndef NM_SYMBOLS_H_
#define NM_SYMBOLS_H_

#include <sstream>
#include <string>

// Parse `nm -P` stdout. Type char is '\0' when the name is absent.
inline char nmSymbolType(const std::string& nmStdout, const std::string& name) {
    const std::string prefix = name + " ";
    std::string line;
    std::istringstream in { nmStdout };
    while (std::getline(in, line)) {
        if (line.compare(0, prefix.size(), prefix) != 0) {
            continue;
        }
        if (line.size() > prefix.size()) {
            return line[prefix.size()];
        }
    }
    return '\0';
}

inline bool nmTypeIsGlobal(char type) {
    return type == 'T' || type == 'D' || type == 'B' || type == 'R';
}

inline bool nmTypeIsDefined(char type) {
    return nmTypeIsGlobal(type) || type == 't' || type == 'd' || type == 'b' || type == 'r';
}

#endif
