#include "LoopHeader.h"

#include "symbols/AnnotationStore.h"

namespace ast {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) } {
}

void LoopHeader::setLoopEntry(symbols::LabelEntry loopEntry) {
    symbols::AnnotationStore::current().setLabel(
            this, symbols::LabelSlot::LoopEntry, std::move(loopEntry));
}

symbols::LabelEntry* LoopHeader::getLoopEntry() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::LoopEntry);
}

void LoopHeader::setLoopContinue(symbols::LabelEntry loopContinue) {
    symbols::AnnotationStore::current().setLabel(
            this, symbols::LabelSlot::LoopContinue, std::move(loopContinue));
}

symbols::LabelEntry* LoopHeader::getLoopContinue() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::LoopContinue);
}

void LoopHeader::setLoopExit(symbols::LabelEntry loopExit) {
    symbols::AnnotationStore::current().setLabel(
            this, symbols::LabelSlot::LoopExit, std::move(loopExit));
}

symbols::LabelEntry* LoopHeader::getLoopExit() const {
    return symbols::AnnotationStore::current().label(this, symbols::LabelSlot::LoopExit);
}

} // namespace ast
