#ifndef _LEX_FILE_SCANNER_BUILDER_
#define _LEX_FILE_SCANNER_BUILDER_

#include "scanner/FiniteAutomaton.h"
#include "scanner/State.h"
#include <string>
#include <map>
#include <memory>

namespace scanner {

class LexFileScannerReader {
public:
    FiniteAutomaton* fromConfiguration(std::string configPath);
};

} // namespace scanner

#endif // _LEX_FILE_SCANNER_BUILDER_
