#include "IrGenerator.h"

#include "CodeGeneratingVisitor.h"
#include "IrPasses.h"

namespace codegen {

IntermediateRepresentation generateIr(ast::AbstractSyntaxTree& tree) {
    CodeGeneratingVisitor visitor(tree.annotations());
    for (const auto& treeNode : tree) {
        treeNode->accept(visitor);
    }
    return runIrPasses(visitor.takeIr());
}

} // namespace codegen
