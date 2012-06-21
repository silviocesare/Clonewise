#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <map>
#include <getopt.h>
#include <set>
#include <list>
#include <math.h>
#include <cctype>
#include <unistd.h>
#include <fuzzy.h>
#include <cstdarg>
#include "Clonewise.h"
#include <string.h>
#include <omp.h>

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-v level] [-o xml] [-d distro] [-j numThreads] [-estr] signature ...\n", argv0);
	fprintf(stderr, "       %s -a [-v level] [-o xml] [-d distro] [-j numThreads] [-est]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	const char *argv0;
	int ch;

	argv0 = argv[0];
	while ((ch = getopt(argc, argv, "j:d:eo:starv:")) != EOF) {
		switch (ch) {
		case 'j':
			omp_set_num_threads(atoi(optarg));
			break;

		case 'd':
			distroString = optarg;
			break;

		case 'e':
			reportError = !reportError;
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

		case 's':
			useSSDeep = false;
			break;

		case 't':
			useExtensions = false;
			break;

		case 'a': 
			allPackages = true;
			break;

		case 'r':
			useRelativePathForSignature = false;
			break;

		case 'v':
			verbose = atoi(optarg);
			break;

		default:
			Usage(argv0);
		}
	}
	argc -= optind;
	argv += optind;

	if ((allPackages == false && argc == 0) || (allPackages == true && argc != 0))
		Usage(argv0);

	if (LoadEverything())
		return 1;
	return RunClonewise(argc, argv);
}
