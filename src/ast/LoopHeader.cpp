#include "LoopHeader.h"

#include <algorithm>

#include "Expression.h"

namespace ast {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) } {
}

code_generator::LabelEntry *LoopHeader::getLoopLabel() const {
    return loop_label;
}

void LoopHeader::setLoopEntry(code_generator::LabelEntry* loopEntry) {
    this->loopEntry = loopEntry;
}

code_generator::LabelEntry* LoopHeader::getLoopEntry() const {
    return loopEntry;
}

void LoopHeader::setLoopExit(code_generator::LabelEntry* loopExit) {
    this->loopExit = loopExit;
}

code_generator::LabelEntry* LoopHeader::getLoopExit() const {
    return loopExit;
}

}
