#ifndef NULLBUFFER_H_
#define NULLBUFFER_H_

#include <iostream>

class NullBuffer: public std::streambuf {
public:
	int overflow(int c) override {
		return c;
	}
};

#endif // NULLBUFFER_H_
