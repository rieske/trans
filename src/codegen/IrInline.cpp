#include "IrInline.h"

#include "IrBuilders.h"
#include "SymbolRefs.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace codegen {
namespace {

bool isVaOp(Op op) {
    return op == Op::VaStart || op == Op::VaArg || op == Op::VaCopy || op == Op::VaEnd;
}

} // namespace

bool isSetjmpFamily(std::string_view name) {
    return name == "setjmp" || name == "_setjmp" || name == "sigsetjmp" || name == "__sigsetjmp"
            || name == "longjmp" || name == "_longjmp" || name == "siglongjmp"
            || name == "__longjmp_chk";
}

int nonLabelCount(const std::vector<Instruction>& body) {
    int n = 0;
    for (const auto& inst : body) {
        if (inst.op != Op::Label) {
            ++n;
        }
    }
    return n;
}

bool calleeLooksUnsafe(const Procedure& callee, const IrStringTable& strings) {
    for (const auto& inst : callee.body) {
        if (isVaOp(inst.op)) {
            return true;
        }
        if (inst.op == Op::Call && !inst.callIndirect && isSetjmpFamily(strings.get(inst.arg0))) {
            return true;
        }
    }
    return false;
}

bool callIsEligible(const Instruction& call,
        const Procedure& caller,
        const Procedure& callee,
        const IrStringTable& strings,
        const InlineCaps& caps,
        int paramCount,
        int inlinesOnCaller,
        int inlinesInTu,
        bool calleeFinished) {
    if (call.op != Op::Call || call.callIndirect) {
        return false;
    }
    if (call.arg0 != callee.name) {
        return false;
    }
    if (callee.variadic || calleeLooksUnsafe(callee, strings)) {
        return false;
    }
    if (!calleeFinished || callee.name == caller.name) {
        return false;
    }
    if (nonLabelCount(callee.body) > caps.maxCalleeInsts) {
        return false;
    }
    if (nonLabelCount(caller.body) + nonLabelCount(callee.body) > caps.maxCallerInsts) {
        return false;
    }
    if (inlinesOnCaller >= caps.maxInlinesPerCaller || inlinesInTu >= caps.maxTotalInlines) {
        return false;
    }
    if (paramCount != static_cast<int>(callee.frame.arguments.size())) {
        return false;
    }
    if (callee.memoryReturn != (call.memoryReturnDest != kNoSymbol)) {
        return false;
    }
    return true;
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

std::unordered_set<int> privateValueIds(const Procedure& callee) {
    std::unordered_set<int> ids;
    for (const auto& value : callee.frame.locals) {
        ids.insert(value.id());
    }
    for (const auto& value : callee.frame.arguments) {
        ids.insert(value.id());
    }
    if (callee.sretId != kNoSymbol) {
        ids.insert(callee.sretId);
    }
    return ids;
}

std::unordered_set<int> privateLabelIds(const Procedure& callee) {
    std::unordered_set<int> ids;
    for (const auto& inst : callee.body) {
        if (inst.op == Op::Label) {
            ids.insert(inst.arg0);
        }
    }
    return ids;
}

bool idMentioned(const SymbolRefs& refs, int id) {
    if (refs.addressOfBase == id) {
        return true;
    }
    for (int use : refs.uses) {
        if (use == id) {
            return true;
        }
    }
    for (int def : refs.defs) {
        if (def == id) {
            return true;
        }
    }
    return false;
}

bool addressTakenIn(const std::vector<Instruction>& body, int id) {
    for (const auto& inst : body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (refs.addressOfBase == id) {
            return true;
        }
    }
    return false;
}

int internFresh(IrStringTable& strings, const std::string& prefix) {
    int n = 0;
    std::string name;
    do {
        name = prefix + std::to_string(n++);
    } while (strings.find(name) != kNoSymbol);
    return strings.intern(name);
}

int internInlinedAutomatic(IrStringTable& strings, int siteId, int sourceId) {
    std::string unique = strings.get(sourceId);
    if (unique.size() >= 2 && unique[0] == 'L' && unique[1] == '$') {
        unique = unique.substr(2);
    }
    std::string cand = "L$inl" + std::to_string(siteId) + "_" + unique;
    if (strings.find(cand) == kNoSymbol) {
        return strings.intern(cand);
    }
    int n = 0;
    std::string next;
    do {
        next = cand + "_" + std::to_string(n++);
    } while (strings.find(next) != kNoSymbol);
    return strings.intern(next);
}

int remapPrivateValue(IrStringTable& strings, int siteId, int sourceId) {
    const std::string& name = strings.get(sourceId);
    if (name.compare(0, 2, "$t") == 0) {
        return internFresh(strings, "$t");
    }
    if (name.compare(0, 3, "__t") == 0) {
        return internFresh(strings, "__t");
    }
    return internInlinedAutomatic(strings, siteId, sourceId);
}

void pushClone(Procedure& caller, const Value& src, int newId, bool asExprTemp) {
    Value clone { newId, 0, src.getType(), src.getSizeInBytes(), src.getClassification() };
    if (asExprTemp || src.isExpressionTemp()) {
        clone.markExpressionTemp();
    }
    caller.frame.locals.push_back(std::move(clone));
}

void rewriteInstructionIds(Instruction& inst, const std::unordered_map<int, int>& remap) {
    const auto rw = [&](int& id) {
        const auto it = remap.find(id);
        if (it != remap.end()) {
            id = it->second;
        }
    };
    switch (inst.op) {
    case Op::Label:
    case Op::Jump:
        rw(inst.arg0);
        return;
    case Op::FunctionAddress:
        rw(inst.result);
        return;
    case Op::AssignConstant:
        rw(inst.result);
        return;
    case Op::Call:
        if (inst.callIndirect) {
            rw(inst.arg0);
        }
        rw(inst.arg1);
        rw(inst.result);
        rw(inst.memoryReturnDest);
        return;
    default:
        rw(inst.arg0);
        rw(inst.arg1);
        rw(inst.result);
        rw(inst.memoryReturnDest);
        return;
    }
}

} // namespace

bool formalCanShareActual(const Procedure& callee, int formalId, int actualId,
        const Procedure& caller) {
    if (formalId == kNoSymbol || actualId == kNoSymbol) {
        return false;
    }
    const auto privateValues = privateValueIds(callee);
    bool formalDefOrAddr = false;
    bool actualMentioned = false;
    bool calleeHasLvalueAssign = false;
    bool calleeHasCall = false;
    bool calleeHasExternalDef = false;
    for (const auto& inst : callee.body) {
        SymbolRefs refs;
        collectSymbolRefs(inst, refs);
        if (idMentioned(refs, formalId) && (refs.addressOfBase == formalId
                || std::find(refs.defs.begin(), refs.defs.end(), formalId) != refs.defs.end())) {
            formalDefOrAddr = true;
        }
        if (privateValues.count(actualId) == 0 && idMentioned(refs, actualId)) {
            actualMentioned = true;
        }
        if (inst.op == Op::LvalueAssign) {
            calleeHasLvalueAssign = true;
        }
        if (inst.op == Op::Call) {
            calleeHasCall = true;
        }
        for (int def : refs.defs) {
            if (privateValues.count(def) == 0) {
                calleeHasExternalDef = true;
            }
        }
    }
    if (formalDefOrAddr || actualMentioned) {
        return false;
    }
    const Value* actual = findValue(caller, actualId);
    const bool c1 = actual && actual->isExpressionTemp() && !addressTakenIn(caller.body, actualId);
    const bool c2 = !calleeHasLvalueAssign && !calleeHasCall && !calleeHasExternalDef;
    return c1 || c2;
}

std::vector<Instruction> cloneCalleeBody(
        Procedure& caller,
        const Procedure& callee,
        const std::vector<int>& actuals,
        int retrieveResult,
        int memoryReturnDest,
        int siteId,
        IrStringTable& strings) {
    const auto privateLabels = privateLabelIds(callee);
    std::unordered_map<int, int> remap;

    std::vector<Instruction> out;
    for (std::size_t k = 0; k < callee.frame.arguments.size(); ++k) {
        const Value& formal = callee.frame.arguments[k];
        const int actual = k < actuals.size() ? actuals[k] : kNoSymbol;
        if (actual != kNoSymbol && formalCanShareActual(callee, formal.id(), actual, caller)) {
            remap[formal.id()] = actual;
            continue;
        }
        const int cloned = remapPrivateValue(strings, siteId, formal.id());
        const bool asTemp = !addressTakenIn(callee.body, formal.id());
        pushClone(caller, formal, cloned, asTemp);
        remap[formal.id()] = cloned;
        if (actual != kNoSymbol) {
            out.push_back(ir::assign(actual, cloned));
        }
    }

    if (callee.memoryReturn && callee.sretId != kNoSymbol) {
        const int dest = memoryReturnDest != kNoSymbol ? memoryReturnDest : retrieveResult;
        if (dest != kNoSymbol) {
            remap[callee.sretId] = dest;
        }
    }

    for (const auto& value : callee.frame.locals) {
        if (remap.count(value.id()) != 0) {
            continue;
        }
        const int cloned = remapPrivateValue(strings, siteId, value.id());
        pushClone(caller, value, cloned, value.isExpressionTemp());
        remap[value.id()] = cloned;
    }
    if (callee.sretId != kNoSymbol && remap.count(callee.sretId) == 0) {
        const int cloned = remapPrivateValue(strings, siteId, callee.sretId);
        if (const Value* sret = findValue(callee, callee.sretId)) {
            pushClone(caller, *sret, cloned, sret->isExpressionTemp());
        }
        remap[callee.sretId] = cloned;
    }

    for (int label : privateLabels) {
        remap[label] = internFresh(strings, "__L");
    }
    const int cont = internFresh(strings, "__L");

    for (const auto& inst : callee.body) {
        if (inst.op == Op::Return || inst.op == Op::VoidReturn) {
            if (retrieveResult != kNoSymbol && inst.op == Op::Return && inst.arg0 != kNoSymbol) {
                int src = inst.arg0;
                const auto it = remap.find(src);
                if (it != remap.end()) {
                    src = it->second;
                }
                out.push_back(ir::assign(src, retrieveResult));
            }
            out.push_back(ir::jump(cont));
            continue;
        }
        Instruction cloned = inst;
        rewriteInstructionIds(cloned, remap);
        out.push_back(cloned);
    }
    out.push_back(ir::label(cont));
    return out;
}

} // namespace codegen
