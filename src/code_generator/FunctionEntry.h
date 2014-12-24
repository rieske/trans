#ifndef FUNCTIONENTRY_H_
#define FUNCTIONENTRY_H_

#include <cstddef>
#include <string>
#include <vector>

#include "../ast/types/Function.h"
#include "../ast/types/Type.h"

namespace code_generator {

class FunctionEntry {
public:
    FunctionEntry(std::string name, ast::Function type, unsigned context);
    virtual ~FunctionEntry();

    const std::string& getName() const;
    const ast::Function& getType() const;
    unsigned getContext() const;

    std::size_t argumentCount() const;
    const std::vector<ast::Type>& argumentTypes() const;
    const ast::Type& returnType() const;

private:
    std::string name;
    ast::Function type;
    unsigned context;
};

} /* namespace code_generator */

#endif /* FUNCTIONENTRY_H_ */
