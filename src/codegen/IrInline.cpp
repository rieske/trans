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
        if (isVaOp(inst.op) || inst.op == Op::Alloca) {
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
        return strings.internFresh("$t");
    }
    if (name.compare(0, 3, "__t") == 0) {
        return strings.internFresh("__t");
    }
    return internInlinedAutomatic(strings, siteId, sourceId);
}

void pushClone(Procedure& caller, const Value& src, int newId, bool asExprTemp) {
    Value clone { newId, 0, src.getType(), src.getSizeInBytes(), src.getClassification() };
    if (asExprTemp || src.isExpressionTemp()) {
        clone.markExpressionTemp();
    }
    if (src.isVolatile()) {
        clone.markVolatile();
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
    bool calleeHasLabel = false;
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
        if (inst.op == Op::Label) {
            calleeHasLabel = true;
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
    const Value* formal = findValue(callee, formalId);
    const Value* actual = findValue(caller, actualId);
    if (formal && formal->isVolatile() && !(actual && actual->isVolatile())) {
        return false;
    }
    if (actual && actual->isExpressionTemp() && calleeHasLabel) {
        return false;
    }
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
        pushClone(caller, formal, cloned, false);
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
        remap[label] = strings.internFresh("__L");
    }
    const int cont = strings.internFresh("__L");

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

namespace {

int countNonLabels(const std::vector<Instruction>& body, int begin, int end) {
    int n = 0;
    for (int i = begin; i < end; ++i) {
        if (body[static_cast<std::size_t>(i)].op != Op::Label) {
            ++n;
        }
    }
    return n;
}

void visitPostOrder(int i,
        const IntermediateRepresentation& ir,
        const std::unordered_map<int, int>& index,
        std::vector<char>& color,
        std::vector<int>& order) {
    color[static_cast<std::size_t>(i)] = 1;
    for (const auto& inst : ir.procedures[static_cast<std::size_t>(i)].body) {
        if (inst.op != Op::Call || inst.callIndirect) {
            continue;
        }
        const auto it = index.find(inst.arg0);
        if (it != index.end() && color[static_cast<std::size_t>(it->second)] == 0) {
            visitPostOrder(it->second, ir, index, color, order);
        }
    }
    color[static_cast<std::size_t>(i)] = 2;
    order.push_back(i);
}

bool calleeIsEmpty(const Procedure& callee) {
    for (const auto& inst : callee.body) {
        if (inst.op == Op::Label) {
            continue;
        }
        if (inst.op == Op::VoidReturn || (inst.op == Op::Return && inst.arg0 == kNoSymbol)) {
            continue;
        }
        return false;
    }
    return true;
}

bool hasIntraTuCall(const IntermediateRepresentation& ir,
        const std::unordered_map<int, int>& index) {
    for (const auto& procedure : ir.procedures) {
        for (const auto& inst : procedure.body) {
            if (inst.op == Op::Call && !inst.callIndirect && index.count(inst.arg0) != 0) {
                return true;
            }
        }
    }
    return false;
}

void inlineInto(Procedure& caller,
        IntermediateRepresentation& ir,
        const std::unordered_map<int, int>& index,
        const std::unordered_set<int>& finished,
        const InlineCaps& caps,
        int& inlinesInTu,
        int& siteId,
        int callerIndex,
        InlineStats& stats) {
    struct Site {
        int begin;
        int end;
        int call;
    };
    std::vector<Site> sites;
    for (int i = 0; i < static_cast<int>(caller.body.size()); ++i) {
        if (caller.body[static_cast<std::size_t>(i)].op != Op::Call) {
            continue;
        }
        int begin = i;
        while (begin > 0 && caller.body[static_cast<std::size_t>(begin) - 1].op == Op::Argument) {
            --begin;
        }
        int end = i + 1;
        if (end < static_cast<int>(caller.body.size())
                && caller.body[static_cast<std::size_t>(end)].op == Op::Retrieve) {
            ++end;
        }
        sites.push_back({ begin, end, i });
    }
    if (sites.empty()) {
        return;
    }
    bool anyTuCall = false;
    for (const Site& site : sites) {
        const Instruction& call = caller.body[static_cast<std::size_t>(site.call)];
        if (!call.callIndirect && index.count(call.arg0) != 0) {
            anyTuCall = true;
            break;
        }
    }
    if (!anyTuCall) {
        stats.sitesConsidered += static_cast<int>(sites.size());
        return;
    }

    int inlinesOnCaller = 0;
    int callerNonLabels = nonLabelCount(caller.body);
    std::vector<Instruction> out;
    int cursor = 0;
    for (const Site& site : sites) {
        out.insert(out.end(), caller.body.begin() + cursor, caller.body.begin() + site.begin);
        std::vector<int> actuals;
        for (int k = site.begin; k < site.call; ++k) {
            actuals.push_back(caller.body[static_cast<std::size_t>(k)].arg0);
        }
        const Instruction& call = caller.body[static_cast<std::size_t>(site.call)];
        int retrieveResult = kNoSymbol;
        if (site.end == site.call + 2) {
            retrieveResult = caller.body[static_cast<std::size_t>(site.call) + 1].result;
        }
        ++stats.sitesConsidered;

        bool inlined = false;
        const auto it = index.find(call.arg0);
        if (it != index.end() && !call.callIndirect) {
            Procedure& callee = ir.procedures[static_cast<std::size_t>(it->second)];
            const bool calleeFinished = finished.count(callee.name) != 0;
            const int calleeNonLabels = nonLabelCount(callee.body);
            bool skipEmpty = calleeIsEmpty(callee);
            if (!skipEmpty) {
                for (const auto& inst : callee.body) {
                    if (inst.op != Op::Call || inst.callIndirect) {
                        continue;
                    }
                    const auto inner = index.find(inst.arg0);
                    if (inner != index.end()
                            && calleeIsEmpty(ir.procedures[static_cast<std::size_t>(inner->second)])) {
                        skipEmpty = true;
                        break;
                    }
                }
            }
            if (!skipEmpty && callIsEligible(call, caller, callee, ir.strings, caps,
                    static_cast<int>(actuals.size()), inlinesOnCaller, inlinesInTu, calleeFinished)
                    && callerNonLabels + calleeNonLabels <= caps.maxCallerInsts) {
                auto cloned = cloneCalleeBody(caller, callee, actuals, retrieveResult,
                        call.memoryReturnDest, siteId, ir.strings);
                const int removed = countNonLabels(caller.body, site.begin, site.end);
                callerNonLabels += nonLabelCount(cloned) - removed;
                out.insert(out.end(), cloned.begin(), cloned.end());
                ++siteId;
                ++inlinesOnCaller;
                ++inlinesInTu;
                ++stats.sitesInlined;
                inlined = true;
            } else if (!calleeFinished || callee.name == caller.name) {
                ++stats.refusedRecursion;
            } else if (calleeNonLabels > caps.maxCalleeInsts
                    || callerNonLabels + calleeNonLabels > caps.maxCallerInsts
                    || inlinesOnCaller >= caps.maxInlinesPerCaller
                    || inlinesInTu >= caps.maxTotalInlines) {
                ++stats.refusedSize;
            } else {
                ++stats.refusedOther;
            }
        }
        if (!inlined) {
            out.insert(out.end(), caller.body.begin() + site.begin, caller.body.begin() + site.end);
        }
        cursor = site.end;
    }
    out.insert(out.end(), caller.body.begin() + cursor, caller.body.end());
    if (inlinesOnCaller != 0) {
        caller.body = std::move(out);
        stats.dirtyCallers.push_back(callerIndex);
    }
}

} // namespace

InlineStats inlineProcedures(IntermediateRepresentation& ir, InlineCaps caps) {
    InlineStats stats;
    const int n = static_cast<int>(ir.procedures.size());
    std::unordered_map<int, int> index;
    for (int i = 0; i < n; ++i) {
        index[ir.procedures[static_cast<std::size_t>(i)].name] = i;
    }
    if (!hasIntraTuCall(ir, index)) {
        return stats;
    }
    std::vector<char> color(static_cast<std::size_t>(n), 0);
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        if (color[static_cast<std::size_t>(i)] == 0) {
            visitPostOrder(i, ir, index, color, order);
        }
    }

    std::unordered_set<int> finished;
    int inlinesInTu = 0;
    int siteId = 0;
    for (int i : order) {
        inlineInto(ir.procedures[static_cast<std::size_t>(i)], ir, index, finished, caps,
                inlinesInTu, siteId, i, stats);
        finished.insert(ir.procedures[static_cast<std::size_t>(i)].name);
    }
    return stats;
}

} // namespace codegen
