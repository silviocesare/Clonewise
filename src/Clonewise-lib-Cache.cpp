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
#include <fuzzy.h>
#include "Clonewise.h"
#include "Clonewise-lib-Cache.h"

static void
Usage(const char *argv0)
{
}

std::map<std::string, std::map<std::string, EmbedType> > embeddedsState;
std::map<std::string, std::set<std::string> > embeddeds;
std::map<std::string, std::map<std::string, std::list<Match> > > cache;
bool pretty = false;
bool showUnfixed = false;

void
LoadEmbeds(const char *filename)
{
	std::ifstream stream;
	std::string lib, package;
	bool start = false;
	stream.open(filename);
	if (!stream) {
	}
	while (!stream.eof()) {
		char s[1024];

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			continue;
		if (start) { 
			int i;

			if (!isspace(s[0])) {
				char str[1024];
				int j;

				str[0] = 0;
				for (i = 0; isspace(s[i]); i++);
				for (j = 0; !isspace(s[i]); i++, j++)
					str[j] = s[i];
				str[j] = 0;
				lib = str;
				embeddeds[lib] = std::set<std::string>();
			} else {
				for (i = 0; isspace(s[i]); i++);
				if (s[i] == '-' && s[i + 1] == ' ') {
					int j;
					char str[1024];

					i += 2;
					for (j = 0; !isspace(s[i]); i++, j++)
						str[j] = s[i];
					str[j] = 0;
					package = str;
					embeddeds[lib].insert(package);
					if (strncmp(&s[i], " <unfixed>", 10) == 0 || strncmp(&s[i], " <unfixable>", 12) == 0) {
						embeddedsState[lib][package] = EMBED_UNFIXED;
					} else {
						embeddedsState[lib][package] = EMBED_FIXED;
					}
				}
			} 
		} else {
			if (strcmp(s, "---BEGIN") == 0) {
				start = true;
			}
		}
	}
	stream.close();
}

void
LoadCache()
{
	std::map<std::string, std::set<std::string> >::const_iterator eIter;

	for (	eIter  = embeddeds.begin();
		eIter != embeddeds.end();
		eIter++)
	{
		std::string lib;
		std::ifstream stream;
		char s[1024];

		snprintf(s, sizeof(s), "/var/lib/Clonewise/distros/%s/cache/%s", "ubuntu", eIter->first.c_str());
		stream.open(s);
		if (!stream)
			continue;
		while (!stream.eof()) {
			stream.getline(s, sizeof(s));
			if (s[0] == 0)
				continue;
			if (!isspace(s[0])) {
				int i;
				int j;
				char str[1024];

				for (i = 0; !isspace(s[i]); i++);
				i++;
				i += sizeof("CLONED_IN_SOURCE") - 1;
				i++;
				for (j = 0; !isspace(s[i]); i++, j++)
					str[j] = s[i];
				str[j] = 0;
				lib = str;
			} else {
				if (strncmp(s, "\t\tMATCH", 7) == 0) {
					std::string s1 = &s[8], s2;
					Match m;
					size_t n1, n2;

					n1 = s1.find_first_of('/');
					m.filename1 = s1.substr(0, n1);
					s2 = s1.substr(n1 + 1);
					n2 = s2.find_first_of(' ');
					m.filename2 = s2.substr(0, n2);
					m.weight = s2.substr(n2 + 1);
					cache[eIter->first][lib].push_back(m);
				}
			}
		}
		stream.close();
	}
}

bool
matchHash(std::map<std::string, std::set<std::string> > &sig1, std::map<std::string, std::set<std::string> > &sig2, const std::set<std::string> &matchFilename)
{
	std::set<std::string>::const_iterator mIter;

	for (	mIter  = matchFilename.begin();
		mIter != matchFilename.end();
		mIter++)
	{
		if (sig1.find(*mIter) != sig1.end() && sig2.find(*mIter) != sig2.end()) {
			std::set<std::string>::const_iterator sIter1, sIter2;

			for (	sIter1  = sig1[*mIter].begin();
				sIter1 != sig1[*mIter].end();
				sIter1++)
			{
				for (	sIter2  = sig2[*mIter].begin();
					sIter2 != sig2[*mIter].end();
					sIter2++)
				{
					float s;

					s = fuzzy_compare(sIter1->c_str(), sIter2->c_str());
					if (s > 0) {
						return true;
                                        }

				}
			}
		}
	}
	return false;
}

bool
matchFunctions(const std::string &packageName, const std::list<std::string> &functions)
{
	// Not implemented. Requires source of packages.
	return true;
}

void
ShowMissingLibs(const std::string &embeddedLib, const std::string &msg, bool useMatchFilename, const std::set<std::string> &matchFilename, const std::set<std::string> &exclude, const std::list<std::string> &functions)
{
	std::map<std::string, std::list<Match> >::const_iterator cIter;
	std::set<std::string>::const_iterator eIter;
	bool any1 = false, any2 = false;
	std::string m;
	bool loadedSig;
	std::map<std::string, std::set<std::string> > eSig;

	if (embeddeds.find(embeddedLib) == embeddeds.end()) {
		return;
	}

	loadedSig = false;
	if (showUnfixed) {
		for (	eIter  = embeddeds[embeddedLib].begin();
			eIter != embeddeds[embeddedLib].end();
			eIter++)
		{
			if (exclude.find(*eIter) == exclude.end() && embeddedsState[embeddedLib][*eIter] == EMBED_UNFIXED) {
				std::string filename;
				std::map<std::string, std::set<std::string> > sig;
				std::map<std::string, std::set<std::string> >::const_iterator sigIter;

				filename = std::string("/var/lib/Clonewise/distros/") + distroString + std::string("/signatures/") + eIter->c_str();
				LoadSignature(filename, sig);
				if (!loadedSig) {
					filename = std::string("/var/lib/Clonewise/distros/") + distroString + std::string("/signatures/") + embeddedLib;
					LoadSignature(filename, eSig);
					loadedSig = true;
				}

				for (	sigIter  = sig.begin();
					sigIter != sig.end();
					sigIter++)
				{
					if (matchFilename.find(sigIter->first) != matchFilename.end()) {
						if ((matchFilename.size() == 0 || matchHash(sig, eSig, matchFilename)) && (functions.size() == 0 || matchFunctions(*eIter, functions))) {
							if (any1 == false && pretty) {
								printf("# The following package clones are tracked in the embedded-code-copies\n# database. They have not been fixed.\n");
								printf("#\n\n");
								any1 = true;
							}
							printf("%s CLONED_IN_SOURCE %s <unfixed> %s\n", embeddedLib.c_str(), eIter->c_str(), msg.c_str());
							break;
						}
					}
				}
			}
		}
		if (any1) {
			printf("\n\n");
		}
	}
	if (cache.find(embeddedLib) == cache.end()) {
		return;
	}
	for (	cIter  = cache[embeddedLib].begin();
		cIter != cache[embeddedLib].end();
		cIter++)
	{
		if (exclude.find(cIter->first) != exclude.end())
			continue;

		if (strcmp(embeddedLib.c_str(), cIter->first.c_str()) == 0)
			continue;

		if (embeddeds[embeddedLib].find(cIter->first.c_str()) == embeddeds[embeddedLib].end()) {
			std::list<Match>::const_iterator mIter;
			char cmd[1024];
			int status;
			std::string filename;
			std::map<std::string, std::set<std::string> > sig;

			filename = std::string("/var/lib/Clonewise/distros/") + distroString + std::string("/signatures/") + cIter->first.c_str();
			LoadSignature(filename, sig);
			if (!loadedSig) {
				filename = std::string("/var/lib/Clonewise/distros/") + distroString + std::string("/signatures/") + embeddedLib;
				LoadSignature(filename, eSig);
				loadedSig = true;
			}
			if (useMatchFilename) {
				for (	mIter  = cIter->second.begin();
					mIter != cIter->second.end();
					mIter++)
				{
					if ((matchFilename.find(mIter->filename1) != matchFilename.end() || matchFilename.find(mIter->filename2) != matchFilename.end()) && matchHash(sig, eSig, matchFilename) && (functions.size() == 0 || matchFunctions(cIter->first, functions))) {
						goto gotit;
					}
				}
				continue;
			}
gotit:
			if (any2 == false && pretty) {
				printf("# The following package clones are NOT tracked in the embedded-code-copies\n# database.\n");
				printf("#\n\n");
				any2 = true;
			}
			snprintf(cmd, sizeof(cmd), "Clonewise-CheckDepends %s %s> /dev/null 2> /dev/null", embeddedLib.c_str(), cIter->first.c_str());
			status = system(cmd);
			if (WEXITSTATUS(status) == 0) {
				printf("%s CLONED_IN_SOURCE %s <fixed> %s\n", embeddedLib.c_str(), cIter->first.c_str(), msg.c_str());
			} else {
				printf("%s CLONED_IN_SOURCE %s <unfixed> %s\n", embeddedLib.c_str(), cIter->first.c_str(), msg.c_str());
			}
			for (	mIter  = cIter->second.begin();
				mIter != cIter->second.end();
				mIter++)
			{
				printf("\t\tMATCH %s/%s %s\n", mIter->filename1.c_str(), mIter->filename2.c_str(), mIter->weight.c_str());
			}
		}
	}
}

void
ShowMissing()
{
	std::map<std::string, std::set<std::string> >::const_iterator eIter;

	for (	eIter  = embeddeds.begin();
		eIter != embeddeds.end();
		eIter++)
	{
		std::set<std::string> vulnSources, exclude;
		std::list<std::string> functions;

		ShowMissingLibs(eIter->first, "", false, vulnSources, exclude, functions);
	}
}
