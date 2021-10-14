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
using std::regex_error;
using std::regex_match;
using std::smatch;

#include <string>
using std::string;

#include <vector>
using std::vector;

#define version "2a"

#ifdef DEBUG
#define dbg(a) cout << __FILE__ << ':' << __LINE__ << ": " << #a << ": " << a << '\n';
#else
#define dbg(a)
#endif

// Unlike the version in ctype.h, this is well-defined for all input values, even if char is signed.
inline bool isSpace(int c) {
	return 0 < c && c <= ' ';
}

void readFile(const string &file, vector<string> &v) {
	ifstream f(file.c_str());
	if (!f) {
		perror(file.c_str());
		exit(1);
	}
	string s;
	while (getline(f, s)) v.push_back(s);
}

void writeFile(const string &file, const vector<string> &v) {
	ofstream f(file.c_str());
	if (!f) {
		perror(file.c_str());
		exit(1);
	}
	for (auto &s : v) f << s << '\n';
}

void help() {
	cout << "Usage: replac [options] from to files\n"
			"\n"
			"Options:\n"
			"-h  Show help\n"
			"-V  Show version\n"
			"\n"
			"-c  Replace in comments\n"
			"    Otherwise, replac tries to skip comments\n"
			"-w  Search for whole words\n"
			"    This is equivalent to putting \\b on either side of from\n"
			"\n"
			"-d  Dry run\n"
			"    Don't actually change any files, just show what changes would be made\n"
			"\n"
			"From: a regular expression\n"
			"To  : a replacement pattern as in C++ std::regex_replace\n"
			"\n"
			"@foo.txt reads args from file foo.txt\n"
			"This is useful for regular expressions that are hard to specify on the command line\n"
			"because they use characters the command shell also regards as special;\n"
			"in that case, from and to can be the first two lines of foo.txt\n";
}

// SORT
vector<string> args1;
bool comments;
bool dry;
bool words;
///

void parse(const vector<string> &args) {
	for (auto &s : args) {
		// Skip empty strings that might be blank lines in response file.
		if (s.empty()) continue;

		// Response file.
		if (s[0] == '@') {
			vector<string> v;
			readFile(s.substr(1, s.size() - 1), v);
			parse(v);
			continue;
		}

		// Does this argument begin with '-'?
		int i = 0;
		while (i < s.size() && s[i] == '-') ++i;

		// No, it's a pattern or file.
		if (!i) {
			args1.push_back(s);
			continue;
		}

		// Yes, but there was nothing after that.
		if (i == s.size()) {
			cerr << s << ": unknown option\n";
			exit(1);
		}

		// Option.
		switch (s[i]) {
		case '?':
		case 'h':
			help();
			exit(0);
		case 'c':
			comments = 1;
			break;
		case 'd':
		case 'n':
			dry = 1;
			break;
		case 'V':
		case 'v':
			cout << "replac version " version "\n";
			exit(0);
		case 'w':
			words = 1;
			break;
		default:
			cerr << s << ": unknown option\n";
			exit(1);
		}
	}
}

bool eq(const string &s, int i, const char *t) {
	for (; *t; ++i)
		if (i >= s.size() || s[i] != *t++) return 0;
	return 1;
}

bool startsWith(const string &s, const char *t) {
	int i = 0;
	while (i < s.size() && isSpace(s[i])) ++i;
	return eq(s, i, t);
}

bool endsWith(const string &s, const char *t) {
	int i = s.size();
	while (i && isSpace(s[i - 1])) --i;
	int n = strlen(t);
	if (n > i) return 0;
	return eq(s, i - n, t);
}

int main(int argc, char **argv) {
	// Set up error handling.
	set_new_handler([]() {
		perror("new");
		exit(1);
	});

	// Command line arguments.
	vector<string> args(argv + 1, argv + argc);
	parse(args);
	if (args1.size() < 3) {
		help();
		return 1;
	}
	if (words) args1[0] = "\\b" + args1[0] + "\\b";

	// From becomes a regular expression; if there was an error in the syntax thereof, an exception will be thrown, so a try-catch
	// block must begin here.
	try {
		regex from(args1[0]);
		auto &to = args1[1];

		// Remember the total number of lines we changed, or would have changed.
		int tot = 0;

		// Process files.
		for (int i = 2; i < args1.size(); ++i) {
			auto &file = args1[i];

			// Going by the extension, does it seem to be a C, C++ or C# file? That matters because in those languages, '#' is used
			// at the start of a line for something other than a line comment.
			bool isc = 0;
			auto j = file.size();
			while (j && file[j - 1] != '.') --j;
			if (j && j < file.size()) switch (file[j]) {
				case 'c':
				case 'C':
				case 'h':
				case 'H':
					isc = 1;
					break;
				}

			// Read the contents of the file.
			vector<string> v;
			readFile(file, v);

			// Remember the number of lines we changed, or would have changed, in this file.
			int changed = 0;

			// For each line in the file.
			for (int j = 0; j < v.size(); ++j) {
				auto &s = v[j];

				// Unless instructed otherwise, replac will try to skip comments. It doesn't claim anything like an encyclopedic
				// knowledge of comment syntax in different programming languages, but looks for some of the more commonly used
				// markers.
				if (!comments) {
					if (startsWith(s, "//")) continue;
					if (!isc && startsWith(s, "#")) continue;
				}

				// Try doing the actual replace in this line.
				auto t = regex_replace(s, from, to);

				// And see whether it changed.
				if (s != t) {
					if (dry) {
						cout << file << ':' << j + 1 << ": ";
						int k = 0;
						while (k < t.size() && isSpace(t[k])) ++k;
						cout << t.substr(k, t.size() - k) << '\n';
					} else
						s = t;
					++changed;
				}
			}

			if (changed) {
				if (!dry) {
					writeFile(file, v);
					cout << file << ' ' << changed << '\n';
				}
				tot += changed;
			}
		}
		cout << tot << '\n';
	} catch (const regex_error &e) {
		cerr << args1[0] << ": " << e.what() << '\n';
		return 1;
	}
	return 0;
}
