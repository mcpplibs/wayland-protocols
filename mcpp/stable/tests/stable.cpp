// wayland-protocols stable, exercised as a consumer would.
//
// Three things can be wrong with this package and none is a missing symbol.
//
//   1. THE HEADERS ARE THE PUBLIC INTERFACE. A consumer writes
//      `#include <xdg-shell-client-protocol.h>`, so the generated directory has to be EXPOSED rather
//      than kept build-private. If it were private this file would not
//      compile, which is the first assertion and not a trivial one —
//      freedesktop.wayland had exactly this bug once, and it was masked
//      locally by a header the SubOS happened to carry.
//
//   2. THE INTERFACE TABLES MUST LINK. A header-only package sails past
//      compilation: `xdg_wm_base_interface` is a `wl_interface` OBJECT defined in the
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
#include <xdg-shell-client-protocol.h>
#include <xdg-shell-server-protocol.h>

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
    std::printf("   xdg_wm_base version %d, %d method(s)\n",
                xdg_wm_base_interface.version, xdg_wm_base_interface.method_count);
    check(xdg_wm_base_interface.name != nullptr
          && std::strcmp(xdg_wm_base_interface.name, "xdg_wm_base") == 0,
          "xdg_wm_base is linked in and names itself");

    // A truncated or wrongly generated table would still carry a name; the
    // method count is what says the protocol body came through.
    check(xdg_wm_base_interface.method_count > 0,
          "…and it describes its requests");

    // ── 2. Both sides were generated ─────────────────────────────────────
    // `xdg_wm_base_send_ping` is a `static inline` the SERVER header defines and the client
    // header does not, so taking its address only compiles if the server side
    // is really there. Client and server headers are separate scanner outputs
    // and a packaging mistake tends to lose one of them.
    check(reinterpret_cast<const void *>(&xdg_wm_base_send_ping) != nullptr,
          "the server-side header was generated too");

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
