#ifndef CODEGEN_FRAMELAYOUT_H_
#define CODEGEN_FRAMELAYOUT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Value.h"
#include "quadruples/BasicBlock.h"
#include "semantic_analyzer/ValueEntry.h"

namespace codegen {

// Linear-scan stack slots: named locals keep distinct slots; expression temps
// reuse slots when live ranges do not overlap.
std::vector<Value> packFrameValues(
        const std::map<std::string, semantic_analyzer::ValueEntry>& locals,
        const std::vector<std::unique_ptr<BasicBlock>>& body);

} // namespace codegen

#endif // CODEGEN_FRAMELAYOUT_H_
