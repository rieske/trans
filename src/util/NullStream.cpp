#include "NullStream.h"

int NullBuffer::overflow(int c) {
    return c;
}

NullStream::NullStream():
    std::ostream(&nullBuffer)
{}

NullStream& NullStream::getInstance() {
	static NullStream instance;
	return instance;
}

