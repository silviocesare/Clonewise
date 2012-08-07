#include "Clonewise.h"
#include <getopt.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: Clonewise %s\n", argv0);
	exit(1);
}

typedef char *charp;

int
Clonewise_find_license_problems(int argc, char *argv[])
{
	int ch;
	const char *argv0 = argv[0];
	char cmd[1024];

	ClonewiseInit();

	while ((ch = getopt(argc, argv, "")) != EOF) {
		switch (ch) {
		default:
			Usage(argv0);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0) {
		Usage(argv0);
	}

	snprintf(cmd, sizeof(cmd), "Clonewise-CheckLicenses");
	system(cmd);
	return 0;
}
