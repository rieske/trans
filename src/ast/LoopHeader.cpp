#include "LoopHeader.h"


namespace ast {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) } {
}







} // namespace ast
