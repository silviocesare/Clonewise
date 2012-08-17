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

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-d distroString] [-o xml]\n", argv0);
	exit(1);
}

int
Clonewise_query_embedded_cache(int argc, char *argv[])
{
	int ch;
	char s[1024];
	const char *argv0 = argv[0];

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "d:o:")) != EOF) {
		switch (ch) {
		case 'd':
			useDistroString = true;
			distroString = optarg;
			break;

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

		default:
			Usage(argv0);
		}
	}

	argc -= optind;
	argv += optind;

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/embedded-code-copies.txt", distroString);
	LoadEmbeds(s);
	LoadEmbeddedCache();

	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("<EmbeddedCodeCopies>\n");
	}
	if (argc == 0) {
		pretty = false;
		showUnfixed = false;
		ShowMissing();
	} else {
		std::set<std::string> vulnSources, exclude;
		std::list<std::string> functions;
		std::set<std::string> packages;

		pretty = true;
		showUnfixed = true;
		if (argv[1] == NULL) {
			ShowMissingLibs(argv[0], "", false, vulnSources, exclude, functions, packages);
		} else {
			std::string m;

			normalizeFeature(m, argv[1]);
			vulnSources.insert(m);
			ShowMissingLibs(argv[0], "", true, vulnSources, exclude, functions, packages);
		}
	}
	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("</EmbeddedCodeCopies>\n");
	}
}
