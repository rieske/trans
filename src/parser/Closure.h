#ifndef CLOSURE_H_
#define CLOSURE_H_

#include <vector>

#include "FirstTable.h"
#include "LR1Item.h"

class Closure {
public:
	Closure(const FirstTable& first);
	virtual ~Closure();

	void operator()(std::vector<LR1Item>& items) const;

private:
	const FirstTable first;
};

#endif /* CLOSURE_H_ */
