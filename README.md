Find and replace patterns in code.

```
Usage: replac [options] from to files

Options:
-h  Show help
-V  Show version

-c  Replace in comments
    Otherwise, replac tries to skip comments

-d  Dry run
    Don't actually change any files, just show what changes would be made

From: a regular expression
To  : a replacement pattern as in C++ std::regex_replace

@foo.txt reads args from file foo.txt
This is useful for regular expressions that are hard to specify on the command line
because they use characters the command shell also regards as special;
in that case, from and to can be the first two lines of foo.txt
and files on subsequent lines or in the rest of the command line
```
