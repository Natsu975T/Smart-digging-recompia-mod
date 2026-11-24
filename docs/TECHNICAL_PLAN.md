# Smart Digging — technical plan

## User-approved behaviour

- Selection model: two distinct points, like WorldEdit (corner A and opposite corner B).
- Region: axis-aligned cuboid derived from the two selected block coordinates.
- Depth: configurable independently from the two-point footprint.
- Preview: display the selected volume before changing the world.
- Controls: configurable activation key; confirm, cancel, and reselect.
- Excavation: invoke native game behaviour; no external input automation.
- Target platform: Recompia 0.17 on Linux x86-64; target game update 2.0.1.

## Recompia integration discovered

The current public Recompia source contains a Modtopia loader, rather than supporting only RomFS replacement mods.

Relevant source locations in Dittobyte/Recompia:

- runtime/include/nxrt/mods.h: host lifecycle and Modtopia integration.
- runtime/third_party/modtopia/sdk/include/modtopia/modtopia.h: stable C ABI for native code mods.
- runtime/third_party/modtopia/sdk/include/modtopia/modtopia_host.h: loader/host contract.
- runtime/third_party/modtopia/mappings/2.0.1.toml: version-specific named functions, globals, fields and build ID.
- runtime/games/pkp/core/pkp_game.cpp: Recompia feature registry and built-in game features.
- runtime/games/pkp/input/pkp_aim.cpp: existing camera-ray targeting logic for tools and abilities.
- runtime/include/nxrt/overlay_canvas.h: host overlay drawing interface (not directly exposed as a drawing API to Modtopia mods in the inspected SDK).

A Modtopia code mod is a native shared library (for Linux, a .so) with an exported entry point and a mod.toml manifest. Recompia packages mods as .pkmod. The manifest declares hook points per supported game version; Recompia only dispatches hooks that its build has explicitly made available.

## Important constraints

1. **Do not assume arbitrary game addresses are hookable.** A mod can only use hook points declared in its manifest and built into the Recompia game build. The 2.0.1 mapping file is not a complete list of every game function.
2. **Do not assume the existing camera aim code is directly callable.** It is an internal Recompia feature. The current SDK exposes guest-memory access, registers, named hook points, game calls, events, config, player input, notifications and screenshots; it does not expose the runtime's C++ feature registry to third-party mods.
3. **Preview rendering is unresolved.** The inspected Modtopia SDK has notifications and screenshots, but no general world-overlay drawing callback. Recompia's internal OverlayCanvas is available to built-in features, not directly to third-party mods. A preview may therefore require a small host-side extension or a supported in-game marker mechanism.
4. **Native excavation is unresolved.** The exact game function(s) that validate and execute a block break, including inventory/tool effects and world updates, must be identified before implementing the operation. Repeatedly synthesizing controller/keyboard input is explicitly out of scope.
5. **Compatibility is build-specific.** The 2.0.1 mapping records build ID 76a489f3bd3642fa5e18bd83d32c98ba; the target Recompia 0.17 runtime must be checked against the actual build and Modtopia API before packaging.

## Proposed implementation phases

### Phase 1 — compatibility and targeting
- Pin the exact Recompia 0.17 source/release and game build.
- Verify the Modtopia API version and the 2.0.1 mapping/build ID used by that release.
- Identify a stable target-cell query for the selected block and prove it with a read-only diagnostic mod.
- Define the coordinate system and validate adjacent, diagonal, vertical and out-of-range selections.

### Phase 2 — selection and settings
- Implement a Modtopia manifest and native code module.
- Add activation, first-point, second-point, confirm and cancel state transitions.
- Store A/B coordinates and configurable depth.
- Reject invalid targets and cap the maximum region volume to avoid runaway operations.

### Phase 3 — preview
- Use a supported game-side rendering hook if available.
- If the Modtopia API cannot render a world overlay, implement the smallest compatible Recompia host extension rather than pretending a notification is a 3D preview.
- Keep rendering read-only.

### Phase 4 — excavation
- Identify the game's authoritative block-break path.
- Invoke it on the correct game thread and preserve the game's normal checks, drops, durability and save behaviour.
- Process a bounded number of cells per frame; stop safely on scene changes, invalid memory, or errors.
- Never begin until the player confirms.

### Phase 5 — package and validation
- Build the Linux x86-64 native library against the exact Modtopia SDK/API.
- Package the library and manifest as a .pkmod.
- Validate loading, settings, selection, preview, cancellation, save persistence and excavation in a disposable test save.
- Publish a release only after a real build and in-game test.

## Current status

### Implemented and locally tested

- `src/selection.c` and `src/selection.h`: independent two-point cuboid geometry.
- Point A sets the top Y plane; point B supplies the opposite X/Z corner. Depth extends downward from A and is configurable from 1 to 256 cells.
- Inclusive bounds and deterministic cell iteration (X fastest, then Z, then descending Y).
- A caller-supplied maximum block count prevents oversized regions; invalid selections and integer underflow are rejected.
- `tests/test_selection.c`: tests for incomplete selections, reversed corners, exact dimensions, iteration, depth, reset and limits.
- Local build and test completed with CMake/Ninja/CTest: **1/1 tests passed**.
- A GitHub Actions workflow was added to run the same tests on pushes and pull requests; its hosted run has not yet been confirmed.

### Still blocked on game integration

Research has confirmed the Modtopia architecture and SDK entry points. It has **not** yet established a safe block-break call or a third-party world-preview path. Those are the two main technical unknowns; no fake implementation or untested binary should be presented as a finished mod.

### Compatibility note

The exact Recompia release tag `v0.17.0` was inspected. Its bundled SDK header is API 0.7 and its Modtopia mapping file is labelled game `2.0.1` with build ID `76a489f3bd3642fa5e18bd83d32c98ba`. Some current game-layer source files describe layouts as `2.0.1a`; that source comment is not proof that the user's installed game is 2.0.1a. Before adding hard-coded game offsets, the runtime's actual game version/build ID must be checked. The implementation must not silently mix the 2.0.1 mapping with a different game build.