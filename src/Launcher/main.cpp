#include <cstdlib>
#include <cstdio>
#include <cstring>

struct Command {
	const char *name;
	int (*main)(int argc, char *argv[]);
};


int Clonewise_build_database(int argc, char *argv[]);
int Clonewise_train(int argc, char *argv[]);
int Clonewise_train2(int argc, char *argv[]);
int Clonewise_query(int argc, char *argv[]);
int Clonewise_query_embedded(int argc, char *argv[]);
int Clonewise_query_cache(int argc, char *argv[]);
int Clonewise_query_embedded_cache(int argc, char *argv[]);
int Clonewise_make_cache(int argc, char *argv[]);
int Clonewise_make_embedded_cache(int argc, char *argv[]);
int Clonewise_parse_database(int argc, char *argv[]);
int Clonewise_find_bugs(int argc, char *argv[]);
int Clonewise_find_file(int argc, char *argv[]);
int Clonewise_query_source(int argc, char *argv[]);
int Clonewise_find_license_problems(int argc, char *argv[]);

Command commands[] = {
	{ "build-database", Clonewise_build_database },
	{ "train-shared", Clonewise_train },
	{ "train-embedded", Clonewise_train2 },
	{ "query-shared", Clonewise_query },
	{ "query-embedded", Clonewise_query_embedded },
	{ "query-shared-source", Clonewise_query_source },
	{ "make-shared-cache", Clonewise_make_cache },
	{ "make-embedded-cache", Clonewise_make_embedded_cache },
	{ "query-shared-cache", Clonewise_query_cache },
	{ "query-embedded-cache", Clonewise_query_embedded_cache },
	{ "parse-database", Clonewise_parse_database },
	{ "find-bugs", Clonewise_find_bugs },
	{ "find-license-problems", Clonewise_find_license_problems },
	{ "find-file", Clonewise_find_file },
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
