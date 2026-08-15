#ifndef TRANSLATIONUNITCONTEXT_H_
#define TRANSLATIONUNITCONTEXT_H_

#include <ostream>
#include <string>

namespace translation_unit {

class Context {
public:
    Context(std::string sourceName, std::size_t offset);

    std::size_t getOffset() const;
    std::string getSourceName() const;

private:
    std::string sourceName;
    std::size_t offset;
};

std::ostream& operator<<(std::ostream& ostream, const Context& context);

std::string to_string(const Context& context);

} // namespace translation_unit

#endif // TRANSLATIONUNITCONTEXT_H_
