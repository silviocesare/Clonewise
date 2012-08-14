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
		fprintf(stderr, "Couldn't open %s\n", filename);
		exit(1);
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
					if (strncmp(&s[i], " <unfixed>", 10) == 0) {
						if (strncmp(&s[i + 10], " (static", 8) == 0) {
							embeddedsState[lib][package] = EMBED_UNFIXED_STATIC;
						} else {
							embeddedsState[lib][package] = EMBED_UNFIXED;
						}
					} else if (strncmp(&s[i], " <unfixable>", 12) == 0) {
						if (strncmp(&s[i + 12], " (static", 8) == 0) {
						
							embeddedsState[lib][package] = EMBED_UNFIXED_STATIC;
						} else {
							embeddedsState[lib][package] = EMBED_UNFIXED;
						}
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
LoadTheCache(std::map<std::string, std::map<std::string, std::list<Match> > > &thecache, const std::string &prefix)
{
	std::map<std::string, std::set<std::string> >::const_iterator eIter;

	for (	eIter  = embeddeds.begin();
		eIter != embeddeds.end();
		eIter++)
	{
		std::string lib;
		std::ifstream stream;
		char s[1024];

		snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/%s/%s", distroString, prefix.c_str(), eIter->first.c_str());
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
				thecache[eIter->first][lib] = std::list<Match>();
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
					thecache[eIter->first][lib].push_back(m);
				}
			}
		}
		stream.close();
	}
}

void
LoadCache()
{
	std::map<std::string, std::map<std::string, std::list<Match> > >::iterator cacheIter, cNext;
	std::map<std::string, std::list<Match> >::iterator mIter, next;

	LoadTheCache(cache, "cache");
	for (	cacheIter  = cache.begin();
		cacheIter != cache.end();
		)
	{
		cNext = cacheIter;
		cNext++;

		for (	mIter  = cacheIter->second.begin();
			mIter != cacheIter->second.end();
			)
		{
			next = mIter;
			next++;

			if (mIter->second.size() == 0) {
				cacheIter->second.erase(mIter);
			} else
				mIter->second = cache[cacheIter->first][mIter->first];
			mIter = next;
		}

		cacheIter = cNext;
	}
}

void
LoadEmbeddedCache()
{
	std::map<std::string, std::map<std::string, std::list<Match> > > sharedcache;
	std::map<std::string, std::map<std::string, std::list<Match> > >::iterator cacheIter, cNext;
	std::map<std::string, std::list<Match> >::iterator mIter, next;

	LoadTheCache(sharedcache, "cache");
	LoadTheCache(cache, "cache-embedded");

	for (	cacheIter  = cache.begin();
		cacheIter != cache.end();
		)
	{
		cNext = cacheIter;
		cNext++;

		for (	mIter  = cacheIter->second.begin();
			mIter != cacheIter->second.end();
			)
		{
			next = mIter;
			next++;

			if (sharedcache[cacheIter->first][mIter->first].size() == 0) {
				cacheIter->second.erase(mIter);
			} else
				mIter->second = sharedcache[cacheIter->first][mIter->first];
			mIter = next;
		}

		cacheIter = cNext;
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
ShowMissingLibs(const std::string &embeddedLib, const std::string &msg, bool useMatchFilename, const std::set<std::string> &matchFilename, const std::set<std::string> &exclude, const std::list<std::string> &functions, std::set<std::string> &packages, bool print)
{
	std::map<std::string, std::list<Match> >::const_iterator cIter;
	std::set<std::string>::const_iterator eIter;
	bool any1 = false, any2 = false;
	std::string m;
	bool loadedSig;
	ClonewiseSignature eSig;
	int packagesSize = packages.size();

	if (embeddeds.find(embeddedLib) == embeddeds.end()) {
		return;
	}

	loadedSig = false;
	if (showUnfixed) {
		if (print && outputFormat == CLONEWISE_OUTPUT_XML) {
			printf("\t<TrackedClones>\n");
		}
		for (	eIter  = embeddeds[embeddedLib].begin();
			eIter != embeddeds[embeddedLib].end();
			eIter++)
		{
			if (exclude.find(*eIter) == exclude.end() && embeddedsState[embeddedLib][*eIter] == EMBED_UNFIXED) {
				std::string filename;
				ClonewiseSignature sig;
				std::map<std::string, std::set<std::string> >::const_iterator sigIter;

				filename = std::string("/var/lib/Clonewise/clones/distros/") + distroString + std::string("/signatures/") + eIter->c_str();
				LoadSignature(*eIter, filename, sig);
				if (!loadedSig) {
					filename = std::string("/var/lib/Clonewise/clones/distros/") + distroString + std::string("/signatures/") + embeddedLib;
					LoadSignature(embeddedLib, filename, eSig);
					loadedSig = true;
				}

				for (	sigIter  = sig.filesAndHashes.begin();
					sigIter != sig.filesAndHashes.end();
					sigIter++)
				{
					if ((matchFilename.size() == 0 || matchFilename.find(sigIter->first) != matchFilename.end()) && (packagesSize == 0 || packages.find(*eIter) != packages.end())) {
						if ((matchFilename.size() == 0 || matchHash(sig.filesAndHashes, eSig.filesAndHashes, matchFilename)) && (functions.size() == 0 || matchFunctions(*eIter, functions))) {
							if (any1 == false && pretty && print) {
								if (outputFormat != CLONEWISE_OUTPUT_XML) {
									printf("# The following package clones are tracked in the embedded-code-copies\n# database. They have not been fixed.\n");
									printf("#\n\n");
								}
								any1 = true;
							}
							if (print) {
								if (outputFormat == CLONEWISE_OUTPUT_XML) {
									printf("\t\t<Clone>\n");
									printf("\t\t\t<EmbeddedLibrary>%s</EmbeddedLibrary>\n", embeddedLib.c_str());
									printf("\t\t\t<Package>%s</Package>\n", eIter->c_str());
									printf("\t\t\t<State>Unfixed</State>\n");
									if (msg.size() != 0)
										printf("\t\t\t<CVE>%s</CVE>\n", msg.c_str());
									printf("\t\t</Clone>\n");
								} else {

									printf("%s CLONED_IN_SOURCE %s <unfixed> %s\n", embeddedLib.c_str(), eIter->c_str(), msg.c_str());
								}
							}
							if (packagesSize == 0)
								packages.insert(*eIter);
							break;
						}
					}
				}
			}
		}
		if (any1 && print) {
			if (outputFormat == CLONEWISE_OUTPUT_XML) {
				printf("\t</TrackedClones>\n");
			} else {
				printf("\n\n");
			}
		}
	}
	if (cache.find(embeddedLib) == cache.end()) {
		return;
	}
	if (print && outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("\t<UntrackedClones>\n");
	}
	for (	cIter  = cache[embeddedLib].begin();
		cIter != cache[embeddedLib].end();
		cIter++)
	{
		if (exclude.find(cIter->first) != exclude.end())
			continue;

		if (strcmp(embeddedLib.c_str(), cIter->first.c_str()) == 0)
			continue;

		if (embeddeds[embeddedLib].find(cIter->first.c_str()) == embeddeds[embeddedLib].end() && (packagesSize == 0 || packages.find(cIter->first) != packages.end())) {
			std::list<Match>::const_iterator mIter;
			char cmd[1024];
			int status;
			std::string filename;
			ClonewiseSignature sig;

			filename = std::string("/var/lib/Clonewise/clones/distros/") + distroString + std::string("/signatures/") + cIter->first.c_str();
			LoadSignature(cIter->first, filename, sig);
			if (!loadedSig) {
				filename = std::string("/var/lib/Clonewise/clones/distros/") + distroString + std::string("/signatures/") + embeddedLib;
				LoadSignature(embeddedLib, filename, eSig);
				loadedSig = true;
			}
			if (useMatchFilename) {
				for (	mIter  = cIter->second.begin();
					mIter != cIter->second.end();
					mIter++)
				{
					if ((matchFilename.find(mIter->filename1) != matchFilename.end() || matchFilename.find(mIter->filename2) != matchFilename.end()) && matchHash(sig.filesAndHashes, eSig.filesAndHashes, matchFilename) && (functions.size() == 0 || matchFunctions(cIter->first, functions))) {
						goto gotit;
					}
				}
				continue;
			}
gotit:
			if (any2 == false && pretty && print) {
				if (outputFormat != CLONEWISE_OUTPUT_XML) {
					printf("# The following package clones are NOT tracked in the embedded-code-copies\n# database.\n");
					printf("#\n\n");
				}
				any2 = true;
			}
			snprintf(cmd, sizeof(cmd), "Clonewise-CheckDepends %s %s %s> /dev/null 2> /dev/null", distroString, embeddedLib.c_str(), cIter->first.c_str());
			status = system(cmd);
			if (print) {
				if (outputFormat == CLONEWISE_OUTPUT_XML) {
					printf("\t\t<Clone>\n");
					printf("\t\t\t<EmbeddedLibrary>%s</EmbeddedLibrary>\n", embeddedLib.c_str());
					printf("\t\t\t<Package>%s</Package>\n", cIter->first.c_str());
					if (msg.size() != 0)
						printf("\t\t\t<CVE>%s</CVE>\n", msg.c_str());
				} else {
					printf("%s CLONED_IN_SOURCE %s ", embeddedLib.c_str(), cIter->first.c_str());
				}
			}
			if (WEXITSTATUS(status) == 0) {
				if (print) {
					if (outputFormat == CLONEWISE_OUTPUT_XML) {
						printf("\t\t\t<State>Fixed</State>\n");
						if (msg.size() != 0)
							printf("\t\t\t<CVE>%s</CVE>\n", msg.c_str());
					} else {
						printf("<fixed> %s\n",  msg.c_str());
					}
				}
			} else {
				if (print) {
					if (outputFormat == CLONEWISE_OUTPUT_XML) {
						printf("\t\t\t<State>Unfixed</State>\n");
						if (msg.size() != 0)
							printf("\t\t\t<CVE>%s</CVE>\n", msg.c_str());
					} else {
						printf("<unfixed> %s\n", msg.c_str());
					}
				}
				if (packagesSize == 0)
					packages.insert(cIter->first);
			}
			if (print) {
				if (outputFormat == CLONEWISE_OUTPUT_XML) {
					printf("\t\t\t<Matches>\n");
				}
				for (	mIter  = cIter->second.begin();
					mIter != cIter->second.end();
					mIter++)
				{
					if (outputFormat == CLONEWISE_OUTPUT_XML) {
						printf("\t\t\t\t<Match><Filename1>%s</Filename1><Filename2>%s</Filename2><Weight>%s</Weight></Match>\n", mIter->filename1.c_str(), mIter->filename2.c_str(), mIter->weight.c_str());
					} else {
						printf("\t\tMATCH %s/%s %s\n", mIter->filename1.c_str(), mIter->filename2.c_str(), mIter->weight.c_str());
					}
				}
				if (outputFormat == CLONEWISE_OUTPUT_XML) {
					printf("\t\t\t</Matches>\n");
					printf("\t\t</Clone>\n");
				}
			}
		}
	}
	if (print && outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("\t<UntrackedClones>\n");
	}
}

bool
HasMissingLibs(const std::string &embeddedLib, const std::string &msg, bool useMatchFilename, const std::set<std::string> &matchFilename, const std::set<std::string> &exclude, const std::list<std::string> &functions)
{
	std::set<std::string> packages;

	ShowMissingLibs(embeddedLib, msg, useMatchFilename, matchFilename, exclude, functions, packages, false);
	return packages.size() != 0;
}

void
ShowMissingLibs(const std::string &embeddedLib, const std::string &msg, bool useMatchFilename, const std::set<std::string> &matchFilename, const std::set<std::string> &exclude, const std::list<std::string> &functions, std::set<std::string> &packages)
{
	return ShowMissingLibs(embeddedLib, msg, useMatchFilename, matchFilename, exclude, functions, packages, true);
}

void
ShowMissing()
{
	std::map<std::string, std::set<std::string> >::const_iterator eIter;

	for (	eIter  = embeddeds.begin();
		eIter != embeddeds.end();
		eIter++)
	{
		std::set<std::string> vulnSources, exclude, packages;
		std::list<std::string> functions;	

		ShowMissingLibs(eIter->first, "", false, vulnSources, exclude, functions, packages);
	}
}
