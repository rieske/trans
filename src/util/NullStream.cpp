#include "NullStream.h"

int NullBuffer::overflow(int c) {
    return c;
}

std::unique_ptr<NullStream> NullStream::instance;

NullStream::NullStream():
    std::ostream(&nullBuffer)
{}

NullStream& NullStream::getInstance() {
	if (!instance) {
		instance = std::unique_ptr<NullStream>(new NullStream());
	}
	return *instance;
}

