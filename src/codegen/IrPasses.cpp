#include "IrPasses.h"

#include "Cfg.h"
#include "SymbolRefs.h"
#include "util/ImmediateFormat.h"
#include "util/IntegerLiteral.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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

void foldConstants(Procedure& procedure, IrStringTable& strings) {
    std::unordered_map<int, unsigned long long> known;
    std::unordered_set<int> escaped;
    for (auto& inst : procedure.body) {
        if (inst.op == Op::Label) {
            known.clear();
            continue;
        }
        if (auto repl = tryFold(inst, known, procedure, strings)) {
            inst = *repl;
        }
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase != kNoSymbol) {
            escaped.insert(refs.addressOfBase);
            known.erase(refs.addressOfBase);
        }
        if (inst.op == Op::AssignConstant && inst.arg1 == kNoSymbol
                && escaped.count(inst.result) == 0) {
            const Value* dest = findValue(procedure, inst.result);
            if (isFoldableInteger(dest)) {
                if (auto bits = parseConstBits(strings.get(inst.arg0))) {
                    known[inst.result] = *bits & widthMask(dest->getSizeInBytes());
                    continue;
                }
            }
        }
        if (inst.op == Op::Assign && escaped.count(inst.result) == 0) {
            const auto src = known.find(inst.arg0);
            const Value* dest = findValue(procedure, inst.result);
            if (src != known.end() && isFoldableInteger(dest)) {
                known[inst.result] = src->second & widthMask(dest->getSizeInBytes());
                continue;
            }
        }
        for (int def : refs.defs) {
            known.erase(def);
        }
    }
}

IntermediateRepresentation applyCfgPasses(IntermediateRepresentation ir, int optLevel) {
    for (auto& procedure : ir.procedures) {
        Cfg cfg = buildCfg(procedure.body);
        if (optLevel >= 1) {
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
        for (auto& procedure : ir.procedures) {
            foldConstants(procedure, ir.strings);
        }
    }
    return ir;
}

} // namespace codegen
