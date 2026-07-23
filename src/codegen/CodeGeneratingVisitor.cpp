#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "types/ObjectAbi.h"
#include "FrameLayout.h"
#include "FrameSymbol.h"
#include "util/ProductApprox.h"
#include "ast/BuiltinRegistry.h"

#include "quadruples/Assign.h"
#include "quadruples/Argument.h"
#include "quadruples/Call.h"
#include "quadruples/Retrieve.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/Inc.h"
#include "quadruples/Dec.h"
#include "quadruples/AddressOf.h"
#include "quadruples/FunctionAddress.h"
#include "quadruples/Dereference.h"
#include "quadruples/UnaryMinus.h"
#include "quadruples/BitwiseNot.h"
#include "quadruples/ValueCompare.h"
#include "quadruples/ZeroCompare.h"
#include "quadruples/Jump.h"
#include "quadruples/Label.h"
#include "quadruples/Add.h"
#include "quadruples/Sub.h"
#include "quadruples/Mul.h"
#include "quadruples/Div.h"
#include "quadruples/Mod.h"
#include "quadruples/And.h"
#include "quadruples/Or.h"
#include "quadruples/Xor.h"
#include "quadruples/Return.h"
#include "quadruples/VoidReturn.h"
#include "quadruples/LvalueAssign.h"
#include "quadruples/StartProcedure.h"
#include "quadruples/EndProcedure.h"
#include "quadruples/Shl.h"
#include "quadruples/Shr.h"
#include "quadruples/FieldAccess.h"
#include "quadruples/IndexAddress.h"
#include "quadruples/Truncate.h"
#include "quadruples/BuiltinOp.h"
#include "quadruples/VaOp.h"
#include "ast/MemberAccess.h"
#include "ast/IdentifierExpression.h"
#include "ast/FunctionCall.h"
#include "ast/Expression.h"
#include "ast/ArrayAccess.h"

namespace codegen {




CodeGeneratingVisitor::CodeGeneratingVisitor() {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}



void CodeGeneratingVisitor::narrowIntegralResult(const type::Type& resultType, const std::string& resultName) {
    if (resultType.kind() != type::TypeKind::Primitive || type::isFloating(resultType)) {
        return;
    }
    const int size = resultType.getSize();
    if (size > 0 && size < 8) {
        instructions.push_back(std::make_unique<Truncate>(
                resultName, size, type::valueIsSigned(resultType)));
    }
}


std::string CodeGeneratingVisitor::generateExpression(ast::Expression& expression) {
    expression.accept(*this);
    materializeArrayDecay(expression, instructions);
    if (const std::string* convertTo = expression.getResultConversionTarget()) {
        instructions.push_back(std::make_unique<Assign>(
                expression.getResultSymbol()->getName(), *convertTo));
        return *convertTo;
    }
    if (!expression.hasResultSymbol()) {
        return {};
    }
    return expression.getResultSymbol()->getName();
}

void CodeGeneratingVisitor::emitAddressOf(ast::Expression& operand, const std::string& destName) {
    // &ptr->m / &obj.m: form field address without loading the member.
    if (auto* member = dynamic_cast<ast::MemberAccess*>(&operand)) {
        generateExpression(*member->getBase());
        bool baseIsPointer = member->isArrow();
        std::string baseName = member->getBaseResultSymbol()->getName();
        resolveMemberBase(baseName, baseIsPointer, member->getBase());
        instructions.push_back(std::make_unique<FieldAddress>(
                baseName, member->getMemberOffset(), destName, baseIsPointer));
        return;
    }
    // &a[i]: form index address without loading the element.
    if (auto* arrayAccess = dynamic_cast<ast::ArrayAccess*>(&operand)) {
        generateExpression(*arrayAccess->getLeftOperand());
        generateExpression(*arrayAccess->getRightOperand());
        std::string baseName = arrayAccess->leftOperandSymbol()->getName();
        bool baseIsArray = arrayAccess->baseIsArray();
        resolveArrayBase(baseName, baseIsArray, arrayAccess->getLeftOperand());
        instructions.push_back(std::make_unique<IndexAddress>(
                baseName,
                arrayAccess->rightOperandSymbol()->getName(),
                arrayAccess->getElementSize(),
                destName,
                baseIsArray));
        return;
    }
    generateExpression(operand);
    // Function designator address is already the result of visiting the operand.
    if (auto* id = dynamic_cast<ast::IdentifierExpression*>(&operand);
            id && id->hasFunctionDesignator()) {
        instructions.push_back(std::make_unique<Assign>(
                operand.getResultSymbol()->getName(), destName));
        return;
    }
    if (auto* lvalue = operand.getLvalueSymbol()) {
        instructions.push_back(std::make_unique<Assign>(lvalue->getName(), destName));
    } else {
        instructions.push_back(std::make_unique<AddressOf>(
                operand.getResultSymbol()->getName(), destName));
    }
}

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers&) {
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    // File-scope variables are initialized in .data; skip children (would emit assigns with no procedure).
    if (declarator.hasInitializer() && declarator.getHolder()->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (!declarator.getStructFieldInits().empty()) {
        // Leaf expressions are evaluated via visit(InitializerListExpression) recursion
        // from visitChildren; only emit field stores here.
        emitStructFieldInits(instructions, declarator.getHolder()->getName(),
                declarator.getStructFieldInits());
        return;
    }
    if (declarator.hasInitializer()) {
        if (dynamic_cast<ast::InitializerListExpression*>(declarator.getInitializer())) {
            // Non-struct list init not expanded (e.g. local array of aggregates).
            return;
        }
        // Pointer/array init: generateExpression materializes decay + conversion if any.
        auto* init = declarator.getInitializer();
        auto* holder = declarator.getHolder();
        // visitChildren already visited init; only materialize delayed decay/conversion.
        materializeArrayDecay(*init, instructions);
        std::string valueName = init->getResultSymbol()->getName();
        if (const std::string* convertTo = init->getResultConversionTarget()) {
            instructions.push_back(std::make_unique<Assign>(valueName, *convertTo));
            valueName = *convertTo;
        }
        // If still array-typed (no semantic decay), form &arr for pointer holders.
        if (holder->getType().isPointer() && init->hasResultSymbol()
                && init->getResultSymbol()->getType().isArray()) {
            if (auto* addr = init->getLvalueSymbol()) {
                instructions.push_back(std::make_unique<Assign>(addr->getName(), holder->getName()));
            } else {
                instructions.push_back(std::make_unique<AddressOf>(
                        init->getResultSymbol()->getName(), holder->getName()));
            }
            return;
        }
        instructions.push_back(std::make_unique<Assign>(valueName, holder->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    // Nested brace lists (array of structs) must evaluate leaf expressions so
    // StructFieldInit sources hold values before LvalueAssign (git options[]).
    expression.visitElements(*this);
}

void CodeGeneratingVisitor::visit(ast::CompoundLiteralExpression& expression) {
    if (expression.getInitializer()) {
        expression.getInitializer()->accept(*this);
    }
    if (!expression.getObjectSymbol()) {
        return;
    }
    emitStructFieldInits(instructions, expression.getObjectSymbol()->getName(),
            expression.getStructFieldInits());
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    generateExpression(*arrayAccess.getLeftOperand());
    generateExpression(*arrayAccess.getRightOperand());

    std::string baseName = arrayAccess.leftOperandSymbol()->getName();
    bool baseIsArray = arrayAccess.baseIsArray();
    // Member / nested array bases already expose the object address as an lvalue.
    resolveArrayBase(baseName, baseIsArray, arrayAccess.getLeftOperand());
    const std::string indexName = arrayAccess.rightOperandSymbol()->getName();
    const std::string addrName = arrayAccess.getLvalue()->getName();
    const std::string resultName = arrayAccess.getResultSymbol()->getName();

    instructions.push_back(std::make_unique<IndexAddress>(
            baseName, indexName, arrayAccess.getElementSize(), addrName, baseIsArray));
    // Array-typed results (multi-dim rows): address only — same as MemberAccess
    // array members. Loading would clobber the pointer home when result aliases
    // the lvalue after decay, and is wrong for T[N] rvalues that decay to T*.
    if (arrayAccess.expressionType().isArray()) {
        if (addrName != resultName) {
            instructions.push_back(std::make_unique<Assign>(addrName, resultName));
        }
    } else {
        const type::Type& elemType = arrayAccess.getResultSymbol()->getType();
        instructions.push_back(std::make_unique<Dereference>(addrName, addrName, resultName,
                type::memoryAccessSizeBytes(elemType), type::memoryAccessIsSigned(elemType)));
    }
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    using ast::BuiltinLowering;
    using ast::lookupBuiltin;

    const auto kind = functionCall.getBuiltinKind();
    if (kind != ast::FunctionCall::BuiltinKind::None) {
        auto desc = lookupBuiltin(kind, "", functionCall.getVaArgResultType());
        const auto& args = functionCall.getArgumentList();
        for (const auto& arg : args) {
            generateExpression(*arg);
        }
        const std::string result = functionCall.getResultSymbol()->getName();
        if (!desc) {
            return;
        }
        switch (desc->lowering) {
        case BuiltinLowering::ConstantZero:
            instructions.push_back(std::make_unique<AssignConstant>(
                    std::to_string(product_approx::foldConstantP()), result));
            return;
        case BuiltinLowering::VaStart: {
            if (args.empty()) {
                return;
            }
            std::string lastStorage;
            if (args.size() >= 2) {
                lastStorage = args[1]->getResultSymbol()->getName();
            }
            instructions.push_back(std::make_unique<VaOp>(
                    VaOp::start(args[0]->getResultSymbol()->getName(), std::move(lastStorage))));
            return;
        }
        case BuiltinLowering::VaEnd:
            instructions.push_back(std::make_unique<VaOp>(VaOp::end()));
            return;
        case BuiltinLowering::VaCopy:
            if (args.size() < 2) {
                return;
            }
            instructions.push_back(std::make_unique<VaOp>(
                    VaOp::copy(args[0]->getResultSymbol()->getName(),
                            args[1]->getResultSymbol()->getName())));
            return;
        case BuiltinLowering::VaArg: {
            if (args.empty()) {
                return;
            }
            type::Type retTy = functionCall.getResultSymbol()->getType();
            int accessSize = type::valueSizeBytes(retTy);
            if (accessSize > 8) {
                accessSize = 8;
            }
            if (retTy.isPointer()) {
                accessSize = 8;
            }
            bool isFloating = type::isFloating(retTy);
            // Load extension policy (kind-aware; pointers are not sign-extended).
            bool isSigned = type::memoryAccessIsSigned(retTy);
            instructions.push_back(std::make_unique<VaOp>(
                    VaOp::arg(args[0]->getResultSymbol()->getName(), result, accessSize,
                            isFloating, isSigned)));
            return;
        }
        case BuiltinLowering::CallAs: {
            if (args.empty()) {
                return;
            }
            const std::string arg = args[0]->getResultSymbol()->getName();
            const char* callee = desc->syntheticName ? desc->syntheticName : "malloc";
            instructions.push_back(std::make_unique<Argument>(arg));
            instructions.push_back(std::make_unique<Call>(callee, false, ""));
            instructions.push_back(std::make_unique<Retrieve>(result));
            return;
        }
        case BuiltinLowering::BuiltinOp: {
            if (args.empty()) {
                return;
            }
            BuiltinOp::Kind opKind = BuiltinOp::Kind::Ctz;
            using BK = ast::FunctionCall::BuiltinKind;
            switch (desc->kind) {
            case BK::Bswap16: opKind = BuiltinOp::Kind::Bswap16; break;
            case BK::Bswap32: opKind = BuiltinOp::Kind::Bswap32; break;
            case BK::Bswap64: opKind = BuiltinOp::Kind::Bswap64; break;
            case BK::Ctz:     opKind = BuiltinOp::Kind::Ctz; break;
            default: break;
            }
            instructions.push_back(std::make_unique<BuiltinOp>(
                    opKind, args[0]->getResultSymbol()->getName(), result));
            return;
        }
        }
        return;
    }

    generateExpression(*functionCall.getOperandExpression());
    // Evaluate all arguments first without materializing array decay. Nested calls
    // emit their own Call/Retrieve fully. Deferring AddressOf keeps compound-literal
    // / array objects live until after later args (e.g. sizeof) finish allocating
    // temps — otherwise sizeof element-size homes can clobber the array storage.
    const auto& argumentList = functionCall.getArgumentList();
    for (std::size_t i { 0 }; i < argumentList.size(); ++i) {
        argumentList.at(i)->accept(*this);
    }
    for (std::size_t i { 0 }; i < argumentList.size(); ++i) {
        auto& arg = *argumentList.at(i);
        materializeArrayDecay(arg, instructions);
        std::string argSymbol = arg.getResultSymbol()->getName();
        if (const std::string* convertTo = arg.getResultConversionTarget()) {
            instructions.push_back(std::make_unique<Assign>(argSymbol, *convertTo));
            argSymbol = *convertTo;
        }
        instructions.push_back(std::make_unique<Argument>(argSymbol));
    }

    // System V: aggregates larger than 16 bytes return via hidden pointer (sret).
    // Match callee: no sret for variadic (ObjectAbi typeNeedsMemoryReturn(type, variadic)).
    std::string memoryReturnDest;
    if (!functionCall.getSymbol()->returnType().isVoid()) {
        const type::Type& retType = functionCall.getSymbol()->returnType();
        const bool variadic = functionCall.getSymbol()->getType().isVariadic();
        if (type::object_abi::typeNeedsMemoryReturn(retType, variadic)) {
            memoryReturnDest = functionCall.getResultSymbol()->getName();
        }
    }

    if (functionCall.isIndirect()) {
        // Callee address is in the operand result symbol.
        instructions.push_back(std::make_unique<Call>(
                functionCall.operandSymbol()->getName(), true, memoryReturnDest));
    } else {
        instructions.push_back(std::make_unique<Call>(
                functionCall.getSymbol()->getName(), false, memoryReturnDest));
    }
    // Void calls have no result symbol / type set.
    if (!functionCall.getSymbol()->returnType().isVoid()) {
        // Pass the same sret decision as Call (do not re-derive at retrieve).
        instructions.push_back(std::make_unique<Retrieve>(
                functionCall.getResultSymbol()->getName(), !memoryReturnDest.empty()));
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (identifier.hasFoldedConstant()) {
        instructions.push_back(std::make_unique<AssignConstant>(
                std::to_string(identifier.getFoldedConstant()), identifier.getResultSymbol()->getName()));
    } else if (identifier.hasFunctionDesignator()) {
        // Load address of function into the result temporary.
        instructions.push_back(std::make_unique<FunctionAddress>(
                identifier.getFunctionDesignator(), identifier.getResultSymbol()->getName()));
    }
}


void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so C suffixes (ul/ULL) and char escapes
    // never reach NASM as raw lexemes.
    std::string immediate;
    if (type::isFloating(constant.expressionType())) {
        // Keep full precision as IEEE bits; evaluateConstant would truncate.
        unsigned long long bits = 0;
        if (!type::floatingLiteralBits(constant.getValue(), bits)) {
            throw std::runtime_error { "invalid floating constant: " + constant.getValue() };
        }
        std::ostringstream hex;
        hex << "0x" << std::hex << bits;
        immediate = hex.str();
    } else {
        long value;
        if (constant.evaluateConstant(value)) {
            // Prefer hex for values outside signed 32-bit so NASM does not warn
            // "signed dword exceeds bounds" on 64-bit immediates.
            auto bits = static_cast<unsigned long long>(value);
            if (bits > 0x7fffffffull) {
                std::ostringstream hex;
                hex << "0x" << std::hex << bits;
                immediate = hex.str();
            } else {
                immediate = std::to_string(static_cast<long long>(value));
            }
        } else {
            immediate = constant.getValue();
            while (!immediate.empty()) {
                char c = immediate.back();
                if (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                    immediate.pop_back();
                } else {
                    break;
                }
            }
        }
    }
    instructions.push_back(std::make_unique<AssignConstant>(immediate, constant.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    instructions.push_back(
        std::make_unique<AssignConstant>(stringLiteral.getConstantSymbol(), stringLiteral.getResultSymbol()->getName())
    );
}


void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclarator& declaration) {
    // Array bounds are constant-folded when computing the type; do not emit code for them
    // (parameter arrays would otherwise produce assigns outside any procedure).
    (void)declaration;
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}


void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    // Semantic analysis skips setSymbol when the definition is invalid (e.g. name conflicts).
    if (!function.hasSymbol()) {
        return;
    }

    function.visitDeclarator(*this);

    // Generate the body first so temp live ranges are known, then pack the frame.
    auto instructionsBak = std::move(instructions);
    function.visitBody(*this);
    auto functionBody = toBasicBlocks(std::move(instructions));
    for (auto& bb : functionBody) {
        if (!bb->terminates()) {
            bb->appendInstruction(std::make_unique<VoidReturn>());
        }
    }

    std::vector<Value> values = packFrameValues(function.getLocalVariables(), functionBody);

    std::vector<Value> arguments;
    for (auto& argumentSymbol : function.getArguments()) {
        // Full type::Type kept on FrameSymbol; size/signedness/kind via TypeQuery.
        arguments.push_back(frameSymbolFrom(argumentSymbol).toValue());
    }
    const bool variadic = function.getSymbol()->getType().isVariadic();
    bool isStatic = false;
    for (const auto& storage : function.getReturnTypeSpecifiers().getStorageSpecifiers()) {
        if (storage.getStorage() == ast::Storage::STATIC) {
            isStatic = true;
            break;
        }
    }
    // System V memory return for aggregates > 16 bytes (git strbuf, config store_create_section).
    // Same predicate as Call sites (variadic suppresses sret on both sides).
    const type::Type& retType = function.getSymbol()->returnType();
    const bool memoryReturn = type::object_abi::typeNeedsMemoryReturn(retType, variadic);

    instructions = std::move(instructionsBak);
    instructions.push_back(std::make_unique<StartProcedure>(
            function.getSymbol()->getName(), std::move(values), std::move(arguments),
            variadic, isStatic, memoryReturn));
    for (auto& bb : functionBody) {
        instructions.push_back(std::move(bb));
    }
    instructions.push_back(std::make_unique<EndProcedure>(function.getSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

std::vector<std::unique_ptr<Quadruple>> CodeGeneratingVisitor::getQuadruples() {
    return std::move(instructions);
}

std::vector<std::unique_ptr<BasicBlock>> toBasicBlocks(std::vector<std::unique_ptr<Quadruple>> instructions) {
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks {};

    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>();

    std::vector<std::unique_ptr<Quadruple>> bbInstructions;
    for (auto& instruction : instructions) {
        if (bb->terminates() || instruction->isLabel()) {
            basicBlocks.push_back(std::move(bb));
            bb = std::make_unique<BasicBlock>();
            basicBlocks.back()->markTerminates();
        }
        bb->appendInstruction(std::move(instruction));
    }
    basicBlocks.push_back(std::move(bb));

    return basicBlocks;
}

} // namespace codegen
