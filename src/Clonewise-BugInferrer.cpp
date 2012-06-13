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
#include "Clonewise-lib-Cache.h"

std::map<std::string, std::set<std::string> > cveReports;
std::set<std::string> notPackages;

static void
Usage(const char *argv0)
{
}

void
loadNotPackages()
{
	std::ifstream stream;

        stream.open("/var/lib/Clonewise/distros/ubuntu/these-are-not-packages");
        if (!stream) {
        }
        while (!stream.eof()) {
                char s[1024];
                       int i;

                stream.getline(s, sizeof(s));
                if (s[0] == 0)
                        continue;

		notPackages.insert(s);
	}
	stream.close();
}

void
loadCveReports()
{
	std::ifstream stream;
        std::string cve, package;

        stream.open("/var/lib/Clonewise/distros/ubuntu/CVE-list");
        if (!stream) {
        }
        while (!stream.eof()) {
                char s[1024];
                       int i;

                stream.getline(s, sizeof(s));
                if (s[0] == 0)
                        continue;

		if (!isspace(s[0])) {
			char str[1024];
			int j;

			str[0] = 0;
			for (i = 0; isspace(s[i]); i++);
			for (j = 0; !isspace(s[i]); i++, j++)
				str[j] = s[i];
			str[j] = 0;
			cve = str;
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
				cveReports[cve].insert(package);
                        }
                }
        }
        stream.close();

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
		if (isspace(s[i]) || (ispunct(s[i]) && s[i] != '.' && s[i] != '_' && s[i] != '-')) {
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
		if (notPackages.find(name1) != notPackages.end())
			continue;
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
	std::string vulnPackage;
	std::set<std::string> vulnSources;
	int ch;
	const char *argv0 = argv[0];
	std::set<std::string>::const_iterator cIter;

	while ((ch = getopt(argc, argv, "")) != EOF) {
		switch (ch) {
		default:
			Usage(argv0);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1)
		Usage(argv0);

	LoadEmbeds("/var/lib/Clonewise/distros/ubuntu/embedded-code-copies.txt");
	LoadCache();
	LoadPackagesInfo();
	LoadExtensions();
	loadCveReports();
	loadNotPackages();

	if (extractCveInfo(argv[0], vulnPackage, vulnSources)) {
		std::set<std::string>::const_iterator sIter;

		printf("# %s relates to a vulnerability in package %s.\n", argv[0], vulnPackage.c_str());
		printf("# The following source filenames are likely responsible:\n");
		for (	sIter  = vulnSources.begin();
			sIter != vulnSources.end();
			sIter++)
		{
			printf("#\t%s\n", sIter->c_str());
		}
		printf("#\n\n");

		if (cveReports.find(argv[0]) != cveReports.end()) {
			printf("# Debian tracks the following packages affected:\n");
			for (	cIter  = cveReports[argv[0]].begin();
				cIter != cveReports[argv[0]].end();
				cIter++)
			{
				printf("#\t%s\n", cIter->c_str());
			}
			printf("#\n\n");
		}
		pretty = true;
		showUnfixed = true;
		ShowMissingLibs(vulnPackage, true, vulnSources, cveReports[argv[0]]);
	}
}
