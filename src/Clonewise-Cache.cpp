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

void
Usage(const char *argv0)
{
}

enum EmbedType {
	EMBED_FIXED,
	EMBED_UNFIXED
};

struct Match {
	std::string filename1;
	std::string filename2;
	std::string weight;
};

std::map<std::string, std::map<std::string, EmbedType> > embeddedsState;
std::map<std::string, std::set<std::string> > embeddeds;
std::map<std::string, std::map<std::string, std::list<Match> > > cache;
bool pretty = false;
bool showUnfixed = false;
bool useCve = false;
std::string cve;

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

void
ShowMissingLibs(const std::string &embeddedLib, bool useMatchFilename, const std::set<std::string> &matchFilename)
{
	std::map<std::string, std::list<Match> >::const_iterator cIter;
	std::set<std::string>::const_iterator eIter;
	bool any1 = false, any2 = false;
	std::string m;

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
			std::list<Match>::const_iterator mIter;

			if (useMatchFilename) {
				for (	mIter  = cIter->second.begin();
					mIter != cIter->second.end();
					mIter++)
				{
					if (matchFilename.find(mIter->filename1) != matchFilename.end() || matchFilename.find(mIter->filename2) != matchFilename.end()) {
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
			printf("%s CLONED_IN_SOURCE %s\n", embeddedLib.c_str(), cIter->first.c_str());
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
		std::set<std::string> vulnSources;

		ShowMissingLibs(eIter->first, false, vulnSources);
	}
}

void
tokenizeCSV(const char *s, std::map<int, std::string> &tokens)
{
	char token[strlen(s)];
	int i, last, index, tokenPtr;
	bool quote;

	// Assumes csv file is not corrupt.
	quote = false;
	tokenPtr = 0;
	index = 0;
	last = 0;
	i = 0;
	while (s[i]) {
		if (quote == false && i == last && s[i] == '"') {
			quote = true;
			i++;
			continue;
		}
		if (quote == true && s[i] == '"') {
			if (s[i + 1] == '"') {
				token[tokenPtr] = '"';
				tokenPtr++;
				i += 2;
				continue;
			} else {
				quote = false;
				i++;
				continue;
			}
		}
		if (quote == false && s[i] == ',') {
			token[tokenPtr] = 0;
			tokenPtr = 0;
			tokens[index] = token;
			index++;
			i++;
			last = i;
			continue;
		}	
		token[tokenPtr] = s[i];
		tokenPtr++;
		i++;
	}
}

void
tokenizeIntoWords(const char *s, std::list<std::string> &wordList)
{
	std::string str(s);
	int last = 0;

	for (int i = 0; s[i]; i++) {
		if (isspace(s[i]) || (ispunct(s[i]) && s[i] != '.' && s[i] != '_')) {
			std::string word;

			word = str.substr(last, i - last);
			if (word.size() != 0) {
				if (word[word.size() - 1] == '.') {
					word = str.substr(last, i - last - 1);
				}
				wordList.push_back(word);
			}
			last = i + 1;
		}
	}
}

int
findPackageFromWordList(std::string &package, const std::list<std::string> &wordList)
{
	std::list<std::string>::const_iterator wIter;

	for (	wIter  = wordList.begin();
		wIter != wordList.end();
		wIter++)
	{
		std::map<std::string, std::set<std::string> >::const_iterator eIter;
		char name1[1024];
		int i;

		for (i = 0; wIter->c_str()[i] && i < sizeof(name1); i++)
			name1[i] = tolower(wIter->c_str()[i]);
		name1[i] = 0;
		for (	eIter  = embeddeds.begin();
			eIter != embeddeds.end();
			eIter++)
		{
			int j;
			char name2[1024];

			for (i = 0; eIter->first.c_str()[i] && i < sizeof(name2); i++)
				name2[i] = tolower(eIter->first.c_str()[i]);
			name2[i] = 0;

			i = 0;
			if (strncmp(name1, "lib", 3) == 0)
				i += 3;
			j = 0;
			if (strncmp(name2, "lib", 3) == 0)
				j += 3;
			for (; name1[i] == name2[j] && name1[i]; i++, j++);
			if (name1[i] == 0 && name2[j] == 0) {
				package = eIter->first;
				return 1;
			}
		}
	}
	return 0;
}

int
findSourcesFromWordList(std::set<std::string> &sources, const std::list<std::string> &wordList)
{
	std::list<std::string>::const_iterator wIter;

	for (	wIter  = wordList.begin();
		wIter != wordList.end();
		wIter++)
	{
		if (IsProgramFilename(*wIter)) {
			std::string m;

			normalizeFeature(m, *wIter);
			sources.insert(m);
		}
	}
	return sources.size() != 0;
}

int
extractCveInfo(const std::string &cve, std::string &vulnPackage, std::set<std::string> &vulnSources)
{
	std::ifstream stream;

	stream.open("/var/lib/Clonewise/allitems.csv");
	if (!stream) {
		return 0;
	}
	while (!stream.eof()) {
		char s[1024 * 32];

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		if (strncmp(s, cve.c_str(), strlen(cve.c_str())) == 0) {
			std::list<std::string> wordList;
			std::map<int, std::string> cveData;

			tokenizeCSV(s, cveData);
			printf("# SUMMARY: %s\n", cveData[2].c_str());
			printf("#\n\n");
			tokenizeIntoWords(cveData[2].c_str(), wordList);
			if (findPackageFromWordList(vulnPackage, wordList)) {
				if (findSourcesFromWordList(vulnSources, wordList)) {
					return 1;
				}
			}
			break;
		}
	}
	stream.close();
	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "c:")) != EOF) {
		switch (ch) {
		case 'c':
			useCve = true;
			cve = optarg;
			break;

		default:
			Usage(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;

	LoadEmbeds("/var/lib/Clonewise/distros/ubuntu/embedded-code-copies.txt");
	LoadCache();
	if (useCve) {
		std::string vulnPackage;
		std::set<std::string> vulnSources;

		LoadPackagesInfo();
		LoadExtensions();
		if (extractCveInfo(cve, vulnPackage, vulnSources)) {
			std::set<std::string>::const_iterator sIter;

			printf("# %s relates to a vulnerability in package %s.\n", cve.c_str(), vulnPackage.c_str());
			printf("# The following source filenames are likely responsible:\n");
			for (	sIter  = vulnSources.begin();
				sIter != vulnSources.end();
				sIter++)
			{
				printf("#\t%s\n", sIter->c_str());
			}
			printf("#\n\n");

			pretty = true;
			showUnfixed = true;
			ShowMissingLibs(vulnPackage, true, vulnSources);
		}
	} else {
		if (argc == 0) {
			pretty = false;
			showUnfixed = false;
			ShowMissing();
		} else {
			std::set<std::string> vulnSources;

			pretty = true;
			showUnfixed = true;
			if (argv[1] == NULL) {
				ShowMissingLibs(argv[0], false, vulnSources);
			} else {
				std::string m;

				normalizeFeature(m, argv[1]);
				vulnSources.insert(m);
				ShowMissingLibs(argv[0], true, vulnSources);
			}
		}
	}
}
