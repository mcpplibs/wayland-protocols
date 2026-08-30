#!/bin/sh
#
# Regenerate mcpp/generated/ from upstream/'s XML.
#
# WHY THE OUTPUT IS CHECKED IN
#
# wayland-protocols ships XML and no code. Something has to run
# wayland-scanner, and the choice is between doing it once here or doing it in
# every consumer's build. Once here, because the output is a pure function of
# the XML and the scanner version — nothing about it depends on the target — so
# the same rule applies as to freedesktop.wayland's own protocol code and
# libglvnd's dispatch tables: precomputable output is checked in, and CI
# regenerates and diffs it. A consumer needs mcpp and nothing else.
#
# THE SCANNER IS THE ECOSYSTEM'S, NOT THE HOST'S
#
# `freedesktop.wayland-scanner` builds it from the same wayland release the
# client and server libraries come from, so the generated code cannot drift
# from the library that consumes it. Passing a host wayland-scanner would
# reintroduce exactly the coupling this index exists to remove.
#
# Usage: mcpp/tools/genproto.sh <path-to-wayland-scanner>
set -eu

SCANNER="${1:?usage: genproto.sh <path-to-wayland-scanner>}"
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT="$ROOT/mcpp/generated"

[ -x "$SCANNER" ] || { echo "not executable: $SCANNER" >&2; exit 1; }
"$SCANNER" --version 2>&1 | head -1

mkdir -p "$OUT"
n=0
# LC_ALL=C so the traversal order is the same everywhere. It does not affect
# file CONTENT here, but it keeps the log comparable run to run — and the GL
# module generator in mcpplibs/libglvnd learned the harder version of this
# lesson, where locale DID change the output.
export LC_ALL=C
for xml in $(find "$ROOT/upstream" -name '*.xml' | sort); do
    base=$(basename "$xml" .xml)
    "$SCANNER" -s public-code   "$xml" "$OUT/$base-protocol.c"
    "$SCANNER" -s client-header "$xml" "$OUT/$base-client-protocol.h"
    "$SCANNER" -s server-header "$xml" "$OUT/$base-server-protocol.h"
    n=$((n + 1))
done
echo "regenerated $n protocol(s) -> mcpp/generated/"
