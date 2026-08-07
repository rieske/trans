#include "IrGenerator.h"

#include <stdexcept>

#include "CodeGeneratingVisitor.h"
#include "IrPasses.h"
#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "parser/ParseTree.h"

namespace codegen {

IntermediateRepresentation generateIr(parser::SyntaxTree& syntaxTree) {
    auto* tree = dynamic_cast<ast::AbstractSyntaxTree*>(&syntaxTree);
    if (!tree) {
        if (dynamic_cast<parser::ParseTree*>(&syntaxTree)) {
            throw std::runtime_error { "can not generate intermediate code from a parse tree" };
        }
        throw std::runtime_error { "generateIr: expected AbstractSyntaxTree" };
    }

    CodeGeneratingVisitor visitor(tree->annotations());
    for (const auto& treeNode : *tree) {
        treeNode->accept(visitor);
    }
    return runIrPasses(visitor.takeIr());
}

} // namespace codegen
