#ifndef SCANNER_H_
#define SCANNER_H_

#include "scanner/Token.h"

class Scanner {
public:
    virtual ~Scanner() {
    }

    virtual Token nextToken() = 0;
};

#endif // SCANNER_H_
