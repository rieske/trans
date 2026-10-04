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
#include <limits>
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

int sameRoot(int id, const std::unordered_map<int, int>& sameAs) {
    for (int n = 0; n < 8; ++n) {
        const auto it = sameAs.find(id);
        if (it == sameAs.end()) {
            return id;
        }
        id = it->second;
    }
    return id;
}

void dropSame(std::unordered_map<int, int>& sameAs, int id) {
    if (id == kNoSymbol) {
        return;
    }
    sameAs.erase(id);
    for (auto it = sameAs.begin(); it != sameAs.end(); ) {
        if (it->second == id) {
            it = sameAs.erase(it);
        } else {
            ++it;
        }
    }
}

std::optional<PendingCompare> pendingFromCompare(const Instruction& inst, std::size_t index,
        const std::unordered_map<int, unsigned long long>& known,
        const std::unordered_map<int, int>& sameAs, const ValueIndex& values) {
    if (inst.op == Op::ZeroCompare) {
        const auto value = known.find(inst.arg0);
        const Value* src = findValue(values, inst.arg0);
        if (value == known.end() || !isFoldableInteger(src)) {
            return std::nullopt;
        }
        return PendingCompare { index, value->second, 0ull, src->getSizeInBytes() };
    }
    if (inst.op == Op::ValueCompare) {
        const Value* lhs = findValue(values, inst.arg0);
        const Value* rhs = findValue(values, inst.arg1);
        if (lhs && rhs && isFoldableInteger(lhs) && isFoldableInteger(rhs) && !lhs->isVolatile()
                && !rhs->isVolatile() && lhs->getSizeInBytes() == rhs->getSizeInBytes()
                && sameRoot(inst.arg0, sameAs) == sameRoot(inst.arg1, sameAs)) {
            return PendingCompare { index, 0ull, 0ull, lhs->getSizeInBytes() };
        }
        const auto left = known.find(inst.arg0);
        const auto right = known.find(inst.arg1);
        if (left == known.end() || right == known.end() || !isFoldableInteger(lhs)
                || !isFoldableInteger(rhs)
                || lhs->getSizeInBytes() != rhs->getSizeInBytes()) {
            return std::nullopt;
        }
        return PendingCompare { index, left->second, right->second, lhs->getSizeInBytes() };
    }
    return std::nullopt;
}

// Escapes are collected during the walk, not up front as in copyPropagate and
// eliminateDeadTemps. Sound because facts only flow forward: a symbol cannot be aliased
// before its AddressOf is reached, and `known` is dropped at any label not entered solely
// by fallthrough, so nothing survives a back edge into a later AddressOf.
const Instruction* defBefore(const std::vector<Instruction>& body, int id, const Instruction* self) {
    const Instruction* found = nullptr;
    for (const auto& inst : body) {
        if (&inst == self) {
            break;
        }
        if (inst.result == id) {
            found = &inst;
        }
    }
    return found;
}

bool unchangedBetween(const std::vector<Instruction>& body, const Instruction* inner,
        const Instruction* outer, int variable, int innerConst) {
    bool seen = false;
    for (const auto& inst : body) {
        if (&inst == inner) {
            seen = true;
            continue;
        }
        if (!seen) {
            continue;
        }
        if (&inst == outer) {
            return true;
        }
        if (inst.op == Op::Call || inst.op == Op::LvalueAssign || inst.op == Op::VaStart
                || inst.op == Op::VaArg || inst.op == Op::VaCopy) {
            return false;
        }
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        for (int def : refs.defs) {
            if (def == variable || def == innerConst) {
                return false;
            }
        }
    }
    return false;
}

int useCount(const std::vector<Instruction>& body, int id) {
    int count = 0;
    for (const auto& inst : body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        for (int use : refs.uses) {
            if (use == id) {
                ++count;
            }
        }
    }
    return count;
}

bool combineConst(Instruction& inst, std::vector<Instruction>& body,
        std::unordered_map<int, unsigned long long>& known, const ValueIndex& values,
        IrStringTable& strings) {
    if (inst.op != Op::Add && inst.op != Op::Sub) {
        return false;
    }
    if (operandIsVolatile(values, inst.arg0) || operandIsVolatile(values, inst.arg1)) {
        return false;
    }
    const bool leftConst = known.count(inst.arg0) != 0;
    const bool rightConst = known.count(inst.arg1) != 0;
    if (leftConst == rightConst) {
        return false;
    }
    if (inst.op == Op::Sub && !rightConst) {
        return false;
    }
    const int constId = rightConst ? inst.arg1 : inst.arg0;
    const int otherId = rightConst ? inst.arg0 : inst.arg1;
    const Instruction* inner = defBefore(body, otherId, &inst);
    if (inner == nullptr || (inner->op != Op::Add && inner->op != Op::Sub)) {
        return false;
    }
    if (operandIsVolatile(values, inner->arg0) || operandIsVolatile(values, inner->arg1)) {
        return false;
    }
    const bool innerLeftConst = known.count(inner->arg0) != 0;
    const bool innerRightConst = known.count(inner->arg1) != 0;
    if (innerLeftConst == innerRightConst) {
        return false;
    }
    if (inner->op == Op::Sub && !innerRightConst) {
        return false;
    }
    const int innerConst = innerRightConst ? inner->arg1 : inner->arg0;
    const int variable = innerRightConst ? inner->arg0 : inner->arg1;
    if (inner->result == variable || !unchangedBetween(body, inner, &inst, variable, innerConst)) {
        return false;
    }
    const Value* dest = findValue(values, inst.result);
    const Value* innerDest = findValue(values, inner->result);
    if (!isFoldableInteger(dest) || !isFoldableInteger(innerDest)
            || dest->getSizeInBytes() != innerDest->getSizeInBytes() || dest->getSizeInBytes() <= 0) {
        return false;
    }
    const int bytes = dest->getSizeInBytes();
    const int width = bitWidth(bytes);
    const long long c1 = asSigned(known.at(innerConst), width);
    const long long c2 = asSigned(known.at(constId), width);
    const long long min = width >= 64 ? std::numeric_limits<long long>::min() : -(1LL << (width - 1));
    const long long max = width >= 64 ? std::numeric_limits<long long>::max() : (1LL << (width - 1)) - 1;
    auto addFits = [&](long long a, long long b, long long& out) {
        if (b > 0) {
            if (a > max - b) {
                return false;
            }
        } else if (b < 0) {
            if (a < min - b) {
                return false;
            }
        }
        out = a + b;
        return true;
    };
    auto negFits = [&](long long v, long long& out) {
        if (v == min) {
            return false;
        }
        out = -v;
        return true;
    };
    long long combined = 0;
    Op resultOp = inst.op;
    if (inner->op == inst.op) {
        if (!addFits(c1, c2, combined)) {
            return false;
        }
    } else {
        long long left = c1;
        long long right = c2;
        if (inner->op == Op::Sub && !negFits(c1, left)) {
            return false;
        }
        if (inst.op == Op::Sub && !negFits(c2, right)) {
            return false;
        }
        if (!addFits(left, right, combined)) {
            return false;
        }
        if (combined == 0) {
            inst = ir::assign(variable, inst.result);
            return true;
        }
        if (combined < 0) {
            long long magnitude = 0;
            if (!negFits(combined, magnitude)) {
                return false;
            }
            combined = magnitude;
            resultOp = Op::Sub;
        } else {
            resultOp = Op::Add;
        }
    }
    if (useCount(body, constId) != 1) {
        return false;
    }
    Instruction* reaching = nullptr;
    for (auto& earlier : body) {
        if (&earlier == &inst) {
            break;
        }
        if (earlier.result == constId) {
            reaching = &earlier;
        }
    }
    if (reaching == nullptr || reaching->op != Op::AssignConstant || reaching->arg1 != kNoSymbol) {
        return false;
    }
    const unsigned long long bits = static_cast<unsigned long long>(combined) & widthMask(bytes);
    reaching->arg0 = strings.intern(util::wordImmediate(bits));
    known[constId] = bits;
    inst.op = resultOp;
    inst.arg0 = variable;
    inst.arg1 = constId;
    return true;
}

bool combineBitConst(Instruction& inst, std::vector<Instruction>& body,
        std::unordered_map<int, unsigned long long>& known, const ValueIndex& values,
        IrStringTable& strings) {
    const bool shift = inst.op == Op::Shl || inst.op == Op::Shr;
    const bool bit = inst.op == Op::And || inst.op == Op::Or || inst.op == Op::Xor;
    if (!shift && !bit) {
        return false;
    }
    if (operandIsVolatile(values, inst.arg0) || operandIsVolatile(values, inst.arg1)) {
        return false;
    }
    const bool leftConst = known.count(inst.arg0) != 0;
    const bool rightConst = known.count(inst.arg1) != 0;
    if (leftConst == rightConst) {
        return false;
    }
    if (shift && !rightConst) {
        return false;
    }
    const int constId = rightConst ? inst.arg1 : inst.arg0;
    const int otherId = rightConst ? inst.arg0 : inst.arg1;
    const Instruction* inner = defBefore(body, otherId, &inst);
    if (inner == nullptr || inner->op != inst.op) {
        return false;
    }
    if (shift && inner->imm != inst.imm) {
        return false;
    }
    if (operandIsVolatile(values, inner->arg0) || operandIsVolatile(values, inner->arg1)) {
        return false;
    }
    const bool innerLeftConst = known.count(inner->arg0) != 0;
    const bool innerRightConst = known.count(inner->arg1) != 0;
    if (innerLeftConst == innerRightConst) {
        return false;
    }
    if (shift && !innerRightConst) {
        return false;
    }
    const int innerConst = innerRightConst ? inner->arg1 : inner->arg0;
    const int variable = innerRightConst ? inner->arg0 : inner->arg1;
    if (inner->result == variable || !unchangedBetween(body, inner, &inst, variable, innerConst)) {
        return false;
    }
    const Value* dest = findValue(values, inst.result);
    const Value* innerDest = findValue(values, inner->result);
    if (!isFoldableInteger(dest) || !isFoldableInteger(innerDest)
            || dest->getSizeInBytes() != innerDest->getSizeInBytes() || dest->getSizeInBytes() <= 0) {
        return false;
    }
    const Value* outerConst = findValue(values, constId);
    const Value* innerConstValue = findValue(values, innerConst);
    if (!isFoldableInteger(outerConst) || !isFoldableInteger(innerConstValue)) {
        return false;
    }
    const int bytes = dest->getSizeInBytes();
    const unsigned long long mask = widthMask(bytes);
    unsigned long long combined = 0;
    bool assignVar = false;
    bool assignConst = false;
    if (shift) {
        const int width = bitWidth(bytes);
        const long long s1 = asSigned(known.at(innerConst), bitWidth(innerConstValue->getSizeInBytes()));
        const long long s2 = asSigned(known.at(constId), bitWidth(outerConst->getSizeInBytes()));
        if (s1 < 0 || s2 < 0 || s1 >= width || s2 >= width || s1 + s2 >= width) {
            return false;
        }
        const long long sum = s1 + s2;
        if (sum == 0) {
            assignVar = true;
        } else {
            combined = static_cast<unsigned long long>(sum);
        }
    } else if (outerConst->getSizeInBytes() != bytes || innerConstValue->getSizeInBytes() != bytes) {
        return false;
    } else if (inst.op == Op::And) {
        combined = (known.at(innerConst) & mask) & (known.at(constId) & mask);
        assignConst = combined == 0;
        assignVar = combined == mask;
    } else if (inst.op == Op::Or) {
        combined = (known.at(innerConst) & mask) | (known.at(constId) & mask);
        assignVar = combined == 0;
        assignConst = combined == mask;
    } else {
        combined = (known.at(innerConst) & mask) ^ (known.at(constId) & mask);
        assignVar = combined == 0;
    }
    if (assignVar) {
        inst = ir::assign(variable, inst.result);
        return true;
    }
    if (assignConst) {
        inst = ir::assignConstant(strings.intern(util::wordImmediate(combined)), inst.result);
        return true;
    }
    if (useCount(body, constId) != 1) {
        return false;
    }
    Instruction* reaching = nullptr;
    for (auto& earlier : body) {
        if (&earlier == &inst) {
            break;
        }
        if (earlier.result == constId) {
            reaching = &earlier;
        }
    }
    if (reaching == nullptr || reaching->op != Op::AssignConstant || reaching->arg1 != kNoSymbol) {
        return false;
    }
    const unsigned long long bits = combined & widthMask(outerConst->getSizeInBytes());
    reaching->arg0 = strings.intern(util::wordImmediate(bits));
    known[constId] = bits;
    inst.arg0 = variable;
    inst.arg1 = constId;
    return true;
}

} // namespace

FoldResult foldConstants(Procedure& procedure, IrStringTable& strings) {
    const ValueIndex values = indexValues(procedure);
    const auto preds = labelPredCounts(procedure.body);
    std::unordered_map<int, unsigned long long> known;
    std::unordered_map<int, int> sameAs;
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
                sameAs.clear();
            }
            pending.reset();
            fall = true;
            continue;
        }
        if (auto repl = tryFold(inst, known, values, strings)) {
            inst = *repl;
            result.changed = true;
        } else if (combineConst(inst, procedure.body, known, values, strings)
                || combineBitConst(inst, procedure.body, known, values, strings)) {
            result.changed = true;
        }
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase != kNoSymbol) {
            escaped.insert(refs.addressOfBase);
            known.erase(refs.addressOfBase);
            dropSame(sameAs, refs.addressOfBase);
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
        if (inst.op == Op::Assign) {
            const Value* src = findValue(values, inst.arg0);
            const Value* dest = findValue(values, inst.result);
            dropSame(sameAs, inst.result);
            if (isFoldableInteger(src) && isFoldableInteger(dest) && !src->isVolatile()
                    && !dest->isVolatile() && escaped.count(inst.result) == 0
                    && escaped.count(inst.arg0) == 0) {
                sameAs[inst.result] = sameRoot(inst.arg0, sameAs);
            }
        } else {
            for (int def : refs.defs) {
                dropSame(sameAs, def);
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
        } else if (auto next = pendingFromCompare(inst, i, known, sameAs, values)) {
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
    if (op == Op::Div || op == Op::Mod) {
        return false;
    }
    return true;
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
        } else if (inst.op == Op::VaStart || inst.op == Op::VaArg || inst.op == Op::VaCopy) {
            valueOf.clear();
            pointsTo.clear();
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

namespace {

struct ExprKey {
    Op op;
    int arg0;
    int arg1;
    int imm;
    bool operator==(const ExprKey& other) const {
        return op == other.op && arg0 == other.arg0 && arg1 == other.arg1 && imm == other.imm;
    }
};

struct ExprKeyHash {
    std::size_t operator()(const ExprKey& key) const {
        return (static_cast<std::size_t>(key.op) * 1315423911u)
                ^ (static_cast<std::size_t>(key.arg0) << 1)
                ^ (static_cast<std::size_t>(key.arg1) << 17)
                ^ (static_cast<std::size_t>(key.imm) << 8);
    }
};

bool isNumberableOp(Op op) {
    switch (op) {
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
    case Op::UnaryMinus:
    case Op::UnaryNot:
        return true;
    default:
        return false;
    }
}

bool sameValueKind(const ValueIndex& values, int left, int right) {
    const Value* from = findValue(values, left);
    const Value* to = findValue(values, right);
    return from && to && from->getType() == to->getType()
            && from->getSizeInBytes() == to->getSizeInBytes()
            && from->getClassification().gprExtend == to->getClassification().gprExtend;
}

bool stableOperand(const ValueIndex& values, int id, const std::unordered_set<int>& addressTaken) {
    if (id == kNoSymbol) {
        return true;
    }
    const Value* value = findValue(values, id);
    return value && !value->isVolatile() && addressTaken.count(id) == 0;
}

using NumberMap = std::unordered_map<ExprKey, int, ExprKeyHash>;

void killNumber(NumberMap& number, int id) {
    if (id == kNoSymbol) {
        return;
    }
    for (auto it = number.begin(); it != number.end(); ) {
        if (it->first.arg0 == id || it->first.arg1 == id || it->second == id) {
            it = number.erase(it);
        } else {
            ++it;
        }
    }
}

NumberMap intersectSameTemp(const NumberMap& left, const NumberMap& right) {
    NumberMap out;
    for (const auto& entry : left) {
        const auto found = right.find(entry.first);
        if (found != right.end() && found->second == entry.second) {
            out.emplace(entry.first, entry.second);
        }
    }
    return out;
}

void valueNumber(Procedure& procedure) {
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
    std::unordered_map<int, std::vector<NumberMap>> reached;
    NumberMap number;
    bool fall = true;
    for (auto& inst : procedure.body) {
        if (inst.op == Op::Label) {
            const int predsHere = preds.count(inst.arg0) != 0 ? preds.at(inst.arg0) : 0;
            const auto incoming = reached.find(inst.arg0);
            const int jumps = incoming == reached.end() ? 0 : static_cast<int>(incoming->second.size());
            const int forward = jumps + (fall ? 1 : 0);
            if (!(fall && predsHere == 1)) {
                if (predsHere == 2 && forward == 2 && incoming != reached.end()) {
                    if (fall) {
                        number = intersectSameTemp(number, incoming->second[0]);
                    } else {
                        number = intersectSameTemp(incoming->second[0], incoming->second[1]);
                    }
                } else {
                    number.clear();
                }
            }
            fall = true;
            continue;
        }
        const Value* dest = findValue(values, inst.result);
        const bool numberable = isNumberableOp(inst.op) && dest && dest->isExpressionTemp()
                && !dest->isVolatile() && addressTaken.count(inst.result) == 0
                && stableOperand(values, inst.arg0, addressTaken)
                && stableOperand(values, inst.arg1, addressTaken);
        if (numberable) {
            killNumber(number, inst.result);
            const ExprKey key { inst.op, inst.arg0, inst.arg1, inst.imm };
            const auto found = number.find(key);
            if (found != number.end() && sameValueKind(values, found->second, inst.result)) {
                inst = ir::assign(found->second, inst.result);
            } else if (inst.result != inst.arg0 && inst.result != inst.arg1) {
                number[key] = inst.result;
            }
        } else {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            for (int def : refs.defs) {
                killNumber(number, def);
            }
        }
        if (inst.op == Op::Call) {
            number.clear();
        }
        if (inst.op == Op::Jump && inst.arg0 != kNoSymbol) {
            reached[inst.arg0].push_back(number);
        }
        if (instructionTransfersControl(inst)
                && (inst.op != Op::Jump || inst.cond == JumpCondition::UNCONDITIONAL)) {
            fall = false;
        }
    }
}

struct ConstKey {
    unsigned long long bits;
    int bytes;
    type::sysv::GprExtend extend;
    bool operator==(const ConstKey& other) const {
        return bits == other.bits && bytes == other.bytes && extend == other.extend;
    }
};

struct ConstKeyHash {
    std::size_t operator()(const ConstKey& key) const {
        return (key.bits * 1315423911ull)
                ^ (static_cast<std::size_t>(key.bytes) << 1)
                ^ (static_cast<std::size_t>(key.extend) << 17);
    }
};

void killReusableConst(std::unordered_map<ConstKey, int, ConstKeyHash>& live,
        std::unordered_map<int, int>& alias, int id) {
    if (id == kNoSymbol) {
        return;
    }
    for (auto it = live.begin(); it != live.end(); ) {
        if (it->second == id) {
            it = live.erase(it);
        } else {
            ++it;
        }
    }
    killCopy(alias, id);
}

// Share one live constant temp so later value numbering sees the same operand id.
// Runs after combineConst, which needs each folded constant to have a single use.
void reuseAssignConstants(Procedure& procedure, IrStringTable& strings) {
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
    std::unordered_map<ConstKey, int, ConstKeyHash> live;
    std::unordered_map<int, int> alias;
    bool fall = true;
    for (auto& inst : procedure.body) {
        if (inst.op == Op::Label) {
            const bool keep = fall && preds.count(inst.arg0) != 0 && preds.at(inst.arg0) == 1;
            if (!keep) {
                live.clear();
                alias.clear();
            }
            fall = true;
            continue;
        }
        rewriteValueUses(inst, alias);
        const Value* dest = findValue(values, inst.result);
        const bool reusable = inst.op == Op::AssignConstant && inst.arg1 == kNoSymbol
                && dest && dest->isExpressionTemp() && isFoldableInteger(dest)
                && addressTaken.count(inst.result) == 0;
        if (reusable) {
            const auto bits = parseConstBits(strings.get(inst.arg0));
            killReusableConst(live, alias, inst.result);
            if (bits) {
                const ConstKey key {
                    *bits & widthMask(dest->getSizeInBytes()),
                    dest->getSizeInBytes(),
                    dest->getClassification().gprExtend,
                };
                const auto found = live.find(key);
                if (found != live.end() && found->second != inst.result
                        && sameValueKind(values, found->second, inst.result)) {
                    const int canon = found->second;
                    const int dup = inst.result;
                    inst = ir::assign(canon, dup);
                    alias[dup] = canon;
                } else {
                    live.emplace(key, inst.result);
                }
            }
        } else {
            SymbolRefs refs;
            collectSymbolRefs(inst, refs);
            for (int def : refs.defs) {
                killReusableConst(live, alias, def);
            }
        }
        if (instructionTransfersControl(inst)
                && (inst.op != Op::Jump || inst.cond == JumpCondition::UNCONDITIONAL)) {
            fall = false;
        }
    }
}

} // namespace

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
            reuseAssignConstants(procedure, ir.strings);
            valueNumber(procedure);
            copyPropagate(procedure);
        }
        foldToFixpoint();
        for (auto& procedure : ir.procedures) {
            forwardLocalLoads(procedure);
            eliminateDeadTemps(procedure);
        }
    }
    return ir;
}

} // namespace codegen
