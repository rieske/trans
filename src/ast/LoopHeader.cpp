#include "LoopHeader.h"

#include <algorithm>

namespace ast {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) }
{
}

void LoopHeader::setLoopEntry(code_generator::LabelEntry loopEntry) {
    this->loopEntry = std::make_unique<code_generator::LabelEntry>(loopEntry);
}

code_generator::LabelEntry* LoopHeader::getLoopEntry() const {
    return loopEntry.get();
}

void LoopHeader::setLoopExit(code_generator::LabelEntry loopExit) {
    this->loopExit = std::make_unique<code_generator::LabelEntry>(loopExit);
}

code_generator::LabelEntry * LoopHeader::getLoopExit() const {
    return loopExit.get();
}

}
