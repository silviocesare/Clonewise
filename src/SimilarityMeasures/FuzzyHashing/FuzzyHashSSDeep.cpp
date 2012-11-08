#include "FuzzyHashSSDeep.h"
#include <fuzzy.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "Clonewise.h"

FuzzyHashSignature*
FuzzyHashSSDeep::NewSignature()
{
	return new FuzzyHashSignatureSSDeep;
}

int
FuzzyHashSSDeep::HashFile(const std::string &filename, FuzzyHashSignature *signature)
{
	FuzzyHashSignatureSSDeep *sig = static_cast<FuzzyHashSignatureSSDeep *>(signature);
        char line[1024], cmd[1024];
        FILE *p;

        snprintf(cmd, sizeof(cmd), "ssdeep %s|tail -n1", filename.c_str());
        p = popen(cmd, "r");
        fgets(line, sizeof(line), p);
        pclose(p);

	lineToHash(line, sig->m_Hash);

	return 0;
}

int
FuzzyHashSSDeep::Hash(unsigned char *data, unsigned int len, FuzzyHashSignature *signature)
{
	return -1;
}

int
FuzzyHashSSDeep::HashCompare(const FuzzyHashSignature *x, const FuzzyHashSignature *y)
{
	const FuzzyHashSignatureSSDeep *x_sig = static_cast<const FuzzyHashSignatureSSDeep *>(x);
	const FuzzyHashSignatureSSDeep *y_sig = static_cast<const FuzzyHashSignatureSSDeep *>(y);

	return fuzzy_compare(x_sig->m_Hash.c_str(), y_sig->m_Hash.c_str());
}
