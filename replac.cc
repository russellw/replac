#ifdef _MSC_VER
#pragma warning(disable : 4530)
#endif

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
using std::cerr;
using std::cout;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <new>
using std::set_new_handler;

#include <regex>
using std::regex;
using std::regex_match;
using std::smatch;

#include <string>
using std::string;

#include <vector>
using std::vector;

#define version "1"

void help() {
	cout << "Usage: replac [options] from to files\n"
			"\n"
			"Options:\n"
			"-h  Show help\n"
			"-V  Show version\n"
			"\n"
			"From: a regular expression\n"
			"To  : a replacement pattern as in C++ std::regex_replace\n"
			"\n"
			"@foo.txt reads args from file foo.txt\n"
			"This is useful for regular expressions that are hard to specify on the command line\n"
			"because they use characters the command shell also regards as special;\n"
			"in that case, from and to can be the first two lines of foo.txt\n"
			"and files on subsequent lines or in the rest of the command line\n";
}

void readFile(const string &file, vector<string> &v) {
	ifstream in(file.c_str());
	if (!in) {
		perror(file.c_str());
		exit(1);
	}
	string str;
	while (getline(in, str)) v.push_back(str);
}

vector<string> files;

int main(int argc, char **argv) {
	set_new_handler([]() {
		perror("new");
		exit(1);
	});

	if (argc < 4) {
		help();
		return 0;
	}

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
			cout << "replac version " version "\n";
			return 0;
		default:
			cerr << argv[i] << ": unknown option\n";
			return 1;
		}
	}

	for (auto file : files) { cout << (file) << '\n'; }
}
