#include "IrGenerator.h"

#include "CodeGeneratingVisitor.h"
#include "ast/AbstractSyntaxTree.h"

namespace codegen {

IntermediateRepresentation generateIr(ast::AbstractSyntaxTree& tree) {
    CodeGeneratingVisitor visitor(tree.annotations());
    for (const auto& treeNode : tree) {
        treeNode->accept(visitor);
    }
    return visitor.takeFinishedIr();
}

} // namespace codegen
