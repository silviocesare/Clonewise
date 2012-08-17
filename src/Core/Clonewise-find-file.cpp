#include "Clonewise.h"
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <fuzzy.h>
#include <set>
#include <cstring>

static bool skipHash = false;

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-h] [-o xml] filename\n", argv0);
	exit(1);
}

static int
findFile(const char *filename)
{
	std::map<std::string, ClonewiseSignature>::const_iterator pIter;
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
	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("<FileClones>\n");
	}
	for (	pIter  = packagesSignatures.begin();
		pIter != packagesSignatures.end();
		pIter++)
	{
		if (pIter->second.filesAndHashes.find(nFilename) != pIter->second.filesAndHashes.end()) {
			std::set<std::string>::const_iterator hIter;

			if (skipHash)
				goto gotit;

			for (	hIter  = pIter->second.filesAndHashes.find(nFilename)->second.begin();
				hIter != pIter->second.filesAndHashes.find(nFilename)->second.end();
				hIter++)
			{
				float s;

				s = fuzzy_compare(hash.c_str(), hIter->c_str());
				if (s > 0.0) {
gotit:
					if (outputFormat == CLONEWISE_OUTPUT_XML) {
						printf("\t<FileClone>\n");
						printf("\t\t<Package>%s</Package>\n", pIter->first.c_str());
						printf("\t\t<Filename>%s</Filename>\n", nFilename.c_str());
						printf("\t\t<Similarity>%f</Similarity>\n", s);
						printf("\t</FileClone>\n");
					} else {
						printf("PACKAGE %s HAS %s (%f)\n", pIter->first.c_str(), nFilename.c_str(), s);
					}
					goto next;
				}
			}
		}
next:
		;
	}
	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("</FileClones>\n");
	}
}

int
Clonewise_find_file(int argc, char *argv[])
{
	int ch;
	const char *argv0 = argv[0];

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "ho:")) != EOF) {
		switch (ch) {
                case 'o':
                        if (strcmp(optarg, "xml") == 0) {
                                outputFormat = CLONEWISE_OUTPUT_XML;
                        } else if (0 && strcmp(optarg, "yaml") == 0) {
                                outputFormat = CLONEWISE_OUTPUT_YAML;
                        } else if (0 && strcmp(optarg, "json") == 0) {
                                outputFormat = CLONEWISE_OUTPUT_JSON;
                        } else {
                                Usage(argv0);
                        }
                        break;

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
