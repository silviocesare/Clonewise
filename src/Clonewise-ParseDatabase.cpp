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
	fprintf(stderr, "Usage: %s [-fu] [-o xml]\n", argv0);
	exit(1);
}

static void
PrintClone(const char *embeddedLib, const char *package, const char *state)
{
	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("\t<Clone>\n");
		printf("\t\t<EmbeddedLibrary>%s</EmbeddedLibrary>\n", embeddedLib);
		printf("\t\t<Package>%s</Package>\n", package);
		printf("\t\t<State>%s</State>\n", state);
		printf("\t</Clone>\n");
	} else {
		printf("%s/%s\n", embeddedLib, package);
	}
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

	while ((ch = getopt(argc, argv, "fuo:")) != EOF) {
		switch (ch) {
		case 'f':
			fixed = true;
			break;

		case 'u':
			unfixed = true;
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

	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("<EmbeddedCodeCopies>\n");
	}
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
					PrintClone(eIter->first.c_str(), sIter->c_str(), "Fixed");
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
					PrintClone(eIter->first.c_str(), sIter->c_str(), "Unfixed");
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
				if (embeddedsState[eIter->first][*sIter] == EMBED_FIXED) {
					PrintClone(eIter->first.c_str(), sIter->c_str(), "Fixed");
				} else {
					PrintClone(eIter->first.c_str(), sIter->c_str(), "Unfixed");
				}
			}
		}

	}
	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		printf("</EmbeddedCodeCopies>\n");
	}
	return 0;
}
