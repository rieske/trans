#include "LoopHeader.h"

namespace ast {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) }
{
}

void LoopHeader::setLoopEntry(symbols::AnnotationStore& store, symbols::LabelEntry loopEntry) {
    store.setLabel(this, symbols::LabelSlot::LoopEntry, std::move(loopEntry));
}

symbols::LabelEntry* LoopHeader::getLoopEntry(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::LoopEntry);
}

void LoopHeader::setLoopExit(symbols::AnnotationStore& store, symbols::LabelEntry loopExit) {
    store.setLabel(this, symbols::LabelSlot::LoopExit, std::move(loopExit));
}

symbols::LabelEntry* LoopHeader::getLoopExit(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::LoopExit);
}

void LoopHeader::setLoopContinue(symbols::AnnotationStore& store, symbols::LabelEntry loopContinue) {
    store.setLabel(this, symbols::LabelSlot::LoopContinue, std::move(loopContinue));
}

symbols::LabelEntry* LoopHeader::getLoopContinue(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::LoopContinue);
}

} // namespace ast
