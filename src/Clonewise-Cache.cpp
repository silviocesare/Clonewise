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
		std::set<std::string> vulnSources, exclude;

		pretty = true;
		showUnfixed = true;
		if (argv[1] == NULL) {
			ShowMissingLibs(argv[0], false, vulnSources, exclude);
		} else {
			std::string m;

			normalizeFeature(m, argv[1]);
			vulnSources.insert(m);
			ShowMissingLibs(argv[0], true, vulnSources, exclude);
		}
	}
}
