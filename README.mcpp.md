# mcpplibs/wayland-protocols

[wayland-protocols](https://gitlab.freedesktop.org/wayland/wayland-protocols)
1.49 with mcpp build support. Three packages reach
[mcpp-index](https://github.com/mcpplibs/mcpp-index):

| member | protocols | published as |
|---|---|---|
| `mcpp/stable` | 5 — xdg-shell, linux-dmabuf-v1, tablet-v2, viewporter, presentation-time | `freedesktop.wayland-protocols-stable` |
| `mcpp/staging` | 31 | `freedesktop.wayland-protocols-staging` |
| `mcpp/unstable` | 19 | `freedesktop.wayland-protocols-unstable` |

```bash
mcpp build --workspace
```

## Why a fork at all

wayland-protocols ships **XML and nothing else** — 65 files and a pkg-config
entry naming the directory. A consumer runs `wayland-scanner` over the ones it
uses and compiles the result itself; there is no library to link and no header
to include until someone generates them. An inline index descriptor has nothing
to compile, so the generator runs once here and the output is checked in.

**No scanner runs in a consumer's build.** `mcpp build` is the whole toolchain.
CI regenerates with the *ecosystem's* `freedesktop.wayland-scanner` — the same
wayland release the client and server libraries come from, so the generated
code cannot drift from the library that marshals it — and diffs.

## Why three packages and not one

Because all 65 in one library **does not link**, and that is measured:

```
multiple definition of `zwp_linux_dmabuf_v1_interface'
```

staging/ and unstable/ carry the same protocol at different maturity levels,
and the scanner emits the same symbol names for both. Counted, per exported
`wl_interface`:

| | |
|---|---|
| stable ∩ staging | **0** |
| stable ∩ unstable | **13** |
| staging ∩ unstable | **0** |
| within any one tier | **0** |

So the tier is exactly the boundary along which the protocols coexist — and it
is upstream's own directory structure, not a split invented here. CI asserts
the three stay disjoint.

### Three unstable protocols are not shipped

`xdg-shell-unstable-v5`, `linux-dmabuf-unstable-v1` and `tablet-unstable-v2` —
exactly the 13-symbol overlap. Each was **superseded by a stable protocol of
the same name**, and upstream keeps the old spelling only for compatibility.
Shipping both would make the unstable package unusable beside the stable one,
which it depends on anyway.

### staging and unstable depend on stable

Not by convention — measured. Both reference `xdg_toplevel_interface`, and
staging also references `zwp_tablet_tool_v2_interface`; stable defines them.
The edge is a `path` dependency inside this workspace, so the tarball is
self-contained.

## The cost of shipping a whole tier

mcpp links a dependency's objects into the consumer, so naming a tier means
carrying it. Measured before deciding: all 65 protocols compile to **270 KB**
of `wl_interface` tables — 4 KB each. The 3.5 MB in `mcpp/generated/` is almost
entirely **headers** (84,288 lines), and a header costs nothing until included.
Splitting per protocol would mean 65 packages for 270 KB.

## experimental/ is not shipped

Upstream's own word for that tier is experimental, the protocols are `xx-`
prefixed to make them unusable by accident, and nothing in the index asks for
them.

## Layout

```
upstream/            wayland-protocols 1.49, verbatim — never touched
mcpp/
  generated/         195 files: -protocol.c and both header sides, per protocol
  tools/genproto.sh  regenerates them from upstream/ with a given scanner
  stable/ staging/ unstable/
mcpp.toml            the workspace root
```
