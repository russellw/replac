#ifdef _MSC_VER
#pragma warning(disable : 4530)
#endif

#include <stdio.h>
#include <stdlib.h>

#include <vector>
using std::vector;

#define version "1"

static void help() {
	printf("Usage: replac from to files\n"
		   "\n"
		   "-h  Show help\n"
		   "-V  Show version\n");
}

int main(int argc, char **argv) {
	vector<const char *> files;
	for (int i = 1; i < argc; ++i) {
		auto s = argv[i];
		if (*s != '-') {
			files.push_back(s);
			continue;
		}
		while (*s == '-') ++s;
		switch (*s) {
		case '?':
		case 'h':
			help();
			return 0;
		case 'V':
		case 'v':
			printf("replac version " version "\n");
			return 0;
		default:
			fprintf(stderr, "%s: unknown option\n", argv[i]);
			return 1;
		}
	}

	for (auto file : files) { puts(file); }
}
