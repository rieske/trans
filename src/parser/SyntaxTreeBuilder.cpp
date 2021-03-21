#include "SyntaxTreeBuilder.h"

namespace parser {

SyntaxTreeBuilder::~SyntaxTreeBuilder() = default;

void SyntaxTreeBuilder::err() {
    this->erred = true;
}

void SyntaxTreeBuilder::assertBuildable() const {
    if (erred) {
        throw std::runtime_error { "parsing failed with syntax errors" };
    }
}

} // namespace parser

