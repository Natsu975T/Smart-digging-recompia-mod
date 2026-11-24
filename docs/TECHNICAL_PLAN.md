# Smart Digging — technical plan

## User-approved behaviour

- Selection: two opposite block corners, WorldEdit-style.
- Region: axis-aligned cuboid derived from the selected block coordinates.
- Depth: configurable, extending downward from point A.
- Preview: red while selecting, green while waiting, blue while excavating.
- Activation: Rock Smash while the player's actual cell is strictly inside the selected volume. Aiming at a block is not required; aiming into empty air is valid. Outside or adjacent to the volume, Rock Smash remains native.
- Excavation: native Rock Smash behaviour only, with native hardness, timing, animation, bounce/failure, energy, progression and drops.
- Batching: at most four blocks per native cycle.
- Target platform: Recompia 0.17 on Linux x86-64; Pokopia 2.0.1.

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

## Important constraints

1. **Do not assume arbitrary game addresses are hookable.** A mod can only use hook points declared in its manifest and built into the Recompia game build.
2. **Do not assume built-in Recompia C++ features are directly callable.** The public SDK does not expose the runtime's internal feature registry or OverlayCanvas renderer to third-party mods.
3. **Preview rendering is unresolved.** A genuine 3D volume preview requires a verified native marker mechanism or a minimal compatible Recompia host extension.
4. **Native excavation is unresolved.** The Rock Smash activation hook, player-position layout, authoritative block-break path, completion signal and side effects must be identified before implementation.
5. **Compatibility is build-specific.** Verify the exact game build ID and Modtopia API before using hard-coded addresses.

## Proposed implementation phases

### Phase 1 — compatibility and targeting
- Pin Recompia 0.17 and the exact game build.
- Verify Modtopia API and mapping/build ID.
- Identify and validate the player's world-cell position.
- Validate strict inside/outside checks, including all six faces of the cuboid.

### Phase 2 — selection and settings
- Implement F6 activation and two right-click point selection.
- Store A/B coordinates and configurable depth.
- Reject invalid targets and cap maximum volume.
- Keep selection independent of the current aim after point B.

### Phase 3 — preview
- Draw a real in-world 3D volume: red during selection, green when valid, blue while excavating.
- If the Modtopia API cannot render a world overlay, implement the smallest compatible Recompia host extension rather than substituting a HUD notification.

### Phase 4 — native Rock Smash excavation
- Detect Rock Smash activation independently of the aimed block.
- Start only when the player's actual position is strictly inside the selected volume.
- Invoke the game's authoritative Rock Smash path, preserving hardness, animation, bounce, progression, energy and drops.
- Process batches of at most four blocks; wait for native completion before advancing.
- Stop safely on cancellation, scene changes, invalid memory or errors.
- Never write blocks directly.

### Phase 5 — package and validation
- Build the Linux x86-64 native library against the exact Modtopia SDK/API.
- Package as .pkmod.
- Validate selection, preview, strict position gating, normal Rock Smash outside the region, native excavation, cancellation, save persistence and multiplayer in a disposable test save.

## Current status

### Implemented and tested

- Two-point cuboid geometry, configurable depth, inclusive bounds and deterministic cell iteration.
- Four-block batch partition helper.
- Strict inclusive cell containment helper for the player's in-world cell, with tests for interior and all six outside faces.
- GitHub Actions builds the native library against the pinned Recompia SDK and runs CTest.

### Not yet implemented or verified in-game

- Reading the live player position and converting it to the authoritative world cell.
- Rock Smash activation detection while aiming into air.
- Native per-block Rock Smash invocation and completion tracking.
- Actual 3D volume preview.
- In-game timing, durability, energy, drops, cancellation and multiplayer validation.

No destructive operation is enabled until these native interfaces are verified.
