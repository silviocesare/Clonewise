#ifndef Clonewise_Cache_h
#define Clonewise_Cache_h

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <map>
#include <set>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include "Clonewise.h"

enum EmbedType {
        EMBED_FIXED,
        EMBED_UNFIXED
};

struct Match {
        std::string filename1;
        std::string filename2;
        std::string weight;
};

extern std::map<std::string, std::map<std::string, EmbedType> > embeddedsState;
extern std::map<std::string, std::set<std::string> > embeddeds;
extern std::map<std::string, std::map<std::string, std::list<Match> > > cache;
extern bool pretty;
extern bool showUnfixed;

void LoadEmbeds(const char *filename);
void LoadCache();
void ShowMissingLibs(const std::string &embeddedLib, const std::string &msg, bool useMatchFilename, const std::set<std::string> &matchFilename, const std::set<std::string> &exclude, const std::list<std::string> &functions);
void ShowMissing();

#endif
