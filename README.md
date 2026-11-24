# Smart Digging for Recompia

Work-in-progress native Modtopia code mod for Recompia / Pokopia.

The intended final tool is an in-game excavation tool: select two opposite block corners (WorldEdit-style), set independent depth, preview the cuboid, then let the game's own Rock Smash power process the selected volume in batches of at most four blocks (the base game's maximum).

## Current native prototype

- F6 toggles Smart Digging on/off.
- While enabled in Pokopia's native pointer/mouse mode, right-click selects point A and then point B.
- The full cuboid volume is automatically validated and reported immediately after point B.
- Turning off with F6 clears the selection; turning it back on starts fresh.
- After a complete selection, the next right-click begins a new selection.
- Depth remains configurable in Modtopia settings (default 5).
- The selection right-click is blocked from also triggering the game's ordinary tool action.\n\n## Confirmed activation specification\n\n- After selecting the cuboid, Rock Smash is the start command even when aiming into empty air.\n- The player must be strictly inside the selected volume; adjacent/outside positions never trigger Smart Digging. Outside the volume, Rock Smash remains native.\n- Preview colours: red during point selection, green while waiting, blue during excavation.\n- Native block hardness, animation duration, instant breaks, too-hard bounce, energy, progression and drops must remain unchanged.\n\nThe strict region-containment predicate is implemented and covered by unit tests. Live player-position integration, Rock Smash activation, native excavation and 3D rendering are still pending; this does not yet produce an installable finished feature.

**This is still a selection prototype, not the finished excavation mod.** It deliberately does not break blocks. A verified native excavation action path and a 3D world preview are still outstanding.

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
- [Native excavation requirements](docs/NATIVE_DIGGING_REQUIREMENTS.md)

The excavation must use the game's own authoritative block/tool behaviour, including its energy cost, progression checks and drops. Each native Rock Smash cycle is capped at four blocks, with smaller final batches as needed. External keyboard/mouse automation is out of scope. No destructive operation is enabled until its ABI and side effects are verified.
