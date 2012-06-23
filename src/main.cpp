#include <cstdlib>
#include <cstdio>
#include <cstring>

struct Command {
	const char *name;
	int (*main)(int argc, char *argv[]);
};


int Clonewise_build_database(int argc, char *argv[]);
int Clonewise_query(int argc, char *argv[]);
int Clonewise_query_cache(int argc, char *argv[]);
int Clonewise_make_cache(int argc, char *argv[]);
int Clonewise_parse_database(int argc, char *argv[]);
int Clonewise_find_bugs(int argc, char *argv[]);

Command commands[] = {
	{ "build-database", Clonewise_build_database },
	{ "query", Clonewise_query },
	{ "make-cache", Clonewise_make_cache },
	{ "query-cache", Clonewise_query_cache },
	{ "parse-database", Clonewise_parse_database },
	{ "find-bugs", Clonewise_find_bugs },
	{ NULL, NULL },
};

static void
Usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s command [args ...]\n", argv0);
	fprintf(stderr, "Commands:\n");
	for (int i = 0; commands[i].main; i++) {
		fprintf(stderr, "\t%s\n", commands[i].name);
	}
	exit(0);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		Usage(argv[0]);
	}
	for (int i = 0; commands[i].main; i++) {
		if (strcmp(argv[1], commands[i].name) == 0) {
			exit(commands[i].main(argc - 1, &argv[1]));
		}
	}
	Usage(argv[0]);
}
