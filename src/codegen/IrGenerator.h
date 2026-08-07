#ifndef CODEGEN_IR_GENERATOR_H_
#define CODEGEN_IR_GENERATOR_H_

#include "Instruction.h"
#include "parser/SyntaxTree.h"

namespace codegen {

IntermediateRepresentation generateIr(parser::SyntaxTree& syntaxTree);

} // namespace codegen

#endif // CODEGEN_IR_GENERATOR_H_
