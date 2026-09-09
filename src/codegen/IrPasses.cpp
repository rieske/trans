#include "IrPasses.h"
#include "codegen/IrBuilders.h"

#include "Cfg.h"
#include "SymbolRefs.h"
#include "util/ImmediateFormat.h"
#include "util/IntegerLiteral.h"

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace codegen {

void sealProcedure(Procedure& procedure) {
    if (procedure.body.empty() || !instructionTransfersControl(procedure.body.back())) {
        procedure.body.push_back(ir::voidReturn());
    }
}

IntermediateRepresentation sealProcedures(IntermediateRepresentation ir) {
    for (auto& procedure : ir.procedures) {
        sealProcedure(procedure);
    }
    return ir;
}

namespace {

const Value* findValue(const Procedure& procedure, int id) {
    for (const auto& value : procedure.frame.locals) {
        if (value.id() == id) {
            return &value;
        }
    }
    for (const auto& value : procedure.frame.arguments) {
        if (value.id() == id) {
            return &value;
        }
    }
    return nullptr;
}

bool isFoldableInteger(const Value* value) {
    return value && value->getType() == Type::INTEGRAL && value->getSizeInBytes() > 0
            && value->getSizeInBytes() <= 8;
}

unsigned long long widthMask(int bytes) {
    if (bytes >= 8) {
        return ~0ull;
    }
    return (1ull << (bytes * 8)) - 1;
}

int bitWidth(int bytes) {
    return bytes * 8;
}

std::optional<unsigned long long> parseConstBits(const std::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }
    bool neg = false;
    std::string token = text;
    if (token.front() == '-') {
        neg = true;
        token.erase(0, 1);
    }
    util::IntegerLiteral lit;
    if (!util::parseIntegerLiteral(token, lit) || lit.value > ~0ull) {
        return std::nullopt;
    }
    unsigned long long bits = static_cast<unsigned long long>(lit.value);
    if (neg) {
        bits = 0ull - bits;
    }
    return bits;
}

long long asSigned(unsigned long long bits, int width) {
    if (width >= 64) {
        return static_cast<long long>(bits);
    }
    const unsigned long long sign = 1ull << (width - 1);
    const unsigned long long mask = widthMask(width / 8);
    bits &= mask;
    if (bits & sign) {
        return static_cast<long long>(bits | ~mask);
    }
    return static_cast<long long>(bits);
}

std::optional<unsigned long long> evalBinary(Op op, unsigned long long left, unsigned long long right,
        int bytes, int imm) {
    const unsigned long long mask = widthMask(bytes);
    const int width = bitWidth(bytes);
    left &= mask;
    right &= mask;
    switch (op) {
    case Op::Add:
        return (left + right) & mask;
    case Op::Sub:
        return (left - right) & mask;
    case Op::Mul:
        return (left * right) & mask;
    case Op::And:
        return left & right;
    case Op::Or:
        return left | right;
    case Op::Xor:
        return left ^ right;
    case Op::Shl:
        if (right >= static_cast<unsigned long long>(width)) {
            return std::nullopt;
        }
        return (left << right) & mask;
    case Op::Shr:
        if (right >= static_cast<unsigned long long>(width)) {
            return std::nullopt;
        }
        if (imm) {
            return static_cast<unsigned long long>(asSigned(left, width) >> right) & mask;
        }
        return left >> right;
    case Op::Div:
    case Op::Mod:
        if (right == 0) {
            return std::nullopt;
        }
        if (imm) {
            const long long a = asSigned(left, width);
            const long long b = asSigned(right, width);
            if (b == -1 && a == asSigned(1ull << (width - 1), width)) {
                return std::nullopt;
            }
            const long long q = (op == Op::Div) ? (a / b) : (a % b);
            return static_cast<unsigned long long>(q) & mask;
        }
        return (op == Op::Div) ? (left / right) : (left % right);
    default:
        return std::nullopt;
    }
}

std::optional<unsigned long long> evalUnary(Op op, unsigned long long operand, int bytes, int imm,
        int destBytes) {
    const unsigned long long mask = widthMask(bytes);
    operand &= mask;
    switch (op) {
    case Op::UnaryMinus:
        return (0ull - operand) & mask;
    case Op::UnaryNot:
        return (~operand) & mask;
    case Op::Widen: {
        const unsigned long long destMask = widthMask(destBytes);
        if (imm) {
            return static_cast<unsigned long long>(asSigned(operand, bitWidth(bytes))) & destMask;
        }
        return operand & destMask;
    }
    default:
        return std::nullopt;
    }
}

std::optional<Instruction> tryFold(const Instruction& inst,
        const std::unordered_map<int, unsigned long long>& known, const Procedure& procedure,
        IrStringTable& strings) {
    const Value* dest = findValue(procedure, inst.result);
    if (!isFoldableInteger(dest)) {
        return std::nullopt;
    }
    const int destBytes = dest->getSizeInBytes();

    auto folded = [&](unsigned long long bits) {
        bits &= widthMask(destBytes);
        return ir::assignConstant(strings.intern(util::wordImmediate(bits)), inst.result);
    };

    switch (inst.op) {
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::Div:
    case Op::Mod:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::Shl:
    case Op::Shr: {
        const auto left = known.find(inst.arg0);
        const auto right = known.find(inst.arg1);
        if (left == known.end() || right == known.end()) {
            return std::nullopt;
        }
        const Value* lhs = findValue(procedure, inst.arg0);
        const Value* rhs = findValue(procedure, inst.arg1);
        if (!isFoldableInteger(lhs) || !isFoldableInteger(rhs)) {
            return std::nullopt;
        }
        const auto bits = evalBinary(inst.op, left->second, right->second, destBytes, inst.imm);
        if (!bits) {
            return std::nullopt;
        }
        return folded(*bits);
    }
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Widen: {
        const auto operand = known.find(inst.arg0);
        if (operand == known.end()) {
            return std::nullopt;
        }
        const Value* src = findValue(procedure, inst.arg0);
        if (!isFoldableInteger(src)) {
            return std::nullopt;
        }
        const auto bits = evalUnary(inst.op, operand->second, src->getSizeInBytes(), inst.imm,
                destBytes);
        if (!bits) {
            return std::nullopt;
        }
        return folded(*bits);
    }
    default:
        return std::nullopt;
    }
}

} // namespace

namespace {

std::unordered_map<int, int> labelPredCounts(const std::vector<Instruction>& body) {
    std::unordered_map<int, int> preds;
    bool fall = true;
    for (const auto& inst : body) {
        if (inst.op == Op::Label) {
            if (fall && inst.arg0 != kNoSymbol) {
                ++preds[inst.arg0];
            }
            fall = true;
            continue;
        }
        if (inst.op == Op::Jump && inst.arg0 != kNoSymbol) {
            ++preds[inst.arg0];
            if (inst.cond == JumpCondition::UNCONDITIONAL) {
                fall = false;
            }
            continue;
        }
        if (instructionTransfersControl(inst)) {
            fall = false;
        }
    }
    return preds;
}

std::optional<bool> compareJumpTaken(JumpCondition cond, unsigned long long left,
        unsigned long long right, int bytes, bool signedRel) {
    const unsigned long long mask = widthMask(bytes);
    const int width = bitWidth(bytes);
    left &= mask;
    right &= mask;
    switch (cond) {
    case JumpCondition::IF_EQUAL:
        return left == right;
    case JumpCondition::IF_NOT_EQUAL:
        return left != right;
    case JumpCondition::IF_BELOW:
        if (signedRel) {
            return asSigned(left, width) < asSigned(right, width);
        }
        return left < right;
    case JumpCondition::IF_ABOVE:
        if (signedRel) {
            return asSigned(left, width) > asSigned(right, width);
        }
        return left > right;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        if (signedRel) {
            return asSigned(left, width) <= asSigned(right, width);
        }
        return left <= right;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        if (signedRel) {
            return asSigned(left, width) >= asSigned(right, width);
        }
        return left >= right;
    case JumpCondition::UNCONDITIONAL:
        return std::nullopt;
    }
    return std::nullopt;
}

struct PendingCompare {
    std::size_t index { 0 };
    unsigned long long left { 0 };
    unsigned long long right { 0 };
    int bytes { 0 };
};

std::optional<PendingCompare> pendingFromCompare(const Instruction& inst, std::size_t index,
        const std::unordered_map<int, unsigned long long>& known, const Procedure& procedure) {
    if (inst.op == Op::ZeroCompare) {
        const auto value = known.find(inst.arg0);
        const Value* src = findValue(procedure, inst.arg0);
        if (value == known.end() || !isFoldableInteger(src)) {
            return std::nullopt;
        }
        return PendingCompare { index, value->second, 0ull, src->getSizeInBytes() };
    }
    if (inst.op == Op::ValueCompare) {
        const auto left = known.find(inst.arg0);
        const auto right = known.find(inst.arg1);
        const Value* lhs = findValue(procedure, inst.arg0);
        const Value* rhs = findValue(procedure, inst.arg1);
        if (left == known.end() || right == known.end() || !isFoldableInteger(lhs)
                || !isFoldableInteger(rhs)
                || lhs->getSizeInBytes() != rhs->getSizeInBytes()) {
            return std::nullopt;
        }
        return PendingCompare { index, left->second, right->second, lhs->getSizeInBytes() };
    }
    return std::nullopt;
}

} // namespace

// Escapes are collected during the walk, not up front as in copyPropagate and
// eliminateDeadTemps. Sound because facts only flow forward: a symbol cannot be aliased
// before its AddressOf is reached, and `known` is dropped at any label not entered solely
// by fallthrough, so nothing survives a back edge into a later AddressOf.
bool foldConstants(Procedure& procedure, IrStringTable& strings) {
    const auto preds = labelPredCounts(procedure.body);
    std::unordered_map<int, unsigned long long> known;
    std::unordered_set<int> escaped;
    std::optional<PendingCompare> pending;
    std::vector<char> drop(procedure.body.size(), 0);
    bool changed = false;
    bool fall = true;

    for (std::size_t i = 0; i < procedure.body.size(); ++i) {
        Instruction& inst = procedure.body[i];
        if (inst.op == Op::Label) {
            const bool keepKnown = fall && preds.count(inst.arg0) && preds.at(inst.arg0) == 1;
            if (!keepKnown) {
                known.clear();
            }
            pending.reset();
            fall = true;
            continue;
        }
        if (auto repl = tryFold(inst, known, procedure, strings)) {
            inst = *repl;
            changed = true;
        }
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase != kNoSymbol) {
            escaped.insert(refs.addressOfBase);
            known.erase(refs.addressOfBase);
        }
        bool recorded = false;
        if (inst.op == Op::AssignConstant && inst.arg1 == kNoSymbol
                && escaped.count(inst.result) == 0) {
            const Value* dest = findValue(procedure, inst.result);
            if (isFoldableInteger(dest)) {
                if (auto bits = parseConstBits(strings.get(inst.arg0))) {
                    known[inst.result] = *bits & widthMask(dest->getSizeInBytes());
                    recorded = true;
                }
            }
        } else if (inst.op == Op::Assign && escaped.count(inst.result) == 0) {
            const auto src = known.find(inst.arg0);
            const Value* dest = findValue(procedure, inst.result);
            if (src != known.end() && isFoldableInteger(dest)) {
                known[inst.result] = src->second & widthMask(dest->getSizeInBytes());
                recorded = true;
            }
        }
        if (!recorded) {
            for (int def : refs.defs) {
                known.erase(def);
            }
        }

        if (inst.op == Op::Jump && inst.cond != JumpCondition::UNCONDITIONAL && pending) {
            const bool signedRel = inst.imm != 0;
            if (auto taken = compareJumpTaken(inst.cond, pending->left, pending->right,
                    pending->bytes, signedRel)) {
                drop[pending->index] = 1;
                if (*taken) {
                    inst.cond = JumpCondition::UNCONDITIONAL;
                } else {
                    drop[i] = 1;
                }
                changed = true;
            }
            pending.reset();
        } else if (auto next = pendingFromCompare(inst, i, known, procedure)) {
            pending = next;
        } else {
            pending.reset();
        }

        if (!drop[i] && instructionTransfersControl(inst)
                && (inst.op != Op::Jump || inst.cond == JumpCondition::UNCONDITIONAL)) {
            fall = false;
        }
    }

    if (changed) {
        std::vector<Instruction> kept;
        kept.reserve(procedure.body.size());
        for (std::size_t i = 0; i < procedure.body.size(); ++i) {
            if (!drop[i]) {
                kept.push_back(procedure.body[i]);
            }
        }
        procedure.body = std::move(kept);
    }
    return changed;
}

namespace {

void rewriteValueUse(int& id, const std::unordered_map<int, int>& copy) {
    const auto it = copy.find(id);
    if (it != copy.end()) {
        id = it->second;
    }
}

void rewriteValueUses(Instruction& inst, const std::unordered_map<int, int>& copy) {
    switch (inst.op) {
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::Div:
    case Op::Mod:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::Shl:
    case Op::Shr:
    case Op::ValueCompare:
    case Op::PointerOffset:
    case Op::PointerDiff:
    case Op::Dereference:
    case Op::VaStart:
    case Op::VaCopy:
        rewriteValueUse(inst.arg0, copy);
        rewriteValueUse(inst.arg1, copy);
        return;
    case Op::Assign:
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::CopyPart:
    case Op::Widen:
    case Op::Bswap:
    case Op::Ctz:
    case Op::Alloca:
    case Op::ZeroCompare:
    case Op::Argument:
    case Op::Return:
    case Op::VaArg:
        rewriteValueUse(inst.arg0, copy);
        return;
    case Op::LvalueAssign:
        rewriteValueUse(inst.arg0, copy);
        rewriteValueUse(inst.result, copy);
        return;
    case Op::IndexAddress:
        if (!symbols::addressBaseUsesLea(inst.baseMode)) {
            rewriteValueUse(inst.arg0, copy);
        }
        rewriteValueUse(inst.arg1, copy);
        return;
    case Op::FieldAddress:
        if (!symbols::addressBaseUsesLea(inst.baseMode)) {
            rewriteValueUse(inst.arg0, copy);
        }
        return;
    case Op::Call:
        if (inst.callIndirect) {
            rewriteValueUse(inst.arg0, copy);
        }
        rewriteValueUse(inst.memoryReturnDest, copy);
        return;
    case Op::AddressOf:
    case Op::Inc:
    case Op::Dec:
    case Op::AssignConstant:
    case Op::AssignLabelAddress:
    case Op::FunctionAddress:
    case Op::Jump:
    case Op::Label:
    case Op::VoidReturn:
    case Op::VaEnd:
    case Op::Retrieve:
        return;
    }
}

void killCopy(std::unordered_map<int, int>& copy, int id) {
    copy.erase(id);
    for (auto it = copy.begin(); it != copy.end(); ) {
        if (it->second == id) {
            it = copy.erase(it);
        } else {
            ++it;
        }
    }
}

bool isEligibleCopy(const Instruction& inst, const Procedure& procedure,
        const std::unordered_set<int>& addressTaken) {
    if (inst.op != Op::Assign) {
        return false;
    }
    if (addressTaken.count(inst.arg0) != 0 || addressTaken.count(inst.result) != 0) {
        return false;
    }
    const Value* src = findValue(procedure, inst.arg0);
    const Value* dest = findValue(procedure, inst.result);
    return src && dest && src->isExpressionTemp() && dest->isExpressionTemp()
            && src->getType() == dest->getType()
            && src->getSizeInBytes() == dest->getSizeInBytes()
            && src->getClassification().gprExtend == dest->getClassification().gprExtend;
}

} // namespace

void copyPropagate(Procedure& procedure) {
    std::unordered_set<int> addressTaken;
    for (const auto& inst : procedure.body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase != kNoSymbol) {
            addressTaken.insert(refs.addressOfBase);
        }
    }

    std::unordered_map<int, int> copy;
    for (auto& inst : procedure.body) {
        if (inst.op == Op::Label) {
            copy.clear();
            continue;
        }
        rewriteValueUses(inst, copy);
        if (inst.op == Op::Call) {
            copy.clear();
            continue;
        }
        if (isEligibleCopy(inst, procedure, addressTaken)) {
            killCopy(copy, inst.result);
            const auto src = copy.find(inst.arg0);
            copy[inst.result] = src == copy.end() ? inst.arg0 : src->second;
            continue;
        }
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        for (int def : refs.defs) {
            killCopy(copy, def);
        }
    }
}

namespace {

bool isDeadAssignable(Op op) {
    switch (op) {
    case Op::AssignConstant:
    case Op::Assign:
    case Op::AssignLabelAddress:
    case Op::FunctionAddress:
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Widen:
    case Op::CopyPart:
    case Op::Bswap:
    case Op::Ctz:
    case Op::Alloca:
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::Div:
    case Op::Mod:
    case Op::And:
    case Op::Or:
    case Op::Xor:
    case Op::Shl:
    case Op::Shr:
    case Op::PointerOffset:
    case Op::PointerDiff:
    case Op::AddressOf:
    case Op::Dereference:
    case Op::IndexAddress:
    case Op::FieldAddress:
        return true;
    default:
        return false;
    }
}

bool isDeadExpressionTempDef(const Procedure& procedure, int id,
        const std::unordered_set<int>& laterUses, const std::unordered_set<int>& addressTaken) {
    if (addressTaken.count(id) != 0 || laterUses.count(id) != 0) {
        return false;
    }
    const Value* dest = findValue(procedure, id);
    return dest && dest->isExpressionTemp();
}

} // namespace

void eliminateDeadTemps(Procedure& procedure) {
    std::unordered_set<int> addressTaken;
    for (const auto& inst : procedure.body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase != kNoSymbol) {
            addressTaken.insert(refs.addressOfBase);
        }
    }

    std::unordered_set<int> laterUses;
    std::vector<Instruction> kept;
    kept.reserve(procedure.body.size());
    for (int i = static_cast<int>(procedure.body.size()) - 1; i >= 0; --i) {
        const Instruction& inst = procedure.body[static_cast<std::size_t>(i)];
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (isDeadAssignable(inst.op)
                && isDeadExpressionTempDef(procedure, inst.result, laterUses, addressTaken)) {
            continue;
        }
        for (int use : refs.uses) {
            laterUses.insert(use);
        }
        kept.push_back(inst);
    }
    std::reverse(kept.begin(), kept.end());
    procedure.body = std::move(kept);

    std::unordered_set<int> remaining;
    for (const auto& inst : procedure.body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        remaining.insert(refs.uses.begin(), refs.uses.end());
        remaining.insert(refs.defs.begin(), refs.defs.end());
        if (refs.addressOfBase != kNoSymbol) {
            remaining.insert(refs.addressOfBase);
        }
    }
    auto& locals = procedure.frame.locals;
    locals.erase(std::remove_if(locals.begin(), locals.end(),
            [&](const Value& local) {
                return local.isExpressionTemp() && remaining.count(local.id()) == 0
                        && addressTaken.count(local.id()) == 0;
            }),
            locals.end());
}

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel) {
    for (auto& procedure : ir.procedures) {
        Cfg cfg = buildCfg(procedure.body);
        if (optLevel >= 1) {
            cfg = threadJumps(std::move(cfg));
            cfg = eliminateUnreachable(std::move(cfg));
        }
        procedure.body = flattenCfg(eliminateJumpToNext(std::move(cfg)));
    }
    return ir;
}

IntermediateRepresentation runIrPasses(IntermediateRepresentation ir, int optLevel) {
    ir = sealProcedures(std::move(ir));
    ir = applyCfgPasses(std::move(ir), optLevel);
    if (optLevel >= 1) {
        for (int iter = 0; iter < 8; ++iter) {
            bool folded = false;
            for (auto& procedure : ir.procedures) {
                if (foldConstants(procedure, ir.strings)) {
                    folded = true;
                }
            }
            ir = applyCfgPasses(std::move(ir), optLevel);
            if (!folded) {
                break;
            }
        }
        for (auto& procedure : ir.procedures) {
            copyPropagate(procedure);
            eliminateDeadTemps(procedure);
        }
    }
    return ir;
}

} // namespace codegen
