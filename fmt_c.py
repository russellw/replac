#!/usr/bin/python3
# Additionally format C++ code for further entropy reduction
# Assumes clang-format already run
# Does not work for all possible C++ programs
# Test carefully before reusing in other projects!

import argparse
import os
import re

parser = argparse.ArgumentParser(description="Additionally format C++ code")
parser.add_argument("files", nargs="+")
args = parser.parse_args()


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


def flatten(xss):
    r = []
    for xs in xss:
        for x in xs:
            r.append(x)
    return r


########################################
# format comments


def bare_comment(s):
    s = s.strip()
    assert s.startswith("//")
    s = s[2:]
    return s.strip()


def special(s):
    s = bare_comment(s)
    if len(s) == 1:
        return 1
    if s in ("namespace", "SORT", "NO_SORT"):
        return 1
    if s.startswith("clang-format off") or s.startswith("clang-format on"):
        return 1
    if s.startswith("http"):
        return 1
    if s.lower().startswith("todo:"):
        return 1


def is_sentence_end(s):
    if s.endswith("e.g.") or s.endswith("i.e."):
        return 0
    if s.endswith(".") or s.endswith(".)"):
        return 1
    if s.endswith("?"):
        return 1
    if s.endswith(":"):
        return 1


def capitalize(s):
    for c in s:
        if c.isupper():
            return s
    if len(s) == 1:
        return s
    if not s[1].isalpha():
        return s
    return s.capitalize()


def comment_block(i):
    m = re.match(r"(\s*)//", lines[i])
    dent = m[1]

    j = i
    words = []
    while j < len(lines) and re.match(r"\s*//", lines[j]) and not special(lines[j]):
        s = bare_comment(lines[j])
        xs = s.split()
        words.extend(xs)
        j += 1

    if i == j:
        return 1

    k = 0
    words[k] = capitalize(words[k])
    for k in range(1, len(words)):
        if is_sentence_end(words[k - 1]):
            words[k] = capitalize(words[k])
    k = len(words) - 1
    if not is_sentence_end(words[k]):
        words[k] += "."

    width = 132 - len(dent) * 4 - 3
    xs = []
    s = ""
    for w in words:
        if len(s) + 1 + len(w) > width:
            xs.append(s)
            s = w
        else:
            if s:
                s += " "
            s += w
    assert s
    xs.append(s)

    for k in range(len(xs)):
        xs[k] = dent + "// " + xs[k]

    lines[i:j] = xs
    return len(xs)


def comments():
    i = 0
    while i < len(lines):
        m = re.match(r"(\s*)//", lines[i])
        if m:
            if re.match(r"\s*//SORT$", lines[i]):
                lines[i] = m[1] + "// SORT"
                i += 1
                continue

            if re.match(r"\s*//NO_SORT$", lines[i]):
                lines[i] = m[1] + "// NO_SORT"
                i += 1
                continue

            s = bare_comment(lines[i])
            if s.lower().startswith("todo:"):
                s = s[5:]
                s = s.strip()
                lines[i] = m[1] + "// TODO: " + s
                i += 1
                continue

            i += comment_block(i)
        else:
            i += 1


########################################
# sort case blocks


def case(i, dent):
    case_mark = dent + "(case .*|default):$"
    while 1:
        if not re.match(case_mark, lines[i]):
            raise ValueError(filename + ":" + str(i + 1) + ": case not found")
        while re.match(case_mark, lines[i]):
            i += 1
        if dent + "{" == lines[i]:
            i += 1
            while dent + "}" != lines[i]:
                if re.match(case_mark, lines[i]):
                    raise ValueError(
                        filename
                        + ":"
                        + str(i + 1)
                        + ": another case in the middle of block"
                    )
                i += 1
            i += 1
            return i
        else:
            while not re.match(case_mark, lines[i]) and dent + "}" != lines[i]:
                i += 1
            if not re.match(r"\s*\[\[fallthrough\]\];", lines[i - 1]):
                return i


def cases(i, dent):
    r = []
    while dent + "}" != lines[i]:
        j = case(i, dent)
        r.append(lines[i:j])
        i = j
    return i, r


def sort_case(c):
    i = 0
    while re.match(r"\s*(case .*|default):$", c[i]):
        i += 1
    c[:i] = sorted(c[:i], key=lambda s: (s.lower(), s))


def sort_cases(i, dent):
    j, cs = cases(i, dent)
    for c in cs:
        sort_case(c)
    cs = sorted(cs, key=lambda xs: (xs[0].lower(), xs[0]))
    lines[i:j] = flatten(cs)


def sort_case_blocks():
    for i in range(len(lines)):
        m = re.match(r"(\s*)switch \(.*\) {", lines[i])
        if m:
            m1 = re.match(r"\s*// NO_SORT", lines[i - 1])
            if m1:
                continue
            sort_cases(i + 1, m[1])


########################################
# sort single-line elements


def var_key(x):
    m = re.match(r".* (\w+) = ", x)
    if m:
        x = m[1]
    else:
        m = re.match(r".* (\w+)\(", x)
        if m:
            x = m[1]
        else:
            m = re.match(r".* (\w+);", x)
            if m:
                x = m[1]
    return x.lower(), x


def sort_single():
    for i in range(len(lines)):
        if re.match(r"\s*// SORT$", lines[i]):
            if lines[i + 1].endswith("{"):
                continue
            j = i + 1
            while not re.match(r"\s*///$", lines[j]):
                j += 1
            lines[i + 1 : j] = sorted(lines[i + 1 : j], key=var_key)


########################################
# sort multi-line elements


def get_multi_element(dent, i, j):
    i0 = i
    while re.match(r"\s*//", lines[i]):
        i += 1
    m = re.match(r"(\s*).*{$", lines[i])
    if not m:
        raise ValueError(filename + ":" + str(i + 1) + ": inconsistent syntax")
    if m[1] != dent:
        raise ValueError(filename + ":" + str(i + 1) + ": inconsistent indent")
    while lines[i] != dent + "}":
        i += 1
        if i > j:
            raise ValueError(filename + ":" + str(i + 1) + ": inconsistent syntax")
    i += 1
    return lines[i0:i], i


def get_multi_elements(i, j):
    m = re.match(r"(\s*).*", lines[i])
    dent = m[1]
    xss = []
    while i < j:
        xs, i = get_multi_element(dent, i, j)
        xss.append(xs)
        while not lines[i]:
            i += 1
    return xss


def fn_key(xs):
    i = 0
    while re.match(r"\s*//", xs[i]):
        i += 1
    x = xs[i]
    m = re.match(r".* (\w+)\(", x)
    if m:
        x = m[1]
    return x.lower(), x


def sort_multi_block(i, j):
    xss = get_multi_elements(i, j)
    xss = sorted(xss, key=fn_key)
    for k in range(len(xss) - 1):
        xss[k].append("")
    xs = flatten(xss)
    lines[i:j] = xs


def sort_multi():
    i = 0
    while i < len(lines):
        if re.match(r"\s*// SORT$", lines[i]):
            i += 1
            if lines[i] == "":
                raise ValueError(
                    filename + ":" + str(i + 1) + ": blank line after SORT directive"
                )
            if not lines[i].endswith("{") and not re.match(r"\s*//", lines[i]):
                continue
            j = i
            while not re.match(r"\s*///$", lines[j]):
                j += 1
            sort_multi_block(i, j)
        else:
            i += 1


########################################
# blank lines before comments


def comment_blank_lines():
    i = 1
    while i < len(lines):
        m = re.match(r"(\s*)//", lines[i])
        if m:
            if special(lines[i]):
                i += 1
                continue
            if not lines[i - 1]:
                i += 1
                continue
            if re.match(r"\s*//", lines[i - 1]):
                i += 1
                continue
            if re.match(r"\s*#", lines[i - 1]):
                i += 1
                continue
            if lines[i - 1].endswith("{"):
                i += 1
                continue
            if lines[i - 1].endswith(":"):
                i += 1
                continue
            lines[i:i] = [""]
            i += 2
        else:
            i += 1


########################################
# top level


def act():
    global lines
    lines = read_lines(filename)
    old = lines[:]

    comments()
    sort_case_blocks()
    sort_single()
    sort_multi()
    comment_blank_lines()

    if lines == old:
        return
    print(filename)
    write_lines(filename, lines)


for arg in args.files:
    if os.path.isfile(arg):
        filename = arg
        act()
        continue
    for root, dirs, files in os.walk(arg):
        for filename in files:
            ext = os.path.splitext(filename)[1]
            if ext not in (".cc", ".cpp", ".h"):
                continue
            filename = os.path.join(root, filename)
            act()
