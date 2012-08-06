#include "Clonewise.h"
#include <getopt.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: Clonewise %s [-v verbosityLevel] srcDirectory\n", argv0);
	exit(1);
}

typedef char *charp;

int
Clonewise_query_source(int argc, char *argv[])
{
	int ch;
	const char *argv0 = argv[0];
	charp queryArgv[2];
	char sigFile[128] = "Clonewise-signature.XXXXXX";
	char cmd[1024];

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "v:")) != EOF) {
		switch (ch) {
		case 'v':
			verbose = atoi(optarg);
			break;

		default:
			Usage(argv0);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		Usage(argv0);
	}

	if (mkstemp(sigFile) == -1) {
		fprintf(stderr, "Error: Can't create temp filename\n");
		exit(1);
	}
	snprintf(cmd, sizeof(cmd), "Clonewise-BuildDatabase build-signature %s %s", argv[0], sigFile);
	system(cmd);

	if (LoadEverything())
		return 1;

	useRelativePathForSignature = false;
	packageAliases[sigFile] = "target";
	queryArgv[0] = sigFile;
	queryArgv[1] = NULL;
	RunClonewise(1, queryArgv);
//	unlink(sigFile);
	return 0;
}
