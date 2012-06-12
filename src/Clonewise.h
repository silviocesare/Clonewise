#ifndef Clonewise_h
#define Clonewise_h

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <map>
#include <set>
#include <list>
#include <math.h>
#include <cctype>
#include <unistd.h>
#include <cstdarg>
#include <string>

enum OutputFormat_e {
	CLONEWISE_OUTPUT_TEXT,
	CLONEWISE_OUTPUT_XML,
	CLONEWISE_OUTPUT_JSON,
	CLONEWISE_OUTPUT_YAML
};

struct Feature {
	const char *Name;
	bool Use;
};

#define NFEATURES 27

extern std::map<std::string, std::set<std::string> > embeddedList;
extern FILE *outFd;
extern std::set<std::string> featureExceptions;
extern const char *distroString;
extern OutputFormat_e outputFormat;
extern bool doCheckRelated;
extern bool useSSDeep;
extern bool useExtensions;
extern int verbose;
extern std::map<std::string, float> idf;
extern std::set<std::string> featureSet;
extern std::set<std::string> ignoreFalsePositives;
extern unsigned int numPackages;
extern bool allPackages;
extern std::map<std::string, std::list<std::string> > packages;
extern std::map<std::string, std::map<std::string, std::set<std::string> > > packagesSignatures;
extern std::set<std::string> extensions;
extern bool reportError;
extern bool useRelativePathForSignature;

void LoadPackagesInfo();
void LoadExtensions();
bool IsProgramFilename(const std::string &feature);
void normalizeFeature(std::string &normalFeature, const std::string &feature);
void LoadEmbeddedCodeCopiesList(const char *filename);
int LoadEverything();
int RunClonewise(int argc, char *argv[]);
bool WriteCheckForClone(std::ofstream &testStream, const std::map<std::string, std::set<std::string> > &embedding, const std::map<std::string, std::set<std::string> > &package, const std::string &cl);
void printMatch(const std::map<std::string, std::set<std::string> > &embedding, const std::map<std::string, std::set<std::string> > &package);

#endif
