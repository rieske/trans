#include "IrPasses.h"
#include "codegen/IrBuilders.h"
#include "IrInline.h"
#include "Licm.h"
#include "StrengthReduce.h"

#include "Cfg.h"
#include "Liveness.h"
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

using ValueIndex = std::unordered_map<int, const Value*>;

ValueIndex indexValues(const Procedure& procedure) {
    ValueIndex index;
    for (const auto& value : procedure.frame.locals) {
        index.emplace(value.id(), &value);
    }
    for (const auto& value : procedure.frame.arguments) {
        index.emplace(value.id(), &value);
    }
    return index;
}

const Value* findValue(const ValueIndex& index, int id) {
    const auto it = index.find(id);
    return it == index.end() ? nullptr : it->second;
}

bool isFoldableInteger(const Value* value) {
    return value && !value->isVolatile() && value->getType() == Type::INTEGRAL
            && value->getSizeInBytes() > 0 && value->getSizeInBytes() <= 8;
}

bool operandIsVolatile(const ValueIndex& index, int id) {
    const Value* value = findValue(index, id);
    return value && value->isVolatile();
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

std::optional<unsigned long long> knownBits(
        const std::unordered_map<int, unsigned long long>& known, int id) {
    const auto it = known.find(id);
    if (it == known.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<Instruction> tryAlgebraicIdentity(const Instruction& inst,
        const std::unordered_map<int, unsigned long long>& known, const ValueIndex& values,
        IrStringTable& strings) {
    if (operandIsVolatile(values, inst.arg0) || operandIsVolatile(values, inst.arg1)) {
        return std::nullopt;
    }
    const auto isZero = [&](int id) {
        const auto bits = knownBits(known, id);
        return bits && *bits == 0;
    };
    const auto isOne = [&](int id) {
        const auto bits = knownBits(known, id);
        return bits && *bits == 1;
    };
    const auto same = inst.arg0 == inst.arg1;
    const auto asAssign = [&](int src) { return ir::assign(src, inst.result); };
    const auto asZero = [&] {
        return ir::assignConstant(strings.intern(util::wordImmediate(0)), inst.result);
    };
    switch (inst.op) {
    case Op::Add:
        if (isZero(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        if (isZero(inst.arg0)) {
            return asAssign(inst.arg1);
        }
        return std::nullopt;
    case Op::Sub:
        if (isZero(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        if (same) {
            return asZero();
        }
        return std::nullopt;
    case Op::Mul:
        if (isZero(inst.arg0) || isZero(inst.arg1)) {
            return asZero();
        }
        if (isOne(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        if (isOne(inst.arg0)) {
            return asAssign(inst.arg1);
        }
        return std::nullopt;
    case Op::Div:
        if (isOne(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        return std::nullopt;
    case Op::Mod:
        if (isOne(inst.arg1)) {
            return asZero();
        }
        return std::nullopt;
    case Op::And:
        if (same) {
            return asAssign(inst.arg0);
        }
        if (isZero(inst.arg0) || isZero(inst.arg1)) {
            return asZero();
        }
        return std::nullopt;
    case Op::Or:
        if (same) {
            return asAssign(inst.arg0);
        }
        if (isZero(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        if (isZero(inst.arg0)) {
            return asAssign(inst.arg1);
        }
        return std::nullopt;
    case Op::Xor:
        if (same) {
            return asZero();
        }
        if (isZero(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        if (isZero(inst.arg0)) {
            return asAssign(inst.arg1);
        }
        return std::nullopt;
    case Op::Shl:
    case Op::Shr:
        if (isZero(inst.arg1)) {
            return asAssign(inst.arg0);
        }
        return std::nullopt;
    default:
        return std::nullopt;
    }
}

std::optional<Instruction> tryFold(const Instruction& inst,
        const std::unordered_map<int, unsigned long long>& known, const ValueIndex& values,
        IrStringTable& strings) {
    const Value* dest = findValue(values, inst.result);
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
        if (left != known.end() && right != known.end()) {
            const Value* lhs = findValue(values, inst.arg0);
            const Value* rhs = findValue(values, inst.arg1);
            if (isFoldableInteger(lhs) && isFoldableInteger(rhs)) {
                const auto bits = evalBinary(inst.op, left->second, right->second, destBytes, inst.imm);
                if (bits) {
                    return folded(*bits);
                }
            }
        }
        return tryAlgebraicIdentity(inst, known, values, strings);
    }
    case Op::UnaryMinus:
    case Op::UnaryNot:
    case Op::Widen: {
        const auto operand = known.find(inst.arg0);
        if (operand == known.end()) {
            return std::nullopt;
        }
        const Value* src = findValue(values, inst.arg0);
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
        const std::unordered_map<int, unsigned long long>& known, const ValueIndex& values) {
    if (inst.op == Op::ZeroCompare) {
        const auto value = known.find(inst.arg0);
        const Value* src = findValue(values, inst.arg0);
        if (value == known.end() || !isFoldableInteger(src)) {
            return std::nullopt;
        }
        return PendingCompare { index, value->second, 0ull, src->getSizeInBytes() };
    }
    if (inst.op == Op::ValueCompare) {
        const auto left = known.find(inst.arg0);
        const auto right = known.find(inst.arg1);
        const Value* lhs = findValue(values, inst.arg0);
        const Value* rhs = findValue(values, inst.arg1);
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
FoldResult foldConstants(Procedure& procedure, IrStringTable& strings) {
    const ValueIndex values = indexValues(procedure);
    const auto preds = labelPredCounts(procedure.body);
    std::unordered_map<int, unsigned long long> known;
    std::unordered_set<int> escaped;
    std::optional<PendingCompare> pending;
    std::vector<char> drop(procedure.body.size(), 0);
    FoldResult result;
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
        if (auto repl = tryFold(inst, known, values, strings)) {
            inst = *repl;
            result.changed = true;
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
            const Value* dest = findValue(values, inst.result);
            if (isFoldableInteger(dest)) {
                if (auto bits = parseConstBits(strings.get(inst.arg0))) {
                    known[inst.result] = *bits & widthMask(dest->getSizeInBytes());
                    recorded = true;
                }
            }
        } else if (inst.op == Op::Assign && escaped.count(inst.result) == 0) {
            const auto src = known.find(inst.arg0);
            const Value* srcVal = findValue(values, inst.arg0);
            const Value* dest = findValue(values, inst.result);
            if (src != known.end() && isFoldableInteger(srcVal) && isFoldableInteger(dest)) {
                unsigned long long bits = src->second;
                const int srcBytes = srcVal->getSizeInBytes();
                const int destBytes = dest->getSizeInBytes();
                if (srcBytes > 0 && srcBytes < destBytes
                        && srcVal->getClassification().gprExtend == type::sysv::GprExtend::Sign) {
                    bits = static_cast<unsigned long long>(asSigned(bits, bitWidth(srcBytes)));
                }
                known[inst.result] = bits & widthMask(destBytes);
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
                result.changed = true;
                result.controlFlow = true;
            }
            pending.reset();
        } else if (auto next = pendingFromCompare(inst, i, known, values)) {
            pending = next;
        } else {
            pending.reset();
        }

        if (!drop[i] && instructionTransfersControl(inst)
                && (inst.op != Op::Jump || inst.cond == JumpCondition::UNCONDITIONAL)) {
            fall = false;
        }
    }

    if (result.changed) {
        std::vector<Instruction> kept;
        kept.reserve(procedure.body.size());
        for (std::size_t i = 0; i < procedure.body.size(); ++i) {
            if (!drop[i]) {
                kept.push_back(procedure.body[i]);
            }
        }
        procedure.body = std::move(kept);
    }
    return result;
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
        rewriteValueUse(inst.arg0, copy);
        rewriteValueUse(inst.arg1, copy);
        return;
    case Op::PointerAdd:
        rewriteValueUse(inst.arg0, copy);
        return;
    case Op::VaStart:
    case Op::VaCopy:
        rewriteValueUse(inst.arg0, copy);
        rewriteValueUse(inst.arg1, copy);
        return;
    case Op::Assign:
    case Op::Dereference:
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

bool isEligibleCopy(const Instruction& inst, const ValueIndex& values,
        const std::unordered_set<int>& addressTaken) {
    if (inst.op != Op::Assign) {
        return false;
    }
    if (addressTaken.count(inst.arg0) != 0 || addressTaken.count(inst.result) != 0) {
        return false;
    }
    const Value* src = findValue(values, inst.arg0);
    const Value* dest = findValue(values, inst.result);
    return src && dest && src->isExpressionTemp() && dest->isExpressionTemp()
            && src->getType() == dest->getType()
            && src->getSizeInBytes() == dest->getSizeInBytes()
            && src->getClassification().gprExtend == dest->getClassification().gprExtend;
}

} // namespace

void copyPropagate(Procedure& procedure) {
    const ValueIndex values = indexValues(procedure);
    std::unordered_set<int> addressTaken;
    for (const auto& inst : procedure.body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase != kNoSymbol) {
            addressTaken.insert(refs.addressOfBase);
        }
    }

    const auto preds = labelPredCounts(procedure.body);
    std::unordered_map<int, int> copy;
    bool fall = true;
    for (auto& inst : procedure.body) {
        if (inst.op == Op::Label) {
            const bool keepCopy = fall && preds.count(inst.arg0) && preds.at(inst.arg0) == 1;
            if (!keepCopy) {
                copy.clear();
            }
            fall = true;
            continue;
        }
        rewriteValueUses(inst, copy);
        if (inst.op == Op::Call) {
            copy.clear();
        } else if (isEligibleCopy(inst, values, addressTaken)) {
            killCopy(copy, inst.result);
            const auto src = copy.find(inst.arg0);
            copy[inst.result] = src == copy.end() ? inst.arg0 : src->second;
        } else {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            for (int def : refs.defs) {
                killCopy(copy, def);
            }
        }
        if (instructionTransfersControl(inst)
                && (inst.op != Op::Jump || inst.cond == JumpCondition::UNCONDITIONAL)) {
            fall = false;
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
    case Op::PointerAdd:
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

bool isDeadValueDef(const ValueIndex& values, Op op, int id, bool liveAfter,
        const std::unordered_set<int>& addressTaken) {
    if (addressTaken.count(id) != 0 || liveAfter) {
        return false;
    }
    const Value* dest = findValue(values, id);
    if (!dest || dest->isVolatile()) {
        return false;
    }
    if (dest->isExpressionTemp()) {
        return true;
    }
    return op != Op::Div && op != Op::Mod;
}

} // namespace

void eliminateDeadTemps(Procedure& procedure) {
    const ValueIndex values = indexValues(procedure);
    TempLiveness live;
    for (;;) {
        live = computeTempLiveness(procedure);
        std::vector<Instruction> kept;
        kept.reserve(procedure.body.size());
        for (int i = static_cast<int>(procedure.body.size()) - 1; i >= 0; --i) {
            const Instruction& inst = procedure.body[static_cast<std::size_t>(i)];
            const bool liveAfter = live.resultLiveAfter[static_cast<std::size_t>(i)] != 0;
            const bool volatileUse = operandIsVolatile(values, inst.arg0)
                    || operandIsVolatile(values, inst.arg1)
                    || operandIsVolatile(values, inst.result);
            if (isDeadAssignable(inst.op) && !volatileUse
                    && isDeadValueDef(values, inst.op, inst.result, liveAfter, live.addressTaken)) {
                continue;
            }
            kept.push_back(inst);
        }
        std::reverse(kept.begin(), kept.end());
        if (kept.size() == procedure.body.size()) {
            procedure.body = std::move(kept);
            break;
        }
        procedure.body = std::move(kept);
    }

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
                return !local.isVolatile() && remaining.count(local.id()) == 0
                        && live.addressTaken.count(local.id()) == 0;
            }),
            locals.end());
}

void applyCfgPasses(Procedure& procedure, int optLevel) {
    Cfg cfg = buildCfg(procedure.body);
    if (optLevel >= 1) {
        cfg = threadJumps(std::move(cfg));
        cfg = eliminateUnreachable(std::move(cfg));
    }
    procedure.body = flattenCfg(eliminateJumpToNext(std::move(cfg)));
}

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel) {
    for (auto& procedure : ir.procedures) {
        applyCfgPasses(procedure, optLevel);
    }
    return ir;
}

void forwardLocalLoads(Procedure& procedure) {
    const ValueIndex values = indexValues(procedure);
    std::unordered_map<int, std::unordered_set<int>> addrOf;
    bool grewTargets = true;
    auto addTarget = [&](int pointer, int object) {
        if (addrOf[pointer].insert(object).second) {
            grewTargets = true;
        }
    };
    auto copyTargets = [&](int from, int to) {
        const auto it = addrOf.find(from);
        if (it == addrOf.end()) {
            return;
        }
        auto& dest = addrOf[to];
        for (int object : it->second) {
            if (dest.insert(object).second) {
                grewTargets = true;
            }
        }
    };
    while (grewTargets) {
        grewTargets = false;
        for (const auto& inst : procedure.body) {
            if (inst.op == Op::AddressOf) {
                addTarget(inst.result, inst.arg0);
            } else if (inst.op == Op::Assign) {
                copyTargets(inst.arg0, inst.result);
            } else if ((inst.op == Op::IndexAddress || inst.op == Op::FieldAddress)
                    && symbols::addressBaseUsesLea(inst.baseMode)) {
                addTarget(inst.result, inst.arg0);
            } else if (inst.op == Op::IndexAddress || inst.op == Op::FieldAddress) {
                copyTargets(inst.arg0, inst.result);
            } else if (inst.op == Op::Dereference) {
                const auto src = addrOf.find(inst.arg0);
                if (src == addrOf.end()) {
                    continue;
                }
                const std::vector<int> objects(src->second.begin(), src->second.end());
                for (int object : objects) {
                    copyTargets(object, inst.result);
                }
            }
        }
    }
    std::unordered_set<int> escaped;
    auto escapes = [&](int id) {
        const auto it = addrOf.find(id);
        if (it == addrOf.end()) {
            return;
        }
        for (int object : it->second) {
            escaped.insert(object);
        }
    };
    for (const auto& inst : procedure.body) {
        if (inst.op == Op::Assign) {
            if (findValue(values, inst.result) == nullptr) {
                escapes(inst.arg0);
            }
            continue;
        }
        if (inst.op == Op::AddressOf || inst.op == Op::Dereference
                || inst.op == Op::IndexAddress || inst.op == Op::FieldAddress) {
            continue;
        }
        if (inst.op == Op::LvalueAssign) {
            escapes(inst.arg0);
            continue;
        }
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        for (int id : refs.uses) {
            escapes(id);
        }
        for (int id : refs.defs) {
            escapes(id);
        }
    }
    for (const auto& value : procedure.frame.locals) {
        if (value.isVolatile()) {
            escaped.insert(value.id());
        }
    }
    for (const auto& value : procedure.frame.arguments) {
        if (value.isVolatile()) {
            escaped.insert(value.id());
        }
    }
    bool grew = true;
    while (grew) {
        grew = false;
        for (const auto& entry : addrOf) {
            if (escaped.count(entry.first) == 0) {
                continue;
            }
            for (int object : entry.second) {
                if (escaped.insert(object).second) {
                    grew = true;
                }
            }
        }
    }
    for (auto it = addrOf.begin(); it != addrOf.end(); ) {
        for (auto object = it->second.begin(); object != it->second.end(); ) {
            if (escaped.count(*object) != 0) {
                object = it->second.erase(object);
            } else {
                ++object;
            }
        }
        if (it->second.empty()) {
            it = addrOf.erase(it);
        } else {
            ++it;
        }
    }

    struct StoreSlots {
        enum { kEmpty = -1 };
        std::unordered_map<int, int> slot;
        std::unordered_map<int, std::vector<int>> holders;

        int get(int id) const {
            const auto it = slot.find(id);
            return it == slot.end() ? kEmpty : it->second;
        }

        void forget(int obj) {
            const auto it = slot.find(obj);
            if (it != slot.end()) {
                std::erase(holders[it->second], obj);
                slot.erase(it);
            }
        }

        void set(int obj, int stored) {
            forget(obj);
            slot[obj] = stored;
            holders[stored].push_back(obj);
        }

        void kill(int id) {
            forget(id);
            const auto it = holders.find(id);
            if (it == holders.end()) {
                return;
            }
            for (const int obj : it->second) {
                slot.erase(obj);
            }
            holders.erase(it);
        }

        void clear() {
            slot.clear();
            holders.clear();
        }

        void dropEscaped(const std::unordered_set<int>& escapedIds) {
            for (auto it = slot.begin(); it != slot.end(); ) {
                if (escapedIds.count(it->first) != 0 || escapedIds.count(it->second) != 0) {
                    std::erase(holders[it->second], it->first);
                    it = slot.erase(it);
                } else {
                    ++it;
                }
            }
        }
    };

    const auto preds = labelPredCounts(procedure.body);
    std::unordered_map<int, int> pointsTo;
    StoreSlots valueOf;
    bool fall = true;
    auto sameKind = [&](int stored, int loaded) {
        const Value* from = findValue(values, stored);
        const Value* to = findValue(values, loaded);
        return from && to && from->getType() == to->getType()
                && from->getSizeInBytes() == to->getSizeInBytes()
                && from->getClassification().gprExtend == to->getClassification().gprExtend;
    };
    auto rewrite = [&](int& id) {
        const int stored = valueOf.get(id);
        if (stored != StoreSlots::kEmpty && sameKind(stored, id)) {
            id = stored;
        }
    };
    auto isObject = [&](int local) {
        if (escaped.count(local) != 0) {
            return false;
        }
        for (const auto& entry : addrOf) {
            if (entry.second.count(local) != 0) {
                return true;
            }
        }
        return false;
    };
    auto remember = [&](int local, int stored) {
        if (!isObject(local)) {
            return;
        }
        const Value* object = findValue(values, local);
        if (!object || object->isVolatile()) {
            return;
        }
        const int known = valueOf.get(stored);
        const int storedId = known == StoreSlots::kEmpty ? stored : known;
        if (!sameKind(storedId, local)) {
            valueOf.forget(local);
            return;
        }
        valueOf.set(local, storedId);
    };
    auto leak = [&](int id) {
        std::vector<int> work;
        std::unordered_set<int> seen;
        const auto pointed = pointsTo.find(id);
        if (pointed != pointsTo.end()) {
            work.push_back(pointed->second);
        }
        const auto may = addrOf.find(id);
        if (may != addrOf.end()) {
            work.insert(work.end(), may->second.begin(), may->second.end());
        }
        while (!work.empty()) {
            const int obj = work.back();
            work.pop_back();
            if (!seen.insert(obj).second) {
                continue;
            }
            escaped.insert(obj);
            valueOf.forget(obj);
            const auto next = pointsTo.find(obj);
            if (next != pointsTo.end()) {
                work.push_back(next->second);
                pointsTo.erase(next);
            }
            const auto more = addrOf.find(obj);
            if (more != addrOf.end()) {
                work.insert(work.end(), more->second.begin(), more->second.end());
            }
        }
    };

    for (auto& inst : procedure.body) {
        if (inst.op == Op::Label) {
            const bool keep = fall && preds.count(inst.arg0) != 0 && preds.at(inst.arg0) == 1;
            if (!keep) {
                pointsTo.clear();
                valueOf.clear();
            }
            fall = true;
            continue;
        }
        if (inst.op == Op::AddressOf) {
            pointsTo[inst.result] = inst.arg0;
        } else if (inst.op == Op::Assign) {
            rewrite(inst.arg0);
            const auto srcPtr = pointsTo.find(inst.arg0);
            if (srcPtr != pointsTo.end()) {
                pointsTo[inst.result] = srcPtr->second;
            } else {
                pointsTo.erase(inst.result);
            }
            valueOf.kill(inst.result);
            remember(inst.result, inst.arg0);
        } else if (inst.op == Op::LvalueAssign) {
            rewrite(inst.arg0);
            const auto ptr = pointsTo.find(inst.result);
            if (ptr == pointsTo.end()) {
                valueOf.clear();
                pointsTo.clear();
            } else {
                valueOf.kill(ptr->second);
                remember(ptr->second, inst.arg0);
                const auto target = pointsTo.find(inst.arg0);
                if (target != pointsTo.end()) {
                    pointsTo[ptr->second] = target->second;
                } else {
                    pointsTo.erase(ptr->second);
                }
            }
        } else if (inst.op == Op::Dereference) {
            const auto ptr = pointsTo.find(inst.arg0);
            if (ptr != pointsTo.end()) {
                const int known = valueOf.get(ptr->second);
                const auto inner = pointsTo.find(ptr->second);
                if (known != StoreSlots::kEmpty && sameKind(known, inst.result)) {
                    const int src = known;
                    inst = ir::assign(src, inst.result);
                    const auto srcPtr = pointsTo.find(src);
                    if (srcPtr != pointsTo.end()) {
                        pointsTo[inst.result] = srcPtr->second;
                    } else if (inner != pointsTo.end()) {
                        pointsTo[inst.result] = inner->second;
                    }
                } else if (inner != pointsTo.end()) {
                    pointsTo[inst.result] = inner->second;
                }
            }
        } else if (inst.op == Op::IndexAddress || inst.op == Op::FieldAddress) {
            if (inst.op == Op::IndexAddress) {
                rewrite(inst.arg1);
            }
            int base = -1;
            if (symbols::addressBaseUsesLea(inst.baseMode)) {
                if (escaped.count(inst.arg0) == 0) {
                    base = inst.arg0;
                }
            } else {
                const auto ptr = pointsTo.find(inst.arg0);
                if (ptr != pointsTo.end()) {
                    base = ptr->second;
                }
            }
            if (base >= 0) {
                pointsTo[inst.result] = base;
            } else {
                pointsTo.erase(inst.result);
            }
        } else if (inst.op == Op::Argument) {
            leak(inst.arg0);
        } else if (inst.op == Op::Call) {
            for (auto it = pointsTo.begin(); it != pointsTo.end(); ) {
                if (escaped.count(it->first) != 0 || escaped.count(it->second) != 0) {
                    it = pointsTo.erase(it);
                } else {
                    ++it;
                }
            }
            valueOf.dropEscaped(escaped);
        } else {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            auto defined = [&](int id) {
                return std::find(refs.defs.begin(), refs.defs.end(), id) != refs.defs.end();
            };
            if (inst.op != Op::Jump && !defined(inst.arg0)) {
                rewrite(inst.arg0);
            }
            if (inst.op != Op::Jump && !defined(inst.arg1)) {
                rewrite(inst.arg1);
            }
            for (int id : refs.defs) {
                valueOf.kill(id);
            }
        }
        if (instructionTransfersControl(inst)
                && (inst.op != Op::Jump || inst.cond == JumpCondition::UNCONDITIONAL)) {
            fall = false;
        }
    }
}

IntermediateRepresentation runIrPasses(IntermediateRepresentation ir, int optLevel) {
    ir = sealProcedures(std::move(ir));
    ir = applyCfgPasses(std::move(ir), optLevel);
    if (optLevel >= 1) {
        const InlineStats stats = inlineProcedures(ir);
        for (int i : stats.dirtyCallers) {
            applyCfgPasses(ir.procedures[static_cast<std::size_t>(i)], optLevel);
        }
        auto foldToFixpoint = [&]() {
            for (int iter = 0; iter < 8; ++iter) {
                bool changed = false;
                for (auto& procedure : ir.procedures) {
                    const FoldResult one = foldConstants(procedure, ir.strings);
                    if (one.controlFlow) {
                        applyCfgPasses(procedure, optLevel);
                    }
                    changed = changed || one.changed;
                }
                if (!changed) {
                    break;
                }
            }
        };
        foldToFixpoint();
        for (auto& procedure : ir.procedures) {
            eliminateDeadTemps(procedure);
        }
        for (auto& procedure : ir.procedures) {
            const LicmStats licm = hoistLoopInvariants(procedure, ir.strings);
            const StrengthReduceStats sr = strengthReduce(procedure, ir.strings);
            if (licm.inserted != 0 || licm.hoisted != 0 || sr.reduced != 0 || sr.inserted != 0) {
                applyCfgPasses(procedure, optLevel);
            }
        }
        foldToFixpoint();
        for (auto& procedure : ir.procedures) {
            copyPropagate(procedure);
            forwardLocalLoads(procedure);
            eliminateDeadTemps(procedure);
        }
    }
    return ir;
}

} // namespace codegen
