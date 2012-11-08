#ifndef FuzzyHashSSDeep_h
#define FuzzyHashSSDeep_h

#include "FuzzyHash.h"

class FuzzyHashSignatureSSDeep : public FuzzyHashSignature {
public:
	std::string m_Hash;
};

class FuzzyHashSSDeep : public FuzzyHash {
public:
	FuzzyHashSignature *NewSignature();
	int Hash(unsigned char *data, unsigned int len, FuzzyHashSignature *signature);
	int HashFile(const std::string &filename, FuzzyHashSignature *signature);
	int HashCompare(const FuzzyHashSignature *x, const FuzzyHashSignature *y);
};

#endif
