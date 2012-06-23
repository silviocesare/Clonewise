#include "Clonewise.h"
#include "Clonewise-lib-Cache.h"
#include <cstdlib>
#include <cstdio>
#include <set>
#include <map>
#include <string>
#include <getopt.h>

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-fu]\n", argv0);
	exit(1);
}

int
Clonewise_parse_database(int argc, char *argv[])
{
	int ch;
	char *argv0 = argv[0];
	bool fixed = false, unfixed = false;
	char s[1024];
	std::map<std::string, std::set<std::string> >::const_iterator eIter;
	std::set<std::string>::const_iterator sIter;

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "fu")) != EOF) {
		switch (ch) {
		case 'f':
			fixed = true;
			break;

		case 'u':
			unfixed = true;
			break;

		default:
			Usage(argv0);
		}
	}
	
	argc -= optind;
	argv += optind;

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/embedded-code-copies.txt", distroString);
	LoadEmbeds(s);

	if (fixed) {
		for (	eIter  = embeddeds.begin();
			eIter != embeddeds.end();
			eIter++)
		{
			for (	sIter  = eIter->second.begin();
				sIter != eIter->second.end();
				sIter++)
			{
				if (embeddedsState[eIter->first][*sIter] == EMBED_FIXED) {
					printf("%s/%s\n", eIter->first.c_str(), sIter->c_str());
				}
			}
		}
	} else if (unfixed) {
		for (	eIter  = embeddeds.begin();
			eIter != embeddeds.end();
			eIter++)
		{
			for (	sIter  = eIter->second.begin();
				sIter != eIter->second.end();
				sIter++)
			{
				if (embeddedsState[eIter->first][*sIter] == EMBED_UNFIXED) {
					printf("%s/%s\n", eIter->first.c_str(), sIter->c_str());
				}
			}
		}
	} else {
		for (	eIter  = embeddeds.begin();
			eIter != embeddeds.end();
			eIter++)
		{
			for (	sIter  = eIter->second.begin();
				sIter != eIter->second.end();
				sIter++)
			{
				printf("%s/%s\n", eIter->first.c_str(), sIter->c_str());
			}
		}

	}
	return 0;
}
