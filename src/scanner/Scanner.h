#ifndef SCANNER_H_
#define SCANNER_H_

#include "scanner/Token.h"

namespace scanner {

class Scanner {
public:
    virtual ~Scanner() {
    }

    virtual Token nextToken() = 0;
};

} // namespace scanner

#endif // SCANNER_H_
