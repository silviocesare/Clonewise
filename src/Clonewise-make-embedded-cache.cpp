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
	LoadCache();

        if (1) {
                std::map<std::string, std::set<std::string> >::const_iterator eIter;

                if (embeddedList.size() == 0) {
                        char s[1024];

                        snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/embedded-code-copies", distroString);
                        LoadEmbeddedCodeCopiesList(s);
                }
                for (   eIter  = embeddedList.begin();
                        eIter != embeddedList.end();
                        eIter++)
                {
			char myName[strlen(eIter->first.c_str()) + 1];
			char *v[] = { myName, NULL };
			char s[1024];
			int c = 1;

			strcpy(myName, eIter->first.c_str());

			snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/cache-embedded/%s", distroString, myName);
			if (0 && access(s, R_OK) == 0) {
				outFd = fopen(s, "r");
			} else {
				outFd = fopen(s, "w+");
				RunClonewise(c, v, true);
			}
		}
	}

	exit(0); 
}
