# Smart Digging for Recompia

Work-in-progress Modtopia code mod for Recompia / Pokopia.

The goal is a native in-game excavation tool: select two opposite block corners, preview the cuboid, set an independent depth, then confirm or cancel the excavation.

## Planned interaction

1. Activate Smart Digging with a configurable key.
2. Select corner A and corner B by targeting two blocks (WorldEdit-style two-point selection).
3. Adjust excavation depth independently of the horizontal footprint.
4. Show a preview of the selected volume.
5. Confirm to excavate, or cancel/reselect either corner.

The excavation must use the game's own block/tool logic. It must not simulate external keyboard or mouse presses.

## Compatibility

- Recompia: 0.17 (requested target; exact runtime/API compatibility still to be checked).
- Game update: 2.0.1 (the requested target; mapping/build-ID compatibility must be verified before release).
- Platform: Linux x86-64 is the first build target.

## Status

**Research / implementation in progress. No installable package has been released yet.**

Recompia includes the Modtopia code-mod loader and SDK. A code mod is a native shared library packaged with a mod.toml manifest as a .pkmod package. It is distinct from a simple RomFS replacement mod.

See docs/TECHNICAL_PLAN.md for the implementation findings and remaining reverse-engineering work.

## Safety

The tool must never execute a destructive operation before explicit confirmation. The preview and selection state must be cleared on cancel, scene change, save/load transition, or invalid target.