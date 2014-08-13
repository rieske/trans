#include "WhileLoopHeader.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

WhileLoopHeader::WhileLoopHeader(std::unique_ptr<Expression> clause) :
        clause { std::move(clause) } {
}

WhileLoopHeader::~WhileLoopHeader() {
}

void WhileLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
