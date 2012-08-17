#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/util/XMLString.hpp>
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

using namespace xercesc;

struct VulnCVEReport {
	std::string cveName;
	std::string vulnPackage;
	std::set<std::string> tracked;
	std::string summary;
	std::list<std::string> functions;
	std::set<std::string> vulnSources;
};

struct VulnReport {
	std::string cveName;
	std::string vulnPackage;
	std::set<std::string> tracked;
	std::string package;
	std::string summary;
	std::list<std::string> functions;
	std::set<std::string> vulnSources;
};

std::map<std::string, std::set<std::string> > cveReports;
std::set<std::string> notPackages;
bool useStdin = false;
std::map<std::string, DOMNode *> cvesByXml;
XercesDOMParser *parser;
std::map<std::string, std::string> cpeMap;
std::map<std::string, std::list<VulnReport> > vulnReportsByPackage;
std::map<std::string, std::list<VulnReport> > vulnReportsByEmbeddedPackage;
std::map<std::string, VulnCVEReport> vulnCVEReportsByPackage;

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-e] [-d distroString] [-s] [-v verbosity] [package1 ...]\n", argv0);
	exit(1);
}

void
loadCpes()
{
	std::ifstream stream;
	char s[1024];

	snprintf(s, sizeof(s), "/var/lib/Clonewise/bugs/distros/%s/CPE-list", distroString);
        stream.open(s);
        if (!stream) {
		fprintf(stderr, "Can't open %s\n", s);
		exit(1);
        }
        while (!stream.eof()) {
                char s[1024];
		std::string str, debianPackage, cpeName;

                stream.getline(s, sizeof(s));
                if (s[0] == 0)
                        continue;
		str = s;
		debianPackage = str.substr(0, str.find_first_of(';'));
		cpeName = str.substr(str.find_last_of(':') + 1);
		cpeMap[cpeName] = debianPackage;
	}
	stream.close();
}

void
loadNotPackages()
{
	std::ifstream stream;
	char s[1024];

	snprintf(s, sizeof(s), "/var/lib/Clonewise/bugs/distros/%s/these-are-not-packages", distroString);

        stream.open(s);
        if (!stream) {
		fprintf(stderr, "Can't open %s\n", s);
		exit(1);
        }
        while (!stream.eof()) {
                char s[1024];

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
	char s[1024];

	snprintf(s, sizeof(s), "/var/lib/Clonewise/bugs/distros/%s/CVE-list", distroString);

        stream.open(s);
        if (!stream) {
		fprintf(stderr, "Can't open %s\n", s);
		exit(1);
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

void
findFunctionsFromWordList(std::list<std::string> &functions, const std::list<std::string> &wordList)
{
	std::list<std::string>::const_iterator wIter;

	for (	wIter  = wordList.begin();
		wIter != wordList.end();
		wIter++)
	{
		if (*wIter == "function") {
			if (wIter != wordList.begin()) {
				std::list<std::string>::const_iterator prev;

				prev = wIter;
				--prev;
				functions.push_back(*prev);
			}
		}
	}
				
}

int
extractCveInfoFromSummary(std::string &vulnPackage, std::set<std::string> &vulnSources, const std::string &summary, std::list<std::string> &functions)
{
	std::list<std::string> wordList;

	tokenizeIntoWords(summary.c_str(), wordList);
	findFunctionsFromWordList(functions, wordList);
	if (findSourcesFromWordList(vulnSources, wordList)) {
		return 1;
	}
	return 0;
}

void
readXML(const char *filename)
{
	XMLCh *entryString = XMLString::transcode("entry");
	XMLCh *nameString = XMLString::transcode("name");

	parser->parse(filename);
	DOMDocument *xmlDoc = parser->getDocument();
	DOMElement *elementRoot = xmlDoc->getDocumentElement();
	DOMNodeList *entries = xmlDoc->getElementsByTagName(entryString);

	for (int i = 0; i < entries->getLength(); i++) {
		DOMNode *currentEntry = entries->item(i);
		char *cve = XMLString::transcode(dynamic_cast<DOMElement *>(currentEntry)->getAttribute(nameString));
		cvesByXml[cve] = currentEntry;
		delete [] cve;
	}

	XMLString::release(&entryString);
	XMLString::release(&nameString);
}
	
int
initXmlParser()
{
	try {
		XMLPlatformUtils::Initialize();
		parser = new XercesDOMParser();
		readXML("/var/lib/Clonewise/bugs/cve/nvdcve-2010.xml");
		readXML("/var/lib/Clonewise/bugs/cve/nvdcve-2011.xml");
		readXML("/var/lib/Clonewise/bugs/cve/nvdcve-2012.xml");
	} catch (XMLException &exception) {
		return 1;
	}
	return 0;
}

void
cleanupXmlParser()
{
	delete parser;
}

int
extractCveInfo(const std::string &cve, std::string &vulnPackage, std::set<std::string> &vulnSources, std::string &summary, std::list<std::string> &functions)
{
	int status = 0;

	try {
		XMLCh *entryString = XMLString::transcode("entry");
		XMLCh *nameString = XMLString::transcode("name");
		XMLCh *cveString = XMLString::transcode(cve.c_str());
		XMLCh *prodString = XMLString::transcode("prod");
		XMLCh *descriptString = XMLString::transcode("descript");

		if (cvesByXml.find(cve) != cvesByXml.end()) {
			DOMNode *currentEntry = cvesByXml[cve];
			char *vulnPackageS, *summaryS;
				
			DOMNodeList *prodList= dynamic_cast<DOMElement *>(currentEntry)->getElementsByTagName(prodString);
			if (prodList->getLength() == 0)
				return status;
			DOMNodeList *descriptList = dynamic_cast<DOMElement *>(currentEntry)->getElementsByTagName(descriptString);

			vulnPackageS = XMLString::transcode(dynamic_cast<DOMElement *>(prodList->item(0))->getAttribute(nameString));
			summaryS = XMLString::transcode(dynamic_cast<DOMElement *>(descriptList->item(0))->getTextContent());
			if (cpeMap.find(vulnPackageS) == cpeMap.end())
				vulnPackage = vulnPackageS;
			else
				vulnPackage = cpeMap[vulnPackageS];
			summary = summaryS;

			delete [] vulnPackageS;
			delete [] summaryS;

			status = extractCveInfoFromSummary(vulnPackage, vulnSources, summary, functions);
		}
		XMLString::release(&nameString);
		XMLString::release(&entryString);
		XMLString::release(&cveString);
		XMLString::release(&descriptString);
		XMLString::release(&prodString);
	} catch (XMLException &exception) {
	}
	return status;
}

int
extractHistoricCveInfo(const std::string &cve, std::string &vulnPackage, std::set<std::string> &vulnSources, std::string &summary, std::list<std::string> &functions)
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
			std::map<int, std::string> cveData;

			tokenizeCSV(s, cveData);
			stream.close();
			return extractCveInfoFromSummary(vulnPackage, vulnSources, cveData[2], functions);
		}
	}
	stream.close();
	return 0;
}

void
PrintVulnReport(const VulnReport &v, std::set<std::string> &packages)
{
	std::set<std::string>::const_iterator sIter;
	std::set<std::string>::const_iterator cIter;
	std::set<std::string>::const_iterator pIter;

	printf("# SUMMARY: %s\n", v.summary.c_str());
	printf("#\n\n");

	printf("# %s relates to a vulnerability in package %s.\n", v.cveName.c_str(), v.vulnPackage.c_str());
	printf("# The following source filenames are likely responsible:\n");
	for (	sIter  = v.vulnSources.begin();
		sIter != v.vulnSources.end();
		sIter++)
	{
		printf("#\t%s\n", sIter->c_str());
	}
	printf("#\n\n");

	for (	pIter  = packages.begin();
		pIter != packages.end();
		pIter++)
	{
		if (v.tracked.find(*pIter) != v.tracked.end()) {
			printf("# Debian tracks the following package as affected:\n");
			{
				printf("#\t%s\n", pIter->c_str());
			}
			printf("#\n\n");
		}
	}

	ShowMissingLibs(v.vulnPackage, v.cveName, true, v.vulnSources, v.tracked, v.functions, packages);
}

void
PrintCVEReport(VulnCVEReport &v, std::set<std::string> &packages)
{
	std::set<std::string>::const_iterator sIter;
	std::set<std::string>::const_iterator cIter;

	if (!HasMissingLibs(v.vulnPackage, v.cveName, true, v.vulnSources, v.tracked, v.functions))
		return;

	printf("SUMMARY: %s\n", v.summary.c_str());
	printf("\n\n");

	printf("%s relates to a vulnerability in package %s.\n", v.cveName.c_str(), v.vulnPackage.c_str());
	printf("The following source filenames are likely responsible:\n");
	for (	sIter  = v.vulnSources.begin();
		sIter != v.vulnSources.end();
		sIter++)
	{
		printf("\t%s\n", sIter->c_str());
	}
	printf("\n\n");

	if (v.tracked.size() != 0) {
		printf("Debian tracks the following packages affected:\n");
		for (	cIter  = v.tracked.begin();
			cIter != v.tracked.end();
			cIter++)
		{
			printf("\t%s\n", cIter->c_str());
		}
		printf("\n\n");
	}

	ShowMissingLibs(v.vulnPackage, v.cveName, true, v.vulnSources, v.tracked, v.functions, packages);
}

void
DoWork(const char *cveName)
{
	std::string vulnPackage, summary;
	std::set<std::string> vulnSources;
	std::list<std::string> functions;

	fprintf(stderr, "; %s\n", cveName);
	if (extractCveInfo(cveName, vulnPackage, vulnSources, summary, functions)) {
		std::set<std::string> packages;
		std::set<std::string>::const_iterator pIter;
		VulnCVEReport v;

		pretty = true;
		showUnfixed = true;

		v.cveName = cveName;
		v.vulnPackage = vulnPackage;
		v.summary = summary;
		v.vulnSources = vulnSources;
		v.tracked = cveReports[cveName];
		v.functions = functions;

		PrintCVEReport(v, packages);
		for (	pIter  = packages.begin();
			pIter != packages.end();
			pIter++)
		{
			VulnReport vv;

			vv.tracked = cveReports[cveName];
			vv.cveName = cveName;
			vv.vulnPackage = vulnPackage;
			vv.package = *pIter;
			vv.summary = summary;
			vv.vulnSources = vulnSources;
			vv.tracked = cveReports[cveName];
			vv.functions = functions;

			vulnReportsByEmbeddedPackage[vulnPackage].push_back(vv);
			vulnReportsByPackage[*pIter].push_back(vv);
		}
	} else {
		if (verbose >= 3) {
			printf("# SUMMARY: %s\n", summary.c_str());
			printf("#\n\n");
		}
	}
}

int
Clonewise_find_bugs(int argc, char *argv[])
{
	int ch;
	const char *argv0 = argv[0];
	char s[1024];
	bool useEmbedded = false;

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "v:sd:e")) != EOF) {
		switch (ch) {
		case 'e':
			useEmbedded = true;
			false;

		case 's':
			useStdin = true;
			break;

		case 'v':
			verbose = atoi(optarg);
			break;

		case 'd':
			useDistroString = true;
			distroString = optarg;
			break;

		default:
			Usage(argv0);
		}
	}

	argc -= optind;
	argv += optind;

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/embedded-code-copies.txt", distroString);
	LoadEmbeds(s);
	if (useEmbedded)
		LoadEmbeddedCache();
	else
		LoadCache();
	LoadPackagesInfo();
	LoadExtensions();
	loadCveReports();
	loadNotPackages();
	loadCpes();
	if (initXmlParser())
		exit(1);

	if (argc == 0) {
		std::map<std::string, std::set<std::string> >::const_iterator cIter;
		std::map<std::string, std::list<VulnReport> >::const_iterator pIter;

		printf("### Reports By CVE:\n");
		printf("###\n\n");

		for (	cIter  = cveReports.begin();
			cIter != cveReports.end();
			cIter++)
		{
			DoWork(cIter->first.c_str());
		}

		printf("### Reports by package:\n");
		printf("###\n\n");
		for (	pIter  = vulnReportsByPackage.begin();
			pIter != vulnReportsByPackage.end();
			pIter++)
		{
			std::list<VulnReport>::const_iterator vIter;

			printf("# Package %s may be vulnerable to the following issues:\n", pIter->first.c_str());
			printf("#\n");
			for (	vIter  = pIter->second.begin();
				vIter != pIter->second.end();
				vIter++)
			{
				printf("\t%s\n", vIter->cveName.c_str());
			}
			printf("\n\n");
			for (	vIter  = pIter->second.begin();
				vIter != pIter->second.end();
				vIter++)
			{
				std::set<std::string> packages;

				packages.insert(pIter->first);
				PrintVulnReport(*vIter, packages);
			}
			printf("#\n#\n\n");
		}
	} else {
		if (useStdin) {
			while (!std::cin.eof()) {
				char cveName[1024];

				std::cin.getline(cveName, sizeof(cveName));
				if (cveName[0] == 0)
					continue;
				DoWork(cveName);
			}
		} else {
			for (int i = 0; i < argc; i++) {
				DoWork(argv[i]);	
			}
		}
	}

	cleanupXmlParser();
	exit(0);
}
