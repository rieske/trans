#ifndef DIRECT_DECLARATOR_H_
#define DIRECT_DECLARATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "../semantic_analyzer/ValueEntry.h"
#include "../translation_unit/Context.h"
#include "Pointer.h"

namespace ast {
class FundamentalType;
class StoredType;
} /* namespace ast */

namespace ast {

class DirectDeclarator: public AbstractSyntaxTreeNode {
public:
    virtual ~DirectDeclarator() = default;

    std::string getName() const;

    translation_unit::Context getContext() const;

    void setHolder(semantic_analyzer::ValueEntry holder);
    semantic_analyzer::ValueEntry* getHolder() const;

    static const std::string ID;

    virtual std::unique_ptr<FundamentalType> getFundamentalType(std::vector<Pointer> indirection, const FundamentalType& baseType) = 0;

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

    std::unique_ptr<semantic_analyzer::ValueEntry> holder { nullptr };
};

}

#endif // DIRECT_DECLARATOR_H_
