# Verified Modtopia / Recompia findings

Research snapshot: 24 November 2026. Repository inspected: `Dittobyte/Recompia`, release `v0.17.0`.

## Modtopia SDK

The public SDK header at `runtime/third_party/modtopia/sdk/include/modtopia/modtopia.h` declares:

- Modtopia C ABI **0.7** (`MT_API_VERSION_MINOR = 7`).
- Native code-mod entry point `mod_init(const MtHost *, MtMod *)`, exported with `MT_DECLARE_MOD()`.
- Hook registration by manifest point name; hooks execute on game threads.
- Guest memory access through `mem_read` / `mem_write`, never by treating guest addresses as host pointers.
- `resolve` for mapped names or `main+0x1234` addresses, `module_base`, and `call_guest`.
- Events including `FrameStart`, `SaveLoaded`, `SaveWritten`, and `AreaEntered`.
- Runtime settings callbacks, `key_down`, controller `pad_read` / `pad_hold` / `pad_block`, notifications and screenshots.

The companion host header identifies Modtopia core **0.8.0**. Recompia's backend supplies guest-memory access, guest calls, and a check that a requested hook point is built into the game build. A mod must not assume that declaring an arbitrary offset in its manifest makes that offset hookable.

## Recompia 0.17 target

The public Recompia repository has a release tagged `v0.17.0` (published 23 November 2026). This is the user's requested host version.

The repository's current game-layer source contains internal Pokopia 2.0.1-family layouts and a camera-aim implementation. The exact installed game's build ID and the exact mapping selected by the user's 0.17 build still need to be confirmed before using any address in a release binary.

## What Bigger Stacks proves

The official Bigger Stacks listing identifies it as:

- A Modtopia gameplay mod.
- Game version 2.0.1.
- Requiring Modtopia 0.1 or newer.
- In testing.
- A mod that changes gameplay and therefore must match for all players in a Link Play session.

This confirms that third-party Modtopia code mods can change gameplay. It does **not** reveal its implementation or prove that block-breaking functions are available to third-party mods.

## Targeting information in Recompia's own source

The internal file `runtime/games/pkp/input/pkp_aim.cpp` documents how Recompia's built-in camera aim works:

- The command target query is at `main+0x2e19a0`.
- The query reads the player's heading and chooses forward cells.
- Tool hit shapes for the command-less swing actions are built in separate paths.
- The row helper reads the active camera pitch and the world collision metadata.
- The built-in feature keeps the game's own highlighted shapes, validity checks, damage, breaking and placing behaviour.

The mouse-mode implementation documents a player-side hit-cell field and a validity flag. These are useful reverse-engineering leads, but they are private game layouts, not stable Modtopia SDK interfaces. They must be validated against the exact target build before use.

## Two blockers still open

### World preview

The inspected Modtopia SDK does not expose a general-purpose world-overlay drawing API. It has notifications and screenshot support. Recompia's `OverlayCanvas` is used by built-in features, but is not part of the third-party `MtHost` table. A genuine cuboid preview therefore needs either a verified native in-game marker mechanism or a minimal host-side extension. A text notification is not an acceptable substitute for the requested 3D preview.

### Authoritative excavation

No verified third-party-callable block-break entry point has been identified yet. The desired operation must preserve the game's own tool checks, block durability, drops, inventory effects, world updates and save behaviour. Directly editing world-cell memory or automating keyboard/mouse presses is not an acceptable implementation.

## Implemented and tested in this repository

`src/selection.c` is the first independent part of the mod: it converts two selected block coordinates into an inclusive X/Z cuboid and applies configurable vertical depth. Point A defines the top Y plane; point B defines the opposite X/Z corner. Depth extends downward from point A.

The geometry code rejects incomplete selections, invalid depths, integer underflow and regions larger than the caller's configured block cap. Its CTest suite covers reversed corners, inclusive dimensions, iteration order, depth, reset, and bounds.

This is a tested core component, **not yet a game-integrated mod or installable .pkmod**.
