#ifndef CODEGEN_IR_GENERATOR_H_
#define CODEGEN_IR_GENERATOR_H_

#include "Instruction.h"
#include "ast/AbstractSyntaxTree.h"

namespace codegen {

IntermediateRepresentation generateIr(ast::AbstractSyntaxTree& tree);

} // namespace codegen

#endif // CODEGEN_IR_GENERATOR_H_
