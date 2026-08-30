// wayland-protocols staging, exercised as a consumer would.
//
// Three things can be wrong with this package and none is a missing symbol.
//
//   1. THE HEADERS ARE THE PUBLIC INTERFACE. A consumer writes
//      `#include <cursor-shape-v1-client-protocol.h>`, so the generated directory has to be EXPOSED rather
//      than kept build-private. If it were private this file would not
//      compile, which is the first assertion and not a trivial one —
//      freedesktop.wayland had exactly this bug once, and it was masked
//      locally by a header the SubOS happened to carry.
//
//   2. THE INTERFACE TABLES MUST LINK. A header-only package sails past
//      compilation: `wp_cursor_shape_manager_v1_interface` is a `wl_interface` OBJECT defined in the
//      generated .c, so reading it is what proves the .c files were compiled.
//
//   3. THE TIER MUST BE THE ONE ASKED FOR. staging/ and unstable/ carry the
//      same protocols under the same symbol names — 38 collisions across
//      tiers, 0 within one — which is why these are three packages. Linking
//      the wrong one is a link error rather than a silent swap, and that is
//      the property worth keeping.
//
// Nothing here connects: no compositor and no display is needed.

#ifdef __linux__

#include <wayland-client.h>
#include <wayland-server.h>
#include <cursor-shape-v1-client-protocol.h>
#include <cursor-shape-v1-server-protocol.h>

#include <cstdio>
#include <cstring>

namespace {
int failures = 0;
void check(bool ok, const char *what)
{
    std::printf("%-58s %s\n", what, ok ? "ok" : "FAILED");
    if (!ok) ++failures;
}
} // namespace

int main()
{
    // ── 1. The interface table linked, and is the right one ──────────────
    std::printf("   wp_cursor_shape_manager_v1 version %d, %d method(s)\n",
                wp_cursor_shape_manager_v1_interface.version, wp_cursor_shape_manager_v1_interface.method_count);
    check(wp_cursor_shape_manager_v1_interface.name != nullptr
          && std::strcmp(wp_cursor_shape_manager_v1_interface.name, "wp_cursor_shape_manager_v1") == 0,
          "wp_cursor_shape_manager_v1 is linked in and names itself");

    // A truncated or wrongly generated table would still carry a name; the
    // method count is what says the protocol body came through.
    check(wp_cursor_shape_manager_v1_interface.method_count > 0,
          "…and it describes its requests");

    // ── 2. Both sides were generated ─────────────────────────────────────
    // cursor-shape has no events, so there is no `_send_` function to point
    // at. Its SECOND interface does the same job: it is defined in the same
    // generated .c and named only by the headers, so referencing it proves
    // both halves were produced. Client and server headers are separate
    // scanner outputs and a packaging mistake tends to lose one of them.
    // cursor-shape has no events, so it has no `_send_` function to point at.
    // Its SECOND interface serves the same purpose: it is defined in the same
    // generated .c and named only by the headers, so referencing it proves
    // both were produced.
    check(wp_cursor_shape_device_v1_interface.name != nullptr,
          "the tier's other interfaces are linked in too");

    // ── 3. The library that marshals these is new enough ─────────────────
    // The generated code calls wl_proxy_marshal_flags, which arrived in
    // libwayland 1.19. Resolving it proves the client library and the scanner
    // that emitted this code are not from different eras.
    check(reinterpret_cast<void *>(&wl_proxy_marshal_flags) != nullptr,
          "libwayland provides wl_proxy_marshal_flags");
    check(reinterpret_cast<void *>(&wl_display_create) != nullptr,
          "libwayland-server is linked for the compositor side");

    std::printf("\n%d check(s) failed\n", failures);
    return failures == 0 ? 0 : 1;
}

#else
int main() { return 0; }
#endif
