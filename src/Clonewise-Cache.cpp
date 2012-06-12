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

void
Usage(const char *argv0)
{
}

enum EmbedType {
	EMBED_FIXED,
	EMBED_UNFIXED
};

std::map<std::string, std::map<std::string, EmbedType> > embeddedsState;
std::map<std::string, std::set<std::string> > embeddeds;
std::map<std::string, std::map<std::string, std::list<std::string> > > cache;
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
					if (strncmp(&s[i], " <unfixed>", 10) == 0) {
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
				if (cache.find(eIter->first) == cache.end()) {
					cache[eIter->first] = std::map<std::string, std::list<std::string> >();
				}
				if (cache[eIter->first].find(lib) == cache[eIter->first].end()) {
					cache[eIter->first][lib] = std::list<std::string>();
				}
			} else {
				if (strncmp(s, "\t\tMATCH", 7) == 0) {
					cache[eIter->first][lib].push_back(&s[8]);
				}
			}
		}
		stream.close();
	}
}

void
ShowMissingLibs(const std::string &embeddedLib)
{
	std::map<std::string, std::list<std::string> >::const_iterator cIter;
	std::set<std::string>::const_iterator eIter;
	bool any1 = false, any2 = false;

	if (embeddeds.find(embeddedLib) == embeddeds.end()) {
		return;
	}
	if (showUnfixed) {
		for (	eIter  = embeddeds[embeddedLib].begin();
			eIter != embeddeds[embeddedLib].end();
			eIter++)
		{
			if (embeddedsState[embeddedLib][*eIter] == EMBED_UNFIXED) {
				if (any1 == false && pretty) {
					printf("# The following package clones are tracked in the embedded-code-copies\n# database. They have not been fixed.\n");
					printf("#\n\n");
					any1 = true;
				}
				printf("%s CLONED_IN_SOURCE %s\n", embeddedLib.c_str(), eIter->c_str());
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
		if (strcmp(embeddedLib.c_str(), cIter->first.c_str()) == 0)
			continue;

		if (embeddeds[embeddedLib].find(cIter->first.c_str()) == embeddeds[embeddedLib].end()) {
			std::list<std::string>::const_iterator mIter;

			if (any2 == false && pretty) {
				printf("# The following package clones are NOT tracked in the embedded-code-copies\n# database.\n");
				printf("#\n\n");
				any2 = true;
			}
			printf("%s CLONED_IN_SOURCE %s\n", embeddedLib.c_str(), cIter->first.c_str());
			for (	mIter  = cIter->second.begin();
				mIter != cIter->second.end();
				mIter++)
			{
				printf("\t\tMATCH %s\n", mIter->c_str());
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
		ShowMissingLibs(eIter->first);
	}
}

int
main(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "")) != EOF) {
		switch (ch) {
		default:
			Usage(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;

	LoadEmbeds("/var/lib/Clonewise/distros/ubuntu/embedded-code-copies.txt");
	LoadCache();
	if (argc == 0) {
		pretty = false;
		showUnfixed = false;
		ShowMissing();
	} else {
		pretty = true;
		showUnfixed = true;
		for (int i = 0; i < argc; i++) {
			ShowMissingLibs(argv[i]);
		}
	}
}
