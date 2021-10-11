#ifdef _MSC_VER
#pragma warning(disable : 4530)
#endif

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <iostream>
using std::cerr;
using std::cout;

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

vector<string> args1;

void parse(const vector<string> &args) {
	for (auto s : args) {
		if (s.empty()) continue;
		int i = 0;
		while (i < s.size() && s[i] == '-') ++i;
		if (!i) {
			args1.push_back(s);
			continue;
		}
		if (i == s.size()) {
			cerr << s << ": unknown option\n";
			exit(1);
		}
		switch (s[i]) {
		case '?':
		case 'h':
			help();
			exit(0);
		case 'V':
		case 'v':
			cout << "replac version " version "\n";
			exit(0);
		default:
			cerr << s << ": unknown option\n";
			exit(1);
		}
	}
}

int main(int argc, char **argv) {
	set_new_handler([]() {
		perror("new");
		exit(1);
	});
	vector<string> args(argv + 1, argv + argc);
	parse(args);
	if (args1.size() < 3) {
		help();
		return 1;
	}
	auto &from = args1[0];
	auto &to = args1[1];
	for (int i = 2; i < args1.size(); ++i) {
		auto &file = args1[i];
		cout << (file) << '\n';
	}
}
