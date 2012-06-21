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
	fprintf(stderr, "Usage: %s [-d distroString]\n", argv0);
	exit(1);
}

int
Clonewise_query_cache(int argc, char *argv[])
{
	int ch;
	char s[1024];

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "d:")) != EOF) {
		switch (ch) {
		case 'd':
			distroString = optarg;
			break;

		default:
			Usage(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/embedded-code-copies.txt", distroString);
	LoadEmbeds(s);
	LoadCache();

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
}
