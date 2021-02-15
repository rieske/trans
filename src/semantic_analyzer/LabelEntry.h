#ifndef LABELENTRY_H_
#define LABELENTRY_H_

#include <string>

namespace semantic_analyzer {

class LabelEntry {
public:
    LabelEntry(std::string name);

    std::string getName() const;

private:
    std::string name;
};

} /* namespace semantic_analyzer */

#endif /* LABELENTRY_H_ */
