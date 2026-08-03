#include "PreamblePlan.h"

#include <set>

namespace codegen {

std::string bareSymbol(std::string name) {
    if (!name.empty() && name[0] == '$') {
        name = name.substr(1);
    }
    return name;
}

PreamblePlan buildPreamblePlan(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions) {
    PreamblePlan plan;
    std::set<std::string> externs {
        "scanf", "printf", "malloc", "free", "realloc",
        "__trans_va_set_areas", "__trans_va_pop_areas",
        "__trans_va_get_reg_save", "__trans_va_get_overflow"
    };
    for (const auto& name : externalFunctions) {
        externs.insert(bareSymbol(name));
    }
    for (const auto& global : globalVariables) {
        if (global.isExternal) {
            externs.insert(bareSymbol(global.name));
        }
    }
    plan.externs.assign(externs.begin(), externs.end());

    for (const auto& constant : constants) {
        PreambleDataItem item;
        item.name = bareSymbol(constant.first);
        item.exportGlobal = false; // pool labels are not .globl
        item.stringToken = constant.second;
        plan.data.push_back(std::move(item));
    }
    for (const auto& global : globalVariables) {
        if (global.isExternal) {
            continue;
        }
        PreambleDataItem item;
        item.name = bareSymbol(global.name);
        item.exportGlobal = !global.isStatic && !item.name.empty() && item.name[0] != '.';
        if (global.stringInitializer) {
            item.stringToken = *global.stringInitializer;
        } else {
            item.widthBytes = global.dataWidthBytes();
            for (const auto& word : global.dataOperands()) {
                item.dataOperands.push_back(bareSymbol(word));
            }
        }
        plan.data.push_back(std::move(item));
    }

    for (const auto& name : definedFunctions) {
        if (!name.empty() && name[0] == '.') {
            continue;
        }
        plan.textGlobals.push_back(bareSymbol(name));
    }
    if (plan.textGlobals.empty()) {
        plan.textGlobals.push_back("main");
    }
    return plan;
}

} // namespace codegen
