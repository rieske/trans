#ifndef TRANSLATIONUNITCONTEXT_H_
#define TRANSLATIONUNITCONTEXT_H_

#include <cstddef>
#include <string>

namespace translation_unit {

class Context {
public:
    Context(const std::string& sourceName, std::size_t offset);
    virtual ~Context();

    size_t getOffset() const;
    const std::string& getSourceName() const;

private:
    std::string sourceName;
    std::size_t offset;
};

}

bool operator==(const translation_unit::Context& lhs, const translation_unit::Context& rhs);
bool operator!=(const translation_unit::Context& lhs, const translation_unit::Context& rhs);

std::string to_string(const translation_unit::Context& context);
std::ostream& operator<<(std::ostream& ostream, const translation_unit::Context& context);

#endif /* TRANSLATIONUNITCONTEXT_H_ */
