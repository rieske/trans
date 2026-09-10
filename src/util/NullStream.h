#ifndef NULLSTREAM_H_
#define NULLSTREAM_H_

#include <iostream>

class NullBuffer: public std::streambuf {
public:
	int overflow(int c) override;
};

class NullStream: public std::ostream {
public:
    static NullStream& getInstance();

private:
	NullStream();
	NullBuffer nullBuffer;
};

#endif // NULLSTREAM_H_
