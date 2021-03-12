#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <memory>
#include <string>
#include <vector>

#include "Grammar.h"

namespace parser {

class BNFFileGrammar {
public:
    BNFFileGrammar(const std::string bnfFileName);

    Grammar readGrammar(const std::string bnfFileName) const;
};

} // namespace parser

#endif // BNFREADER_H_
