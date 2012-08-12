#include "Clonewise.h"
#include "Clonewise-lib-Cache.h"
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

	while ((ch = getopt(argc, argv, "cd:")) != EOF) {
		switch (ch) {
		case 'c':
			checkCacheOnly = true;
			break;

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
	
	MakeEmbeddedCache(argc, argv);

	exit(0); 
}
