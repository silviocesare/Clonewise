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
	fprintf(stderr, "Usage: %s [-o xml] [-d distro] [-estrvvv] signature ...\n", argv0);
	fprintf(stderr, "       %s -a [-o xml] [-d distro] [-estvvv]\n", argv0);
//	fprintf(stderr, "       %s -g [-o xml] [-d distro] [-estvvv]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	const char *argv0;
	int ch;

#if 1
	omp_set_num_threads(2);
#endif
	useRelativePathForSignature = true;
	argv0 = argv[0];
	while ((ch = getopt(argc, argv, "d:eo:starv")) != EOF) {
		switch (ch) {
/*
		case 'g':
			doCheckRelated = true;
			break;
*/
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
			verbose++;
			break;

		default:
			Usage(argv0);
		}
	}
	argc -= optind;
	argv += optind;

	if ((allPackages == false && argc == 0 && doCheckRelated == false) || (allPackages == true && argc != 0))
		Usage(argv0);

	if (LoadEverything())
		return 1;
	return RunClonewise(argc, argv);
}
