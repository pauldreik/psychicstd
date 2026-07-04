#!/usr/bin/env python3
"""Count the member functions of std::basic_string in psychicstd vs the system
libstdc++, straight from clang's JSON AST dump (authoritative -- not regex or
line counting), and classify what psychicstd is missing.

The point is to answer "how complete / usable is psychicstd's std::string?" and,
just as importantly, to show that the size gap against libstdc++ is dominated by
private implementation machinery (the SSO / allocator helpers psychicstd
deliberately omits), not by missing public API.

Usage:  python3 count_members.py
Env:    CXX (default clang++-21)
"""

import json
import os
import subprocess
import tempfile

INCLUDE = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))),
    "include",
)
CXX = os.environ.get("CXX", "clang++-21")
STD = os.environ.get("STD", "c++20")

CONFIGS = {
    "system libstdc++": [f"-std={STD}"],
    "psychicstd": [f"-std={STD}", "-nostdinc++", "-isystem", INCLUDE],
}

KINDS = (
    "CXXConstructorDecl",
    "CXXDestructorDecl",
    "CXXMethodDecl",
    "CXXConversionDecl",
)


def dump_ast(flags):
    with tempfile.NamedTemporaryFile("w", suffix=".cpp", delete=False) as f:
        f.write("#include <string>\n")
        src = f.name
    try:
        out = subprocess.run(
            [CXX, *flags, "-Xclang", "-ast-dump=json", "-fsyntax-only", src],
            capture_output=True,
            text=True,
        )
        return json.loads(out.stdout)
    finally:
        os.unlink(src)


def find_basic_string_records(node):
    """Yield the templated CXXRecordDecl of every ClassTemplateDecl named
    basic_string (the primary template)."""
    if isinstance(node, dict):
        if (
            node.get("kind") == "ClassTemplateDecl"
            and node.get("name") == "basic_string"
        ):
            for inner in node.get("inner", []):
                if inner.get("kind") == "CXXRecordDecl":
                    yield inner
        for inner in node.get("inner", []):
            yield from find_basic_string_records(inner)


def collect_members(rec):
    """Walk a record's children in order, tracking the current access level
    (set by AccessSpecDecl 'public:'/'private:' markers), and return the list
    of (kind, name, access) for each explicit member function."""
    access = "private"  # default for a class
    out = []
    for node in rec.get("inner", []):
        if node.get("kind") == "AccessSpecDecl":
            access = node.get("access", access)
        elif node.get("kind") in KINDS and not node.get("isImplicit", False):
            out.append((node["kind"], node.get("name", "<operator>"), access))
    return out


def analyze(flags):
    ast = dump_ast(flags)
    # Pick the record with the most members (guards against forward decls).
    best = []
    for rec in find_basic_string_records(ast):
        members = collect_members(rec)
        if len(members) > len(best):
            best = members
    counts = dict.fromkeys(KINDS, 0)
    all_names, public_names = set(), set()
    for kind, name, access in best:
        counts[kind] += 1
        if kind == "CXXMethodDecl":
            all_names.add(name)
            if access == "public":
                public_names.add(name)
    return counts, all_names, public_names


def main():
    ver = subprocess.run(
        [CXX, "--version"], capture_output=True, text=True
    ).stdout.splitlines()[0]
    print(f"compiler: {ver}\n")

    data = {label: analyze(flags) for label, flags in CONFIGS.items()}
    (pc, pn, ppub), (sc, sn, spub) = data["psychicstd"], data["system libstdc++"]

    print("=== member counts of basic_string ===")
    print(f"  {'':<22}{'psychicstd':>12}{'libstdc++':>12}")
    for k in KINDS:
        print(f"  {k:<22}{pc[k]:>12}{sc[k]:>12}")
    ptot, stot = sum(pc.values()), sum(sc.values())
    print(f"  {'TOTAL members':<22}{ptot:>12}{stot:>12}")
    print(f"  {'distinct method names':<22}{len(pn):>12}{len(sn):>12}")
    print(f"  {'  of which public':<22}{len(ppub):>12}{len(spub):>12}")
    print(
        f"\n  psychicstd has {100 * ptot / stot:.0f}% of libstdc++'s members "
        f"and {100 * len(ppub) / len(spub):.0f}% of its public method names."
    )

    # Access is classified from libstdc++'s own declarations.
    missing = sn - pn
    pub_gap = sorted(spub - ppub)  # public in libstdc++, absent from psychicstd
    priv_missing = sorted(n for n in missing if n not in spub)

    print(f"\n=== what psychicstd is missing ({len(missing)} distinct names) ===")
    print(
        f"  public API gaps ({len(pub_gap)}): {', '.join(pub_gap) if pub_gap else '(none)'}"
    )
    print(
        f"  private helpers libstdc++ has that psychicstd doesn't need ({len(priv_missing)}):"
    )
    print(f"    {', '.join(priv_missing)}")
    print(
        f"\n  => {len(priv_missing)}/{len(missing)} of the gap is private implementation"
    )
    print(f"     machinery; only {len(pub_gap)} are actual public API differences.")

    extra_pub = sorted(ppub - spub)
    extra_priv = sorted((pn - sn) - ppub)
    print(f"\n=== names psychicstd has that libstdc++ doesn't ({len(pn - sn)}) ===")
    print(f"  public: {', '.join(extra_pub) if extra_pub else '(none)'}")
    print(f"  private helpers: {', '.join(extra_priv) if extra_priv else '(none)'}")


if __name__ == "__main__":
    main()
