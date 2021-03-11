#ifndef CLOSURE_H_
#define CLOSURE_H_

#include <vector>

#include "FirstTable.h"
#include "LR1Item.h"

namespace parser {

class Closure {
public:
	Closure(const FirstTable& first, const Grammar* grammar);

	void operator()(std::vector<LR1Item>& items) const;

private:
	const FirstTable first;
	const Grammar* grammar;
};

} // namespace parser

#endif // CLOSURE_H_
