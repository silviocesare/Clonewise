#include "Clonewise.h"
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <fuzzy.h>
#include <set>

static bool skipHash = false;

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-h] filename\n", argv0);
	exit(1);
}

static int
findFile(const char *filename)
{
	std::map<std::string, std::map<std::string, std::set<std::string> > >::const_iterator pIter;
	std::string nFilename;
	char line[1024], cmd[1024];
	std::string feature, hash;
	FILE *p;

	snprintf(cmd, sizeof(cmd), "ssdeep %s|tail -n1", filename);
	p = popen(cmd, "r");
	fgets(line, sizeof(line), p);
	pclose(p);
	lineToFeature(line, feature, hash);
	normalizeFeature(nFilename, feature);
	for (	pIter  = packagesSignatures.begin();
		pIter != packagesSignatures.end();
		pIter++)
	{
		if (pIter->second.find(nFilename) != pIter->second.end()) {
			std::set<std::string>::const_iterator hIter;

			if (skipHash)
				goto gotit;

			for (	hIter  = pIter->second.find(nFilename)->second.begin();
				hIter != pIter->second.find(nFilename)->second.end();
				hIter++)
			{
				float s;

				s = fuzzy_compare(hash.c_str(), hIter->c_str());
				if (s > 0.0) {
gotit:
					printf("PACKAGE %s HAS %s\n", pIter->first.c_str(), nFilename.c_str());
					goto next;
				}
			}
		}
next:
		;
	}
}

int
Clonewise_find_file(int argc, char *argv[])
{
	int ch;
	const char *argv0 = argv[0];

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "h")) != EOF) {
		switch (ch) {
		case 'h':
			skipHash = true;
			break;

		default:
			Usage(argv0);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		Usage(argv0);
	}

	LoadEverything();
	return findFile(argv[0]);
}
