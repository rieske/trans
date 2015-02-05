#ifndef FUNCTIONENTRY_H_
#define FUNCTIONENTRY_H_

#include <cstddef>
#include <string>
#include <vector>

#include "../ast/types/Function.h"
#include "../ast/types/Type.h"
#include "translation_unit/Context.h"

namespace code_generator {

class FunctionEntry {
public:
    FunctionEntry(std::string name, ast::Function type, translation_unit::Context context);
    virtual ~FunctionEntry();

    const std::string& getName() const;
    const ast::Function& getType() const;
    translation_unit::Context getContext() const;

    std::size_t argumentCount() const;
    const std::vector<std::pair<std::string, ast::Type>>& arguments() const;
    const ast::Type& returnType() const;

private:
    std::string name;
    ast::Function type;
    translation_unit::Context context;
};

} /* namespace code_generator */

#endif /* FUNCTIONENTRY_H_ */
