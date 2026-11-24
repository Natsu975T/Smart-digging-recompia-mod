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

The mouse-mode implementation documents a player-side hit-cell field and a validity flag. Recompia's v0.17 game layout also names the local character's world-position vector at `character + 0x20c0` and its current action component at `character + 0x18b8` (the running action ID is at action component `+0x984`). The game derives its feet row as `floor((position.y + 0.2) / 100)` and its X/Z cell coordinates from 100-unit grid spacing with a -102400 world origin. These are private, build-specific layouts, not stable Modtopia SDK interfaces; the exact 2.0.1 build ID must be checked before relying on them. They must be validated against the exact target build before use.

## Additional native Rock Smash lifecycle evidence

A second pass through Recompia v0.17.0's `runtime/games/pkp/input/pkp_aim.cpp` confirms useful boundaries, but not yet a callable break routine:

- The five command-less swing actions are IDs **881–885** (Rock Smash, Cut and their upgrades). The exact ID-to-upgrade mapping still needs verification from the game's action data.
- The native action begins at the StartAction mid-hook `main+0x27e5d8`; at that point the action ID is in `w28` and the action component is in `x19`. The action component's owner is at `+0x978`.
- The live swing hit shape is built during the action at `main+0x6d9b40`; its heading is read at `main+0x6d9d70`, and the live shape position is handled at `main+0x6da610`. The preview is a separate path at `main+0x721e40`.
- Recompia's camera-aim feature deliberately leaves the game's shape, validity, breaking and placing behaviour intact. This is evidence that the game performs the real hit evaluation downstream of the live swing shape; it is **not** proof that a third-party mod can safely invoke that path for arbitrary queued cells.
- The action ID becoming zero or changing is used as an action-ended observation by camera-aim code. It may help detect a terminal action state, but must be validated for multi-hit Rock Smash, failure/bounce, interruption and cancellation before it is used as the excavation completion signal.

Consequently, the next reverse-engineering target is the consumer of the live swing hit shape and the native action state machine after StartAction: identify where target blocks receive damage, how energy is charged, and which terminal state corresponds to a completed break. Do not invoke the action repeatedly or advance the queue based only on a guessed duration.

## Two blockers still open

### Strict inside-volume activation\n\nThe independent selection core now exposes `sd_region_contains_cell`, with tests for points inside and outside all six cuboid faces. This is only the geometry predicate: live character-position reading, conversion to the game's authoritative cell coordinates, and Rock Smash activation detection have not yet been integrated.\n\n### World preview

The inspected Modtopia SDK does not expose a general-purpose world-overlay drawing API. It has notifications and screenshot support. Recompia's `OverlayCanvas` is used by built-in features, but is not part of the third-party `MtHost` table. A genuine cuboid preview therefore needs either a verified native in-game marker mechanism or a minimal host-side extension. A text notification is not an acceptable substitute for the requested 3D preview.

### Authoritative excavation

No verified third-party-callable block-break entry point has been identified yet. The desired operation must preserve the game's own tool checks, block durability, drops, inventory effects, world updates and save behaviour. Directly editing world-cell memory or automating keyboard/mouse presses is not an acceptable implementation.

## Implemented and tested in this repository

`src/selection.c` is the first independent part of the mod: it converts two selected block coordinates into an inclusive X/Z cuboid and applies configurable vertical depth. Point A defines the top Y plane; point B defines the opposite X/Z corner. Depth extends downward from point A.

The geometry code rejects incomplete selections, invalid depths, integer underflow and regions larger than the caller's configured block cap. Its CTest suite covers reversed corners, inclusive dimensions, iteration order, depth, reset, and bounds.

This is a tested core component, **not yet a game-integrated mod or installable .pkmod**.
