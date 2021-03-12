#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <memory>
#include <string>
#include <vector>

#include "Grammar.h"

namespace parser {

class BNFFileGrammar: public Grammar {
public:
    BNFFileGrammar(const std::string bnfFileName);
    virtual ~BNFFileGrammar();
};

} // namespace parser

#endif // BNFREADER_H_
