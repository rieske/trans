#include "LoopHeader.h"
#include "semantic_analyzer/LabelEntry.h"

namespace ast {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) }
{
}

void LoopHeader::setLoopEntry(semantic_analyzer::LabelEntry loopEntry) {
    this->loopEntry = std::make_unique<semantic_analyzer::LabelEntry>(loopEntry);
}

semantic_analyzer::LabelEntry* LoopHeader::getLoopEntry() const {
    return loopEntry.get();
}

void LoopHeader::setLoopExit(semantic_analyzer::LabelEntry loopExit) {
    this->loopExit = std::make_unique<semantic_analyzer::LabelEntry>(loopExit);
}

semantic_analyzer::LabelEntry* LoopHeader::getLoopExit() const {
    return loopExit.get();
}

} // namespace ast

