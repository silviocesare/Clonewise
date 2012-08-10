#include "Clonewise.h"
#include <cstdio>
#include <cstdlib>

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-d distroString]\n", argv0);
	exit(1);
}

int
Clonewise_make_embedded_cache(int argc, char* argv[])
{ 
	int ch;

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "d:")) != EOF) {
		switch (ch) {
                case 'd':
			useDistroString = true;
                        distroString = optarg;
                        break;

		default:
			Usage(argv[0]);
			break;
		}
	}

	argc -= optind;
	argv += optind;
	
	LoadEverything();
//	outputFormat = CLONEWISE_OUTPUT_XML;

	exit(0); 
}
