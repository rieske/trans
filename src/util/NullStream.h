#ifndef NULLSTREAM_H_
#define NULLSTREAM_H_

#include <iostream>

#include "NullBuffer.h"

class NullStream: public std::ostream {
public:
	NullStream() :
			std::ostream(&nullBuffer) {
	}
private:
	NullBuffer nullBuffer;
};

static NullStream nullStream;

#endif // NULLSTREAM_H_
