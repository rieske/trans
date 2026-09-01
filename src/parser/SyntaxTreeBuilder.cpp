#include "SyntaxTreeBuilder.h"

#include "util/Diagnostic.h"

#include <stdexcept>

namespace parser {

SyntaxTreeBuilder::~SyntaxTreeBuilder() = default;

diag::Sink& SyntaxTreeBuilder::sink() const {
    if (!sink_) {
        throw std::logic_error { "missing diagnostic sink" };
    }
    return *sink_;
}

void SyntaxTreeBuilder::err() {
    this->erred = true;
}

void SyntaxTreeBuilder::assertBuildable() const {
    if (erred) {
        throw std::runtime_error { "parsing failed with syntax errors" };
    }
}

} // namespace parser

