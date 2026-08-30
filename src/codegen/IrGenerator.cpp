#include "IrGenerator.h"

#include "CodeGeneratingVisitor.h"
#include "FrameLayout.h"
#include "IrPasses.h"

namespace codegen {

IntermediateRepresentation generateIr(ast::AbstractSyntaxTree& tree) {
    CodeGeneratingVisitor visitor(tree.annotations(), tree.vlaExpressions());
    for (const auto& treeNode : tree) {
        treeNode->accept(visitor);
    }
    IntermediateRepresentation ir = runIrPasses(visitor.takeIr());
    for (auto& procedure : ir.procedures) {
        procedure.frame.locals = packFrameValues(std::move(procedure.frame.locals), procedure.body);
    }
    return ir;
}

} // namespace codegen
