# Smart Digging for Recompia

Work-in-progress native Modtopia code mod for Recompia / Pokopia.

The intended final tool is an in-game excavation tool: select two opposite block corners (WorldEdit-style), set independent depth, preview the cuboid, then confirm or cancel.

## Current native prototype

The repository now contains the first native integration in `src/mod.cpp`:

- Modtopia C ABI entry point and `FrameStart` event subscription.
- Strict compatibility gate for Recompia v0.17.0's bundled Pokopia 2.0.1 mapping build ID `76a489f3bd3642fa5e18bd83d32c98ba`.
- Configurable F6/F7/F8/F9/F10 controls.
- Read-only acquisition of the game's current pointer hit cell, when Pokopia's own mouse mode is active.
- Read-only block-ID query to reject empty cells.
- Two-point cuboid geometry with configurable depth and a 4096-cell safety cap.

**This is a selection prototype, not the finished excavation mod.** F10 only validates the selected region. It deliberately does not break blocks. A verified general-purpose native break path and a third-party world-preview path are still outstanding.

## Controls

| Key | Action |
|---|---|
| F6 | Enable/disable Smart Digging |
| F7 | Capture point A |
| F8 | Capture point B |
| F9 | Clear selection |
| F10 | Validate selection (no excavation) |

The keys and depth can be changed in Modtopia's in-game mod settings. Point capture currently requires Pokopia's native pointer/mouse mode to be active and aimed at a solid block.

## Compatibility

- Recompia source release: `v0.17.0`
- Modtopia SDK: API 0.7
- Pokopia mapping: 2.0.1
- Required build ID: `76a489f3bd3642fa5e18bd83d32c98ba`
- First build target: Linux x86-64

## Build

GitHub Actions builds the native library against the SDK bundled in the pinned Recompia release and packages a root-layout `.pkmod` artifact. See the Actions tab for the build result.

## Research and implementation notes

- [Technical plan](docs/TECHNICAL_PLAN.md)
- [SDK and reverse-engineering findings](docs/SDK_FINDINGS.md)
- [Native prototype details and limitations](docs/NATIVE_PROTOTYPE.md)

The excavation must eventually use the game's own authoritative block/tool behaviour. External keyboard/mouse automation is out of scope. No destructive operation is enabled until its ABI and side effects are verified.
