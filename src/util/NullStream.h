#ifndef NULLSTREAM_H_
#define NULLSTREAM_H_

#include <iostream>
#include <memory>

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

	static std::unique_ptr<NullStream> instance;
};

#endif // NULLSTREAM_H_
