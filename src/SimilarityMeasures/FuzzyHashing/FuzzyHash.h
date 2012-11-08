#ifndef FuzzyHash_h
#define FuzzyHash_h

#include <string>

class FuzzyHashSignature {
public:
	virtual ~FuzzyHashSignature() {}
};

class FuzzyHash {
public:
	virtual ~FuzzyHash() {}

	virtual FuzzyHashSignature *NewSignature() = 0;
	virtual int Hash(unsigned char *data, unsigned int len, FuzzyHashSignature *signature) = 0;
	virtual int HashFile(const std::string &filename, FuzzyHashSignature *signature) = 0;
	virtual int HashCompare(const FuzzyHashSignature *x, const FuzzyHashSignature *y) = 0;
};

#endif
