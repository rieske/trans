#include "QuadrupleGenerator.h"

#include <algorithm>
#include <stdexcept>

#include "ast/AbstractSyntaxTreeNode.h"
#include "CodeGeneratingVisitor.h"

namespace codegen {

std::vector<std::unique_ptr<Quadruple>> QuadrupleGenerator::generateQuadruplesFrom(parser::SyntaxTree& syntaxTree) {
    syntaxTree.accept(*this);
    return std::move(quadruples);
}

void QuadrupleGenerator::visit(ast::AbstractSyntaxTree& tree) {
    CodeGeneratingVisitor quadrupleVisitor;
    for (const auto& treeNode : tree) {
        treeNode->accept(quadrupleVisitor);
    }
    quadruples = quadrupleVisitor.getQuadruples();
}

void QuadrupleGenerator::visit(parser::ParseTree&) {
    throw std::runtime_error { "can not generate quadruple code from a parse tree" };
}

} /* namespace codegen */
