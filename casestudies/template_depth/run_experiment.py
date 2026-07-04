#!/usr/bin/env python3
"""Test the hypothesis: template instantiation dominates compile time far more
than raw source-byte count does. A small amount of template code can generate a
large amount of compiler work.

Method
------
We generate self-contained C++ programs with four strategies, each producing a
tunable amount of "work" M, and sweep M:

  plain            M hand-written structs, each with a method.
                   -> LOTS of source, ~no template instantiation. Baseline.
  instances        1 class template instantiated at M distinct arguments.
                   -> TINY source, M light instantiations. Same resulting
                      entities as `plain` but ~1/M the source bytes.
  instances_heavy  M instantiations, but each one triggers a chain of nested
                   trait instantiations (mimics STL traits machinery).
                   -> TINY source, M heavy instantiations.
  recursive        a single inheritance chain of depth M.
                   -> ~CONSTANT source, instantiation DEPTH grows with M.

For each (strategy, M) we record the source size and the median compile time.

  plain vs instances        isolates bytes vs instantiation (same entities).
  instances vs *_heavy      isolates per-instantiation complexity (same M).
  recursive                 isolates pure depth (source ~constant).

Everything is self-contained (no STL), so we measure the pure effect.

Usage:  python3 run_experiment.py
Env:    CXX (default clang++-21), N (reps), MS (comma-separated)
"""

import os
import statistics
import subprocess
import tempfile
import time

CXX = os.environ.get("CXX", "clang++-21")
STD = os.environ.get("STD", "c++20")
N = int(os.environ.get("N", "7"))
MS = [int(x) for x in os.environ.get("MS", "50,100,200,400,800,1600").split(",")]


def gen_plain(m):
    parts = [
        f"struct P{i} {{ int a=1,b=2,c=3; int f() const {{ return a+b+c+{i}; }} }};\n"
        for i in range(m)
    ]
    uses = "".join(f"  s += P{i}{{}}.f();\n" for i in range(m))
    parts.append(f"long use() {{\n  long s = 0;\n{uses}  return s;\n}}\n")
    parts.append("int main() { return (int)use(); }\n")
    return "".join(parts)


def gen_instances(m):
    s = "template <int I> struct T { int a=1,b=2,c=3; int f() const { return a+b+c+I; } };\n"
    uses = "".join(f"  s += T<{i}>{{}}.f();\n" for i in range(m))
    s += f"long use() {{\n  long s = 0;\n{uses}  return s;\n}}\n"
    s += "int main() { return (int)use(); }\n"
    return s


def gen_instances_heavy(m):
    # Each H<I> has a unique tag type, so instantiating H<I> forces a fresh
    # chain<tag, 12..0> (13 instantiations) that is NOT shared across I.
    s = (
        "template <class Tag, int D> struct chain {\n"
        "  static constexpr long v = chain<Tag, D - 1>::v + (long)sizeof(Tag);\n"
        "};\n"
        "template <class Tag> struct chain<Tag, 0> { static constexpr long v = 0; };\n"
        "template <int I> struct H {\n"
        "  struct tag { char pad[1 + (I % 4)]; };\n"
        "  static constexpr long value = chain<tag, 12>::v + I;\n"
        "};\n"
    )
    uses = "".join(f"  s += H<{i}>::value;\n" for i in range(m))
    s += f"long use() {{\n  long s = 0;\n{uses}  return s;\n}}\n"
    s += "int main() { return (int)use(); }\n"
    return s


def gen_recursive(m):
    # Inheritance forces instantiation of every base -> a chain of depth m.
    # Source is ~constant regardless of m (only the number's digits change).
    return (
        "template <long Nn> struct R : R<Nn - 1> { int x; };\n"
        "template <> struct R<0> { int x; };\n"
        f"unsigned long use() {{ return sizeof(R<{m}>); }}\n"
        "int main() { return (int)use(); }\n"
    )


STRATEGIES = {
    "plain": (gen_plain, []),
    "instances": (gen_instances, []),
    "instances_heavy": (gen_instances_heavy, []),
    "recursive": (gen_recursive, ["-ftemplate-depth=5000"]),
}


def compile_ms(src_path, extra):
    start = time.perf_counter()
    subprocess.run(
        [CXX, f"-std={STD}", *extra, "-c", src_path, "-o", "/dev/null"],
        check=True,
    )
    return (time.perf_counter() - start) * 1000.0


def measure(gen, extra, m, workdir):
    src = gen(m)
    path = os.path.join(workdir, "prog.cpp")
    with open(path, "w") as fh:
        fh.write(src)
    compile_ms(path, extra)  # warm
    med = statistics.median(sorted(compile_ms(path, extra) for _ in range(N)))
    return len(src), med


def main():
    ver = subprocess.run(
        [CXX, "--version"], capture_output=True, text=True
    ).stdout.splitlines()[0]
    print(f"compiler: {ver}")
    print(f"reps per point: {N}, standard: {STD}\n")

    workdir = tempfile.mkdtemp(prefix="tmpl_depth_")
    results = {}
    for name, (gen, extra) in STRATEGIES.items():
        print(f"=== {name} ===")
        rows = []
        for m in MS:
            nbytes, med = measure(gen, extra, m, workdir)
            rows.append((m, nbytes, med))
            per_kb = med / (nbytes / 1024.0)
            print(
                f"  M={m:<5} source={nbytes:>8}B  median={med:7.1f}ms  ({per_kb:6.3f} ms/KB)"
            )
        results[name] = rows

    big = MS[-1]

    def at(name, m):
        return next(r for r in results[name] if r[0] == m)

    print("\n=== summary (at M =", big, ") ===")
    pm, pb, pt = at("plain", big)
    im, ib, it = at("instances", big)
    hm, hb, ht = at("instances_heavy", big)
    rm, rb, rt = at("recursive", big)

    print("\n-- bytes vs instantiation: plain vs instances (same M entities) --")
    print(f"  plain     : {pb:>8}B source, {pt:7.1f} ms")
    print(f"  instances : {ib:>8}B source, {it:7.1f} ms")
    print(
        f"  instances has {pb / ib:.1f}x LESS source but takes {it / pt:.2f}x the time"
    )
    print(
        f"  => time per source byte: {1e3 * it / ib / (1e3 * pt / pb):.1f}x higher for templates"
    )

    print("\n-- per-instantiation complexity: instances vs instances_heavy (same M) --")
    print(f"  instances       : {it:7.1f} ms")
    print(
        f"  instances_heavy : {ht:7.1f} ms  ({ht / it:.1f}x slower for ~same source, same M)"
    )

    print("\n-- pure depth: recursive (source ~constant) --")
    r_small = at("recursive", MS[0])
    print(f"  M={MS[0]:<5}: {r_small[1]}B source, {r_small[2]:.1f} ms")
    print(f"  M={big:<5}: {rb}B source, {rt:.1f} ms")
    print(
        f"  {big / MS[0]:.0f}x more depth on ~constant source => {rt / r_small[2]:.1f}x compile time"
    )


if __name__ == "__main__":
    main()
