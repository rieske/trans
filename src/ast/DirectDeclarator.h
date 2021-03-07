#ifndef DIRECT_DECLARATOR_H_
#define DIRECT_DECLARATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "semantic_analyzer/ValueEntry.h"
#include "ast/Pointer.h"
#include "types/Type.h"

namespace ast {

class DirectDeclarator: public AbstractSyntaxTreeNode {
public:
    virtual ~DirectDeclarator() = default;

    std::string getName() const;

    translation_unit::Context getContext() const;

    void setHolder(semantic_analyzer::ValueEntry holder);
    semantic_analyzer::ValueEntry* getHolder() const;

    static const std::string ID;

    virtual type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) = 0;

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

    std::unique_ptr<semantic_analyzer::ValueEntry> holder { nullptr };
};

} // namespace ast

#endif // DIRECT_DECLARATOR_H_
